/* -------------------------------------------------------------------- */
/*                              window.c                                */
/*            Machine dependent windowing routines for Citadel          */
/* -------------------------------------------------------------------- */

#include <alloc.h>
#include <dos.h>
#include <string.h>
#include <time.h>
#include <conio.h>				/* defines colors */
#include "ctdl.h"
#include "proto.h"
#include "global.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/*      connectcls()            clears the screen upon carrier detect   */
/*      update25()              updates the 25th line                   */
/*		char ansi(char c)		Handle ansi escape sequences            */
/* -------------------------------------------------------------------- */


/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/WINDOW.C_V  $
 * 
 *    Rev 1.47   01 Nov 1991 11:21:02   FJM
 * Added gl_ structures
 *
 *    Rev 1.46   21 Sep 1991 10:19:32   FJM
 * FredCit release
 *
 *    Rev 1.45   06 Jun 1991  9:19:52   FJM
 *
 *    Rev 1.44   29 May 1991 11:20:46   FJM
 * Removed redundent includes.
 *
 *    Rev 1.43   16 May 1991  8:43:50   FJM
 * Seperated out video-specific code.
 *
 *    Rev 1.40   17 Apr 1991 12:55:58   FJM
 *    Rev 1.34   10 Feb 1991 17:34:16   FJM
 *    Rev 1.31   28 Jan 1991 13:13:14   FJM
 *    Rev 1.29   19 Jan 1991 14:17:34   FJM
 * Fixed update25().  Added time display.
 *
 *    Rev 1.22   11 Jan 1991 12:43:56   FJM
 *    Rev 1.18   06 Jan 1991 12:46:10   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.17   27 Dec 1990 20:16:36   FJM
 * Changes for porting.
 *
 *    Rev 1.14   22 Dec 1990 13:39:06   FJM
 *    Rev 1.8   09 Dec 1990 15:24:30   FJM
 * ANSI/ISO color support for local screen.
 *
 *    Rev 1.7   07 Dec 1990 23:10:42   FJM
 *    Rev 1.5   24 Nov 1990  3:07:50   FJM
 * Changes for shell/door mode.
 *
 *    Rev 1.4   23 Nov 1990 13:25:32   FJM
 * Header change
 *
 *    Rev 1.2   17 Nov 1990 16:12:10   FJM
 * Added version control log header
 * -------------------------------------------------------------------- *
 *  EARLY HISTORY:
 *
 *  03/14/90    FJM     Added 43/50 line mode support.
 *  03/20/90    FJM     Added wscroll() for window scrolling.
 *  03/29/90    FJM     Fixed bug in save_screen() (cursor pos).
 *  04/02/90    FJM     Another fixed bug in save_screen() (cursor pos).
 *
 * -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*      connectcls()  clears the screen upon carrier detect             */
/* -------------------------------------------------------------------- */
void connectcls(void)
{
    cls();
    /* update25();	*/
	do_idle(0);
}

/* -------------------------------------------------------------------- */
/*      update25()  updates the 25th line according to global variables */
/* -------------------------------------------------------------------- */
#define NYMFIELD	29
/* one more place where IBM graphics characters are in strings.  Ugh */
#ifdef __MSDOS__
#define FILLCHAR1	177
#define FILLCHAR2	177
#define BARCHAR     179
#else
#define FILLCHAR1	'~'
#define FILLCHAR2	' '
#define BARCHAR     |
#endif
void update25(void)
{
    char string[83];
    char str2[80];
    label name;
    char flags[10];
    char carr[5];
	char chatstr[5];
	char prtstr[4];
	char reqstr[4];
    uchar row, col;
	char *p;
	int len;
	int h;

	h = screenheight();

    readpos(&row, &col);

    if (scrollpos == h - 6)
        updatehelp();

    if (cfg.bios)
        cursoff();

    readpos(&row, &col);

    if (loggedIn) {
		/* center nym in field of squiggly characters */
		memset (name,FILLCHAR1,sizeof(name)-1);
		len = strlen(logBuf.lbname);
		p = ((len) >= NYMFIELD) ?
			(name) : (name + ((NYMFIELD-len)/2+1));
		memcpy(p,logBuf.lbname,min(NYMFIELD,len));
    } else {		/*123456789012345678901234567890*/
		cit_strftime(name, NYMFIELD, cfg.vdatestamp, 0L);
        /*strcpy(name, "ÍÍÍÍÍÍ Not logged in ÍÍÍÍÍÍ");*/
    }
	name[NAMESIZE] = '\0';

	/* carrier field */
    if (justLostCarrier || ExitToMsdos)
        strcpy(carr, "JL");		/* Just loss carrier	*/
    else if (haveCarrier)
        strcpy(carr, "CD");		/* carrier detected		*/
    else
        strcpy(carr, "NC");		/* no carrier			*/

	/* chat field */
	memset(chatstr,FILLCHAR2,sizeof(chatstr)-1);
	if (cfg.noChat) {
		if (chatReq)
			strcpy(chatstr,"rcht");
	} else {
		if (chatReq)
			strcpy(chatstr,"RCht");
		else
			strcpy(chatstr,"Chat");
	}
	chatstr[sizeof(chatstr)-1] = '\0';
	
	/* prt field */
	memset(prtstr,FILLCHAR2,sizeof(prtstr)-1);
	if (printing) {
		strcpy(prtstr,"Prt");
	}
	prtstr[sizeof(prtstr)-1] = '\0';
	
	/* req field */
	memset(reqstr,FILLCHAR2,sizeof(reqstr)-1);
	if (sysReq) {
		strcpy(reqstr,"Req");
	}
	reqstr[sizeof(reqstr)-1] = '\0';
	
	/* flags field */
	memset(flags,FILLCHAR2,sizeof(flags)-1);
	flags[sizeof(flags)-1]='\0';

    if (gl_user.aide)
        flags[0] = 'A';
		
    if (gl_user.twit)
        flags[1] = 'T';
		
    if (gl_user.sysop)
        flags[5] = 'S';

    if (loggedIn) {
        if (logBuf.lbflags.PERMANENT)
            flags[2] = 'P';
        if (logBuf.lbflags.UNLISTED)
            flags[3] = 'U';
        if (logBuf.lbflags.NETUSER)
            flags[4] = 'N';
        if (logBuf.lbflags.NOMAIL)
            flags[6] = 'M';
    }
    sprintf(string, "%-*.*s³%s³%s³%5d baud³%c³%c%c%c%c³%s³%s³%s³%s",
		NYMFIELD,NYMFIELD,name,
		(whichIO == CONSOLE) ? "Console" : " Modem ",
		carr,bauds[speed],
		(disabled)    ? 'D'       : 'E',
		(cfg.noBells) ? FILLCHAR2 : '\x0e',
		(backout)     ? '\x9d'    : FILLCHAR2,
		(debug)       ? '\xe8'    : FILLCHAR2,
		(ConLock)     ? '\x0f'    : FILLCHAR2,
		chatstr,prtstr,reqstr,flags
	);

    sprintf(str2, "%-79s ", string);

    (*stringattr) (h - 1, str2, cfg.wattr);
/*
    sprintf(string, "%-*.*s%c%s%c%s%c%4d baud%c%s%c%c%c%c%c%c%c",
		NYMFIELD,NYMFIELD,name,BARCHAR,
		(whichIO == CONSOLE) ? "Console" : " Modem ",BARCHAR,
		carr,BARCHAR,bauds[speed],BARCHAR,
		(disabled)    ? "DS"      : "EN",BARCHAR,
		(cfg.noBells) ? FILLCHAR2 : '\x0e',
		(backout)     ? '\x9d'    : FILLCHAR2,
		(debug)       ? '\xe8'    : FILLCHAR2,
		(ConLock)     ? '\x0f'    : FILLCHAR2,BARCHAR);
		
    sprintf(str2, "%-79s ", string);
    (*stringattr) (h - 1, str2, cfg.wattr);
	
	sprintf(string,"%s%c",chatstr,BARCHAR);
	if (chatReq)
		(*stringattr) (h - 1, string, cfg.attr);
	else
		(*stringattr) (h - 1, string, cfg.wattr);
		
	sprintf(string,"%s%c%s%c%s",prtstr,BARCHAR,reqstr,BARCHAR,flags);
	(*stringattr) (h - 1, string, cfg.wattr);
*/
    position(row, col);

    if (cfg.bios)
        curson();

}


/* -------------------------------------------------------------------- */
/* Handle ansi escape sequences                                         */
/* -------------------------------------------------------------------- */
char ansi(char c)
{
    static char args[20], first = FALSE;
    static uchar c_x = 0, c_y = 0;
    uchar argc, a[5];
    int i;						/* array index				*/
	char rc;					/* return code				*/
    uchar x, y;					/* screen column and row	*/
    char *p;
	int h;

	h = screenheight();

    if (c == 27 /* ESC */ ) {
        strcpy(args, "");
        first = TRUE;
        rc = TRUE;
    } else if (first && c != '[') {
        first = FALSE;
        rc = FALSE;
    } else if (first && c == '[') {
        first = FALSE;
        rc = TRUE;
    } else if (isalpha(c)) {
        i = 0;
        p = args;
        argc = 0;
        while (*p) {
            if (isdigit(*p)) {
                char done = 0;

                a[argc] = (uchar) atoi(p);
                while (!done) {
                    p++;
                    if (!(*p) || !isdigit(*p))
                        done = TRUE;
                }
                argc++;
            } else
                p++;
        }
        switch (c) {
            case 'J':       /* cls */
                cls();
                /* update25();	*/
				do_idle(0);
                break;
            case 'K':       /* del to end of line */
                clreol();
                break;
            case 'm':
                for (i = 0; i < argc; i++) {

                    switch (a[i]) {
                        case 5:
                            ansiattr = ansiattr | 128;  /* blink */
                            break;
                        case 4:
                            ansiattr = ansiattr | 1;    /* underline */
                            break;
                        case 7:
                            ansiattr = 0x70;   /* Reverse Video */
                            /*ansiattr = cfg.wattr;    Reverse Vido */
                            break;
                        case 0:
                            ansiattr = cfg.attr;    /* default */
                            break;
                        case 1:
							/* ansiattr = cfg.cattr;       Bold */
                            ansiattr |= 8;  /* set bold */
                            break;

                        case 30:    /* black fg      */
                            ansiattr = (ansiattr & 0xf8) | BLACK;
                            break;
                        case 31:    /* red fg        */
                            ansiattr = (ansiattr & 0xf8) | RED;
                            break;
                        case 32:    /* green fg      */
                            ansiattr = (ansiattr & 0xf8) | GREEN;
                            break;
                        case 33:    /* yellow fg     */
                            ansiattr = (ansiattr & 0xf8) | BROWN;
                            break;
                        case 34:    /* blue fg       */
                            ansiattr = (ansiattr & 0xf8) | BLUE;
                            break;
                        case 35:    /* magenta fg    */
                            ansiattr = (ansiattr & 0xf8) | MAGENTA;
                            break;
                        case 36:    /* cyan fg       */
                            ansiattr = (ansiattr & 0xf8) | CYAN;
                            break;
                        case 37:    /* white fg      */
                            ansiattr = (ansiattr & 0xf8) | LIGHTGRAY;
                            break;

                        case 40:    /* black bg     */
                            ansiattr = (ansiattr & 0x8f) | (BLACK << 4);
                            break;
                        case 41:    /* red bg        */
                            ansiattr = (ansiattr & 0x8f) | (RED << 4);
                            break;
                        case 42:    /* green bg      */
                            ansiattr = (ansiattr & 0x8f) | (GREEN << 4);
                            break;
                        case 43:    /* yellow bg     */
                            ansiattr = (ansiattr & 0x8f) | (YELLOW << 4);
                            break;
                        case 44:    /* blue bg       */
                            ansiattr = (ansiattr & 0x8f) | (BLUE << 4);
                            break;
                        case 45:    /* magenta bg    */
                            ansiattr = (ansiattr & 0x8f) | (MAGENTA << 4);
                            break;
                        case 46:    /* cyan bg       */
                            ansiattr = (ansiattr & 0x8f) | (CYAN << 4);
                            break;
                        case 47:    /* white bg      */
                            ansiattr = (ansiattr & 0x8f) | (WHITE << 4);
                            break;

                        default:
                            break;
                    }
                }
                break;
            case 's':       /* save cursor */
                readpos(&c_x, &c_y);
                break;
            case 'u':       /* restore cursor */
                position(c_x, c_y);
                break;
            case 'A':
                readpos(&x, &y);
                if (argc)
                    x -= a[0];
                else
                    x--;
                x = x % (h - 1);
                position(x, y);
                break;
            case 'B':
                readpos(&x, &y);
                if (argc)
                    x += a[0];
                else
                    x++;
                x = x % (h - 1);
                position(x, y);
                break;
            case 'D':
                readpos(&x, &y);
                if (argc)
                    y -= a[0];
                else
                    y--;
                y = y % 80;
                position(x, y);
                break;
            case 'C':
                readpos(&x, &y);
                if (argc)
                    y += a[0];
                else
                    y++;
                y = y % 80;		/* shouldn't 80 be the terminal width? */
                position(x, y);
                break;
            case 'f':
            case 'H':
                if (!argc) {
                    position(0, 0);
                    break;
                }
                if (argc == 1) {
                    if (args[0] == ';') {
                        a[1] = a[0];
                        a[0] = 1;
                    } else {
                        a[1] = 1;
                    }
                    argc = 2;
                }
								/* shouldn't 80 be the terminal width? */
                if (argc == 2 && a[0] < h && a[1] < 80) {
                    position(a[0] - 1, a[1] - 1);
                    break;
                }
            default:
            {
                char str[80];

                sprintf(str, "[%s%c %d %d %d ", args, c, argc, a[0], a[1]);
                (*stringattr) (0, str, cfg.wattr);
            }
                break;
        }
        if (debug) {
            char str[80];

            sprintf(str, "[%s%c %d %d %d ", args, c, argc, a[0], a[1]);
            (*stringattr) (0, str, cfg.wattr);
        }
        rc = FALSE;
    } else {
		i = strlen(args);
		args[i] = c;
		args[i + 1] = NULL;
        rc = TRUE;
    }
	return rc;
}

