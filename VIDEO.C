/* $Header:   D:/VCS/FCIT/VIDEO.C_V   1.1   01 Nov 1991 11:21:00   FJM  $ */
/*
 * 		Video.c:  IBM PC video routines.  Hardware dependant.
 *
 *	Fred - Created 15th May, 1991 from routines in Window.c
 *
 * --------------------------------------------------------------------
 *  Version control log:
 *
 * $Log:   D:/VCS/FCIT/VIDEO.C_V  $
 * 
 *    Rev 1.1   01 Nov 1991 11:21:00   FJM
 * Added gl_ structures
 *
 *    Rev 1.0   11 Jun 1991  8:19:08   FJM
 * Initial revision.
 *
 * --------------------------------------------------------------------
 */


#include <alloc.h>
#include <conio.h>
#include <dos.h>
#include <string.h>
#include <time.h>
#include "ctdl.h"
#include "proto.h"
#include "global.h"

/* -------------------------------------------------------------------- */
/*      cls()                   clears the screen                       */
/*      cursoff()               turns cursor off                        */
/*      curson()                turns cursor on                         */
/*      gmode()                 checks for monochrome card              */
/*      position()              positions the cursor                    */
/*      readpos()               returns current row, col position       */
/*      scroll()                scrolls window up                       */
/*      setscreen()             sets videotype flag                     */
/*      directchar()            Direct screen write char with attr      */
/*      directstring()          Direct screen write string w/attr at row*/
/*      bioschar()              BIOS print char with attr               */
/*      biosstring()            BIOS print string w/attr at row         */
/*      setscreen()             Set SCREEN to correct address for VIDEO */
/*      setscreen()             Set SCREEN to correct address for VIDEO */
/*      help()                  toggles help menu                       */
/*      updatehelp()            updates the help window                 */
/* -------------------------------------------------------------------- */

#define CHAR_WIDTH	2
#define ROW_LENGTH	(v_info.cols*CHAR_WIDTH)
#define SCR_BUFSIZ  (v_info.rows*ROW_LENGTH)


//#define gmode()	((int) v_info.mode)

#define V_UNKNOWN	0
#define V_MDA		1
#define V_CGA		2
#define V_EGA		3
#define V_VGA		4


#define MDA_BASE		((char FAR *)0xb0000000L)
#define COLOR_BASE		((char FAR *)0xb8000000L)

#define CHT_REG			0x0485
#define CGAB_REG		0x03d9

#define VBIOS			0x10
#define VBIOS_READSTATE	0x0f00
#define VBIOS_OVERSCAN	0x1001
#define VBIOS_READPAL	0x1007
#define VBIOS_FONTINFO	0x1130
#define VBIOS_ALTSELECT	0x1200
#define VBIOS_DISPCODE	0x1a00

struct svideo {		/* video adapter structure 		*/
	char FAR *base;	/* beginning of screen buffer	*/
	int adapter;	/* adapter type 				*/
	int mode;		/* video mode					*/
	int rows;		/* # screen rows				*/
	int cols;		/* # screen cols				*/
	int cht;		/* character height - not used, for debugging get_vinfo */
};



/* --------------------------------------------------------------------
 *  Static data
 * -------------------------------------------------------------------- */
static long f10timeout;     /* when was the f10 window opened?        */
static struct svideo v_info;
static char FAR *screen;    /* memory address for direct screen I/O   */
static char FAR *saveBuffer;    /* memory buffer for screen saves         */
static unsigned char srow, scolumn; /* static vars for saved position    */
static struct text_info t_info; /* Turbo C Text info                 */

/* --------------------------------------------------------------------
 *  Local routines
 * -------------------------------------------------------------------- */
static void border(int attr);
static void get_vinfo(struct svideo *v);

/* --------------------------------------------------------------------
 *  screenheight()  Number of rows on console
 * -------------------------------------------------------------------- */

unsigned char screenheight(void)
{
	return v_info.rows;
}

/* --------------------------------------------------------------------
 *  screenwidth()   Number of columns on console
 * -------------------------------------------------------------------- */

unsigned char screenwidth(void)
{
	return v_info.cols;
}

/* -------------------------------------------------------------------- */
/*      cls()  clears the screen                                        */
/* -------------------------------------------------------------------- */
void cls(void)
{
    /* scroll everything but kitchen sink */
	/* note:  errno reports an error after this call to scroll */
    scroll(v_info.rows - 1, 0, cfg.attr);
    position(0, 0);
}


/* -------------------------------------------------------------------- */
/*      cursoff()  make cursor disapear                                 */
/* -------------------------------------------------------------------- */
void cursoff(void)
{
    union REGS reg;

    reg.h.ah = 01;
    reg.h.bh = 00;
    reg.h.ch = 0x26;
    reg.h.cl = 7;
    int86(0x10, &reg, &reg);
}


/* -------------------------------------------------------------------- */
/*      curson()  Put cursor back on screen checking for adapter.       */
/* -------------------------------------------------------------------- */
void curson(void)
{
    union REGS regs;
    uchar st, en;

    if (gmode() == 7) {     /* Monocrone adapter */
        st = 12;
        en = 13;
    } else {            /* Color graphics adapter. */
        st = 6;
        en = 7;
    }

    regs.h.ah = 0x01;
    regs.h.bh = 0x00;
    regs.h.ch = st;
	regs.h.cl = en;
    int86(0x10, &regs, &regs);
}


/* -------------------------------------------------------------------- */
/*      gmode()  Check for monochrome or graphics.                      */
/* -------------------------------------------------------------------- */
int gmode(void)
{
    return ((int) v_info.mode);
}


/* -------------------------------------------------------------------- */
/*      position()  positions the cursor                                */
/* -------------------------------------------------------------------- */
void position(uchar row, uchar column)
{
    /* the turboc way */
    gotoxy((int) column + 1, (int) row + 1);
}


/* -------------------------------------------------------------------- */
/*      readpos()  returns current cursor position                      */
/* -------------------------------------------------------------------- */
void readpos(uchar *row, uchar *column)
{
    struct text_info t_buf, *t = &t_buf;

    /* the turboc way */
    gettextinfo(t);
	*row = (unsigned char) (t->cury - 1);
    *column = (unsigned char) (t->curx - 1);
}


/* -------------------------------------------------------------------- */
/*      scroll()  scrolls window up from specified line                 */
/* -------------------------------------------------------------------- */
void scroll(uchar row, uchar howmany, uchar attr)
{
    union REGS regs;
    uchar rw, col;

    readpos(&rw, &col);

    regs.h.al = howmany;    /* scroll how many lines           */

    regs.h.cl = 0;      /* row # of upper left hand corner */
    regs.h.ch = 0x00;       /* col # of upper left hand corner */
    regs.h.dh = row;        /* row # of lower left hand corner */
    regs.h.dl = 79;     /* col # of upper left hand corner */

    regs.h.ah = 0x06;       /* scroll window up interrupt      */
    regs.h.bh = attr;       /* char attribute for blank lines  */

    int86(0x10, &regs, &regs);

    /* put cursor back! */
    position(rw, col);
}


/* -------------------------------------------------------------------- */
/*      wscroll()  scrolls a window between specified lines             */
/* -------------------------------------------------------------------- */
void wscroll(int left, int top, int right, int bottom,
int lines, unsigned char attr)
{
    union REGS regs;
    uchar rw, col;

    readpos(&rw, &col);
    regs.h.al = lines;      /* scroll how many lines           */

    regs.h.ch = top;        /* row number, upper left            */
    regs.h.cl = left;       /* column number, upper left         */
    regs.h.dl = right;      /* column number, lower right        */
    regs.h.dh = bottom;     /* row number, lower right           */

    regs.h.ah = 0x06;       /* scroll window up interrupt      */
    regs.h.bh = attr;       /* char attribute for blank lines  */

    int86(0x10, &regs, &regs);

    /* put cursor back! */
    position(rw, col);
}

/* -------------------------------------------------------------------- */
/*      directstring() print a string with attribute at row             */
/* -------------------------------------------------------------------- */
void directstring(unsigned int row, char *str, uchar attr)
{
    register int i, j, l;

    l = strlen(str);

    for (i = (row * 160), j = 0; j < l; i += CHAR_WIDTH, j++) {
		screen[i] = str[j];
        screen[i + 1] = attr;
    }
}

/* -------------------------------------------------------------------- */
/*      directchar() print a char directly with attribute at row        */
/* -------------------------------------------------------------------- */
void directchar(char ch, uchar attr)
{
    int i;
    uchar row, col;

    readpos(&row, &col);

    i = (row * ROW_LENGTH) + (col * CHAR_WIDTH);

    screen[i] = ch;
    screen[i + 1] = attr;

    position(row, col + 1);
}

/* -------------------------------------------------------------------- */
/*      biosstring() print a string with attribute                      */
/* -------------------------------------------------------------------- */
void biosstring(unsigned int row, char *str, uchar attr)
{
    union REGS regs;
    union REGS temp_regs;
    register int i = 0;

    regs.h.ah = 9;      /* service 9, write character # attribute */
    regs.h.bl = attr;       /* character attribute                    */
    regs.x.cx = 1;      /* number of character to write           */
    regs.h.bh = 0;      /* display page                           */

    while (str[i]) {
        position((uchar) row, (uchar) i);   /* Move cursor to the correct position */
        regs.h.al = str[i]; /* set character to write     0x0900   */
        int86(0x10, &regs, &temp_regs);
        i++;
    }
}


/* -------------------------------------------------------------------- */
/*      bioschar() print a char with attribute                          */
/* -------------------------------------------------------------------- */
void bioschar(char ch, uchar attr)
{
    uchar row, col;
    union REGS regs;

    regs.h.ah = 9;      /* service 9, write character # attribute */
    regs.h.bl = attr;       /* character attribute                    */
    regs.h.al = ch;     /* write 0x0900                           */
    regs.x.cx = 1;      /* write 1 character                      */
    regs.h.bh = 0;      /* display page                           */
    int86(0x10, &regs, &regs);

    readpos(&row, &col);
    position(row, col + 1);
}

/* -------------------------------------------------------------------- */
/*      setscreen() set video mode flag 0 mono 1 cga                    */
/* -------------------------------------------------------------------- */
void setscreen(void)
{
    gettextinfo(&t_info);
	get_vinfo(&v_info);
    if (scrollpos != (v_info.rows - 6))
        scrollpos = v_info.rows - 2;

    screen = v_info.base;
	border(cfg.battr);

    if (cfg.bios) {
        charattr = bioschar;
        stringattr = biosstring;
    } else {
        charattr = directchar;
        stringattr = directstring;
    }

	ansiattr = cfg.attr;
}

/* -------------------------------------------------------------------- */
/*  save_screen() allocates a buffer and saves the screen               */
/* -------------------------------------------------------------------- */
void save_screen(void)
{
	saveBuffer = farcalloc((long) SCR_BUFSIZ, sizeof(char));
	if (saveBuffer)
		memcpy(saveBuffer, screen, SCR_BUFSIZ);
	readpos(&srow, &scolumn);
}

/* -------------------------------------------------------------------- */
/*   restore_screen() restores screen and free's buffer                 */
/* -------------------------------------------------------------------- */
void restore_screen(void)
{
    int size;

    size = SCR_BUFSIZ;
    setscreen();
    size = (SCR_BUFSIZ <= size) ? SCR_BUFSIZ : size;

    if (saveBuffer) {
        memcpy(screen, saveBuffer, SCR_BUFSIZ);
        farfree((void *) saveBuffer);
    }
    position(srow, scolumn);
}

/* -------------------------------------------------------------------- */
/*      help()  this toggles our help menu                              */
/* -------------------------------------------------------------------- */
void help(void)
{
    /* the turboc way */
    uchar row, col;
    int i,h;

    readpos(&row, &col);
	h = screenheight();
    if (scrollpos == (h - 2)) {   /* small window */
        if (row > (h - 6)) {
            scroll(h - 2, row - (h - 6),
            cfg.wattr);
            row = h - 6;
        }
        scrollpos = h - 6;    /* big window */

        f10timeout = cit_time();

        textattr((int) cfg.attr);
        for (i = 0; i < 4; i++) {
            position(h - 5 + i, 0);
            clreol();
        }

    } else {            /* big window */
        scrollpos = h - 2;    /* small window */

        f10timeout = cit_time();

        textattr((int) cfg.attr);
        for (i = 0; i < 4; i++) {
            position(h - 5 + i, 0);
            clreol();
        }
    }

    position(row, col);
}


/* -------------------------------------------------------------------- */
/*      updatehelp()  updates the help menu according to global vars    */
/* -------------------------------------------------------------------- */
void updatehelp(void)
{
    char bigline[81];
    uchar row, col;
	int h;

	h = screenheight();

    if (f10timeout < (cit_time() - 120L)) {
        help();
        return;
    }
    if (cfg.bios)
        cursoff();

    strcpy(bigline,
    "ษอออออออออออออออัอออออออออออออออัออออออออออออออัอออออออออออออออัอออออออออออออออป");

    readpos(&row, &col);

    position(h - 5, 0);

    (*stringattr) (h - 5, bigline, cfg.wattr);

    sprintf(bigline, "บ%sณ%sณ%sณ%sณ%sบ",
    " F1 þ Shutdown ", " F2 þ Startup  ", " F3 þ Request ",
    (anyEcho) ? " F4 þ Echo-Off " : " F4 þ Echo-On  ",
    (whichIO == CONSOLE) ? " F5  þ Modem   " : " F5  þ Console ");

    (*stringattr) (h - 4, bigline, cfg.wattr);

    sprintf(bigline, "บ%sณ%sณ%sณ%sณ%sบ",
    " F6 þ Sysop Fn ", (cfg.noBells) ? " F7 þ Bells-On " : " F7 þ Bells-Off",
    " F8 þ ChatMode", (cfg.noChat) ? " F9 þ Chat-On  " : " F9 þ Chat-Off ",
    " F10 þ Help    ");

    (*stringattr) (h - 3, bigline, cfg.wattr);

    strcpy(bigline,
    "ศอออออออออออออออฯอออออออออออออออฯออออออออออออออฯอออออออออออออออฯอออออออออออออออผ");

    (*stringattr) (h - 2, bigline, cfg.wattr);

    position(row >= (h - 6) ? scrollpos : row, col);

    if (cfg.bios)
        curson();
}


/* --------------------------------------------------------------------
 *	get_vinfo	return video adapter info (see struct svinfo)
 * -------------------------------------------------------------------- */

static void get_vinfo(struct svideo *v)
{
	union REGS regs;			/* registers for BIOS calls		*/

	regs.x.ax = VBIOS_READSTATE;/* get video mode	*/
	int86(VBIOS,&regs,&regs);
	v->mode = regs.h.al;
	
	v->cols = regs.h.ah;		/* fetch # cols		*/

	if (v->mode == 7)
		v->cht = 14;
	else
		v->cht = 8;
		
	regs.h.dl = 24;				/* in case # rows not returned	*/
    regs.h.bh = 0;
	regs.x.ax = VBIOS_FONTINFO;	/* get # rows		*/
	int86(VBIOS,&regs,&regs);
	v->rows = regs.h.dl;
	++v->rows;					/* normalize		*/
	
	regs.x.ax = VBIOS_DISPCODE;	/* get state		*/
	int86(VBIOS,&regs,&regs);
	if (regs.h.al == 0x1a) {
		switch(regs.h.bl) {
			
			case 1:
				v->adapter = V_MDA;
			break;
			
			case 2:
				v->adapter = V_CGA;
			break;
			
			case 4:				/* EGA color		*/
			case 5:				/* EGA mono			*/
				v->adapter = V_EGA;
			break;
			
			case 6:				/* PGA				*/
			case 7:				/* PS/2 VGA mono	*/
			case 8:				/* PS/2 VGA color	*/
			case 11:			/* PS/2 VGA mono	*/
			case 12:			/* PS/2 VGA color	*/
				v->adapter = V_VGA;
			break;
			
			case 0:
			default:
				v->adapter = V_UNKNOWN;
		}
	} else {
		regs.x.ax = VBIOS_ALTSELECT;	/* EGA/VGA alternate select	*/
		regs.h.bl = 0x10;		/* get video info	*/
		int86(VBIOS,&regs,&regs);
		if (regs.h.bl != 0x10)	/* if memory size set & not VGA must be EGA */
			v->adapter = V_EGA;
		else if (v->mode > 6)
			v->adapter = V_MDA;
		else
			v->adapter = V_CGA;
	}
	if (v->adapter == V_VGA || v->adapter == V_EGA) {
		v->cht = peek(0,CHT_REG);	/* read EGA/VGA register */
		switch (v->cht) {
			case 8:
				v->rows = (v->adapter == V_EGA) ? 43 : 50;
			break;
			case 10:
				v->rows = (v->adapter == V_EGA) ? 35 : 40;
			break;
			case 14:			/* EGA only? */
			case 16:			/* VGA only? */
				v->rows = 25;
			break;
		}
	}
	if (v->adapter == V_MDA) {
         v->base = MDA_BASE;
	} else {
         v->base = COLOR_BASE;
	}
}

/* --------------------------------------------------------------------
 *	border	Set the border attribute for CGA/EGA/VGA
 * -------------------------------------------------------------------- */

static void border(int attr)
{
	union REGS regs;
	int at;

	at = 0x0f & attr;

	if (v_info.adapter == V_CGA) {		/* if CGA display				*/
		outp(CGAB_REG, at);			/* set CGA border color			*/
	} else if (v_info.adapter == V_VGA) {	/* VGA	 					*/
		/* read the indexed palette register */
		regs.h.bl = (unsigned char) at;
		regs.x.ax = VBIOS_READPAL;	/* get pallette value for index	*/
		int86(VBIOS,&regs,&regs);	/* get color in BH	*/
		regs.x.ax = VBIOS_OVERSCAN;	/* Set overscan register service*/
		int86(VBIOS, &regs, &regs);	/* call BIOS to do it			*/
	} else if (v_info.adapter == V_EGA) {	/* EGA						*/
//		unsigned char rgb;
//
//		regs.x.ax =
//		regs.h.
//		rgb
		
//		regs.h.bh = rgb;			/* border attribute				*/
		regs.x.ax = VBIOS_OVERSCAN;	/* Set overscan register service*/
		regs.h.bh = at;				/* border attribute				*/
		int86(VBIOS, &regs, &regs);	/* call BIOS to do it			*/
	}
}

