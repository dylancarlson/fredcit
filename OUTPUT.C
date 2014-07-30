/* $Header:   D:/VCS/FCIT/OUTPUT.C_V   1.43   01 Nov 1991 11:20:44   FJM  $ */
/* -------------------------------------------------------------------- */
/*  OUTPUT.C                  Citadel                                   */
/* -------------------------------------------------------------------- */
/*  This file contains the output functions                             */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Includes                                                            */
/* -------------------------------------------------------------------- */
#include <string.h>
#include <stdarg.h>
#include "ctdl.h"

#ifndef ATARI_ST
#include <conio.h>
#endif

#include "proto.h"
#include "global.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  getWord()       Gets the next word from the buffer and returns it   */
/*  mFormat()       Outputs a string to modem and console w/ wordwrap   */
/*  putWord()       Writes one word to modem and console, w/ wordwrap   */
/*  doBS()          does a backspace to modem & console                 */
/*  doCR()          does a return to both modem & console               */
/*  dospCR()        does CR for entry of initials & pw                  */
/*  doTAB()         prints a tab to modem & console according to flag   */
/*  oChar()         is the top-level user-output function (one byte)    */
/*  updcrtpos()     updates crtColumn according to character            */
/*  mPrintf()       sends formatted output to modem & console           */
/*  cPrintf()       send formatted output to console                    */
/*  cCPrintf()      send formatted output to console, centered          */
/* -------------------------------------------------------------------- */


/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/OUTPUT.C_V  $
 * 
 *    Rev 1.43   01 Nov 1991 11:20:44   FJM
 * Added gl_ structures
 *
 *    Rev 1.42   08 Jul 1991 16:19:36   FJM
 *
 *    Rev 1.41   15 Jun 1991  8:44:54   FJM
 *
 *    Rev 1.40   06 Jun 1991  9:19:40   FJM
 *
 *    Rev 1.39   29 May 1991 11:19:52   FJM
 * Removed redundent includes, fixed up header.
 *
 *    Rev 1.38   27 May 1991 11:43:16   FJM
 *    Rev 1.23   18 Jan 1991 16:51:58   FJM
 *    Rev 1.19   11 Jan 1991 12:43:42   FJM
 *    Rev 1.15   06 Jan 1991 12:44:56   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.14   27 Dec 1990 20:16:48   FJM
 * Changes for porting.
 *
 *    Rev 1.11   22 Dec 1990 13:39:24   FJM
 *    Rev 1.8   15 Dec 1990 21:21:00   FJM
 * Modifed the way wrapping is done.
 * Added function mtPrintf()
 * Reduced input filtering in normalizeString
 *
 *    Rev 1.7   09 Dec 1990 15:24:04   FJM
 * ANSI/ISO color support added!
 *
 *    Rev 1.6   07 Dec 1990 23:10:38   FJM
 *    Rev 1.4   23 Nov 1990 13:25:22   FJM
 * Header change
 *
 *    Rev 1.2   17 Nov 1990 16:11:32   FJM
 * Added version control log header
 * -------------------------------------------------------------------- */

/* --------------------------------------------------------------------
 *  HISTORY:
 *
 *  05/26/89    (PAT)   Created from MISC.C to break that module into
 *                      more managable and logical peices. Also draws
 *                      off MODEM.C and FORMAT.C
 *  03/13/90     FJM    Added IBM Graphics
 *  03/15/90    {zm}    It's called PsYcHo ChIcKeN, so you'll know.
 *  06/16/90     FJM    Made IBM Graphics characters a seperate option.
 *
 * -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  External data                                                       */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Static data/functions                                               */
/* -------------------------------------------------------------------- */
static void ansiCode(char *str);
static char buff[256];		/* prevent heap overflows */
static char buff2[132];		/* prevent heap overflows */

/* -------------------------------------------------------------------- */
/*  getWord()       Gets the next word from the buffer and returns it   */
/* -------------------------------------------------------------------- */
int getWord(char *dest, char *source, register int offset, int lim)
{
    register int i;     /* , j; */

    /* skip leading blanks if any  */
    for (i = 0; source[offset + i] == ' ' && (i < lim); i++);

    /* step over word              */
    for (; source[offset + i] != ' ' && (i < lim) && source[offset + i]; i++);

    /* pick up any trailing blanks */
    for (; source[offset + i] == ' ' && (i < lim); i++);

    /* copy word over */
    /* for (j  = 0; j < i; j++) */
    strncpy(dest, source + offset, i);
    /* dest[j] = source[offset+j]; */
    dest[i] = 0;        /* null to tie off string */

    return (offset + i);
}

/* -------------------------------------------------------------------- */
/*  mFormat()       Outputs a string to modem and console w/ wordwrap   */
/* -------------------------------------------------------------------- */
#define MAXWORD 256
void mFormat(char *string)
{
    char wordBuf[MAXWORD + 8];
    int i;

    for (i = 0; string[i] &&
    (outFlag == OUTOK || outFlag == IMPERVIOUS || outFlag == OUTPARAGRAPH);) {
        i = getWord(wordBuf, string, i, MAXWORD);
        putWord(wordBuf);
        if (mAbort())
            return;
    }
}

/* -------------------------------------------------------------------- */
/*  putWord()       Writes one word to modem and console, w/ wordwrap   */
/* -------------------------------------------------------------------- */
void putWord(char *st)
{
    char *s;
    int newColumn;

    setio(whichIO, echo, outFlag);
    /* calculate word end */
    for (newColumn = crtColumn, s = st; *s; s++) {
        if (*s == '\t')
            while ((++newColumn % 8) != 1);
        else if (*s == 1)
            --newColumn;    /* don't count ANSI codes  */
        else if (*s > ' ')  /* don't count what we don't print */
            ++newColumn;
    }
    if (newColumn >= gl_term.termWidth) /* Wrap words that don't fit */
        doCR();

    for (; *st; ++st) {
    /* check for Ctrl-A codes */
        if (*st == 1) {
            if (*++st) {    /* must have something after ^A */
                termCap(*st);
                continue;
            } else {
                break;
            }
        }
    /* worry about words longer than a line:   */
        if (crtColumn > gl_term.termWidth)
            doCR();

        if ((prevChar != '\n') || (*st > ' ')) {
            oChar(*st);
        } else {
        /* end of paragraph: */
            if (outFlag == OUTPARAGRAPH) {
                outFlag = OUTOK;
            }
            doCR();
            oChar(*st);
        }
    }
}

/* -------------------------------------------------------------------- */
/*  termCap()       Does a terminal command                             */
/* -------------------------------------------------------------------- */
void termCap(char c)
{
    if  (!gl_term.ansiOn)
        return;

	if (debug)					/* FJM - debug/display ANSI sequences */
		cPrintf("^A%c",c);
    switch (c) {
    /* foreground color settings */
        case TERM_BLACK_FG:
            ansiCode("30m");
            ansiattr = (ansiattr & 0xf8) | BLACK;
            break;
        case TERM_RED_FG:
            ansiCode("31m");
            ansiattr = (ansiattr & 0xf8) | RED;
            break;
        case TERM_GREEN_FG:
            ansiCode("32m");
            ansiattr = (ansiattr & 0xf8) | GREEN;
            break;
        case TERM_YELLOW_FG:
            ansiCode("33m");
            ansiattr = (ansiattr & 0xf8) | BROWN;
            break;
        case TERM_BLUE_FG:
            ansiCode("34m");
            ansiattr = (ansiattr & 0xf8) | BLUE;
            break;
        case TERM_MAGENTA_FG:
            ansiCode("35m");
            ansiattr = (ansiattr & 0xf8) | MAGENTA;
            break;
        case TERM_CYAN_FG:
            ansiCode("36m");
            ansiattr = (ansiattr & 0xf8) | CYAN;
            break;
        case TERM_WHITE_FG:
            ansiCode("37m");
            ansiattr = (ansiattr & 0xf8) | LIGHTGRAY;
            break;

    /* background color settings */
        case TERM_BLACK_BG:
            ansiCode("40m");
            ansiattr = (ansiattr & 0x8f) | (BLACK << 4);
            break;
        case TERM_RED_BG:
            ansiCode("41m");
            ansiattr = (ansiattr & 0x8f) | (RED << 4);
            break;
        case TERM_GREEN_BG:
            ansiCode("42m");
            ansiattr = (ansiattr & 0x8f) | (GREEN << 4);
            break;
        case TERM_YELLOW_BG:
            ansiCode("43m");
            ansiattr = (ansiattr & 0x8f) | (BROWN << 4);
            break;
        case TERM_BLUE_BG:
            ansiCode("44m");
            ansiattr = (ansiattr & 0x8f) | (BLUE << 4);
            break;
        case TERM_MAGENTA_BG:
            ansiCode("45m");
            ansiattr = (ansiattr & 0x8f) | (MAGENTA << 4);
            break;
        case TERM_CYAN_BG:
            ansiCode("46m");
            ansiattr = (ansiattr & 0x8f) | (CYAN << 4);
            break;
        case TERM_WHITE_BG:
            ansiCode("47m");
            ansiattr = (ansiattr & 0x8f) | (LIGHTGRAY << 4);
            break;

    /* attribute settings */
        case TERM_BLINK:
            ansiCode("5m");
            ansiattr = ansiattr | 128;
            break;
        case TERM_REVERSE:
            ansiCode("7m");
            ansiattr = cfg.wattr;   /* these cfg references have to change now */
            break;
        case TERM_BOLD:
            ansiCode("1m");
            ansiattr = cfg.cattr;
            break;
        case TERM_UNDERLINE:
            ansiCode("4m");
            ansiattr = cfg.uttr;
            break;
        case TERM_NORMAL:
        default:
            ansiCode("0m");
            ansiattr = cfg.attr;
            break;
    }
}

/* -------------------------------------------------------------------- */
/*   ansiCode() adds the escape if needed                               */
/* -------------------------------------------------------------------- */
static void ansiCode(char *str)
{
    char tmp[30], *p;

    if (!gl_term.ansiOn)
        return;

    sprintf(tmp, "%c[%s", 27, str);

    p = tmp;

    while (*p) {
        outMod(*p++);
    }
}

/* -------------------------------------------------------------------- */
/*  doBS()          does a backspace to modem & console                 */
/* -------------------------------------------------------------------- */
void doBS(void)
{
    oChar('\b');
    oChar(' ');
    oChar('\b');
}

/* -------------------------------------------------------------------- */
/*  doCR()          does a return to both modem & console               */
/* -------------------------------------------------------------------- */
void doCR(void)
{
    static numLines = 0;

	do_idle(0);					/* clear idle display */
	
    crtColumn = 1;
    setio(whichIO, echo, outFlag);

    domcr();
    doccr();

    if (printing)
        fprintf(printfile, "\n");

    prevChar = ' ';

    /* pause on full screen */
    if (logBuf.linesScreen) {
        if (outFlag == OUTOK) {
            numLines++;
            if (numLines == logBuf.linesScreen) {
                outFlag = OUTPAUSE;
                mAbort();
                numLines = 0;
            }
        } else {
            numLines = 0;
        }
    } else {
        numLines = 0;
    }
}

/* -------------------------------------------------------------------- */
/*  dospCR()        does CR for entry of initials & pw                  */
/* -------------------------------------------------------------------- */
void dospCR(void)
{
    char oldecho;

    oldecho = echo;

    echo = BOTH;
    setio(whichIO, echo, outFlag);

    if (cfg.nopwecho == 1)
        doCR();
    else {
        if (onConsole) {
            if (gotCarrier())
                domcr();
        } else
            doccr();
    }
    echo = oldecho;
}

/* -------------------------------------------------------------------- */
/*  doTAB()         prints a tab to modem & console according to flag   */
/* -------------------------------------------------------------------- */
void doTAB(void)
{
    int column, column2;

    column = crtColumn;
    column2 = crtColumn;

    do {
        outCon(' ');
    } while ((++column % 8) != 1);

    if (haveCarrier) {
        if (gl_term.termTab)
            outMod('\t');
        else
            do {
                outMod(' ');
            }
        while ((++column2 % 8) != 1);
    }
    updcrtpos('\t');
}

/* -------------------------------------------------------------------- */
/*  echocharacter() echos bbs input according to global flags           */
/* -------------------------------------------------------------------- */
void echocharacter(char c)
{
    setio(whichIO, echo, outFlag);

    if (echo == NEITHER) {
        if (echoChar != '\0') {
            echo = BOTH;
            if (c == '\b')
                doBS();
            else if (c == '\t')
                doTAB();
            else if (c == '\n') {
                echo = CALLER;
                doCR();
            } else
                oChar(echoChar);
            echo = NEITHER;
        }
    } else if (c == '\b')
        doBS();
    else if (c == '\n')
        doCR();
    else
        oChar(c);
}

/* -------------------------------------------------------------------- */
/*  oChar()         is the top-level user-output function (one byte)    */
/*        sends to modem port and console both                          */
/*        does conversion to upper-case etc as necessary                */
/*        in "debug" mode, converts control chars to uppercase letters  */
/*      Globals modified:       prevChar                                */
/* -------------------------------------------------------------------- */
void oChar(unsigned char c)
{
    static int UpDoWn = TRUE;   /* You don't want to know... */

    /* it's >not< a Bunny filter */

    /* translate on output  FJM     */
    if (!gl_term.IBMOn)
        c = filt_out[c];

    prevChar = c;       /* for end-of-paragraph code    */

    if (c == 1)
        c = 0;
	else if (c == '\t') {
        doTAB();
    } else {
		if (c == '\n')
			c = ' ';        /* doCR() handles real newlines */
		else if (gl_term.termUpper)
			c = (char) toupper(c);
		else if (backout) {      /* You don't want to know. Really. */
			if (UpDoWn)
				c = (char) toupper(c);
			else
				c = (char) tolower(c);
			UpDoWn = !UpDoWn;
		}
		/* show on console */
		if (console)
			outCon(c);
	
		/* show on printer */
		if (printing)
			fputc(c, printfile);
	
		/* send out the modem  */
		if (haveCarrier && modem)
			outMod(c);
	
		updcrtpos(c);       /* update CRT column count */
	}
}

/* -------------------------------------------------------------------- */
/*  updcrtpos()     updates crtColumn according to character            */
/* -------------------------------------------------------------------- */
void updcrtpos(char c)
{
    if  (c == '\b')
        crtColumn--;
    else if (c == '\t')
        while ((++crtColumn % 8) != 1);
    else if ((c == '\n') || (c == '\r'))
        crtColumn = 1;
    else
        crtColumn++;
}

/* -------------------------------------------------------------------- */
/*  mPrintf()       sends formatted output to modem & console           */
/* -------------------------------------------------------------------- */
void mPrintf(char *fmt,...)
{
    va_list ap;

    va_start(ap, fmt);
    vsprintf(buff, fmt, ap);
    va_end(ap);

    mFormat(buff);
}

/* --------------------------------------------------------------------
 *  mpPrintf()       sends menu prompt output to modem & console
 * -------------------------------------------------------------------- */
void mpPrintf(char *fmt,...)
{
    va_list ap;
	char *p,*p2;
	int markers = 0;

    va_start(ap, fmt);
    vsprintf(buff, fmt, ap);
    va_end(ap);

	if (gl_term.ansiOn) {
		for (p=buff,p2=buff2;*p;++p,++p2) {
			if (*p == '<' && markers == 0) {	/* parse first marker */
				*p2++ = 1;		/* ^A */
				*p2 = TERM_BOLD;
				++markers;
			} else if (*p == '>' && markers == 1) {
				*p2++ = 1;		/* ^A */
				*p2 = TERM_NORMAL;
				++markers;
			} else {
				*p2 = *p;
			}
		}
		*p2 = '\0';
		mFormat(buff2);
	} else
		mFormat(buff);
}

/* --------------------------------------------------------------------
 *  mtPrintf()      Sends formatted output to modem & console
 *                  with termCap attributes.  Resets attributes when
 *                  done.
 * -------------------------------------------------------------------- */
void mtPrintf(char attr, char *fmt,...)
{
    va_list ap;

    termCap(attr);
    va_start(ap, fmt);
    vsprintf(buff, fmt, ap);
    va_end(ap);

    mFormat(buff);
    termCap(TERM_NORMAL);
}

/* -------------------------------------------------------------------- */
/*  cPrintf()       send formatted output to console                    */
/* -------------------------------------------------------------------- */
void cPrintf(char *fmt,...)
{
    char *buf = buff;
    va_list ap;

    va_start(ap, fmt);
    vsprintf(buff, fmt, ap);
    va_end(ap);

    while (*buf) {
        outCon(*buf++);
    }
}

/* -------------------------------------------------------------------- */
/*  cCPrintf()      send formatted output to console, centered          */
/* -------------------------------------------------------------------- */
void cCPrintf(char *fmt,...)
{
    va_list ap;
    int i;

    va_start(ap, fmt);
    vsprintf(buff, fmt, ap);
    va_end(ap);

    i = (80 - strlen(buff)) / 2;

    strrev(buff);

    while (i--)
        strcat(buff, " ");

    strrev(buff);

    (*stringattr) (wherey() - 1, buff, cfg.attr);
}

/* -------------------------------------------------------------------- */
/*  old  cCPrintf()      send formatted output to console, centered     */
/* -------------------------------------------------------------------- */
/*
void cCPrintf(char *fmt, ... )
{
    char register *buf = buff;
    va_list ap;
    int i;

    va_start(ap, fmt);
    vsprintf(buff, fmt, ap);
    va_end(ap);

    i = (80 - strlen(buff)) / 2;

    while(i--)
    {
        outCon(' ');
    }

    while(*buf)
    {
        outCon(*buf++);
    }
}
*/

/* EOF */
