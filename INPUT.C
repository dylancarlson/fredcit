/* --------------------------------------------------------------------
 *  INPUT.C                   Citadel
 * --------------------------------------------------------------------
 *  This file contains the input functions
 * -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Includes                                                            */
/* -------------------------------------------------------------------- */
#include <string.h>
#include <time.h>
#include "ctdl.h"

#ifndef ATARI_ST
#include <conio.h>
#endif

#include "proto.h"
#include "global.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  getNormStr()    gets a string and normalizes it. No default.        */
/*  getNumber()     Get a number in range (top, bottom)                 */
/*  getString()     gets a string from user w/ prompt & default, ext.   */
/*  getYesNo()      Gets a yes/no/abort or the default                  */
/*  BBSCharReady()  Returns if char is avalible from modem or con       */
/*  iChar()         Get a character from user. This also indicated      */
/*                  timeout, carrierdetect, and a host of other things  */
/*  setio()         set io flags according to whichio,echo and outflag  */
/*  systimeout()    returns 1 if time has been exceeded                 */
/* -------------------------------------------------------------------- */


/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/INPUT.C_V  $
 * 
 *    Rev 1.48   01 Nov 1991 11:20:16   FJM
 * Added gl_ structures
 *
 *    Rev 1.47   21 Sep 1991 12:26:04   FJM
 * FredCit release
 *
 *    Rev 1.46   21 Sep 1991 10:19:00   FJM
 * FredCit release
 *
 *    Rev 1.45   06 Jun 1991  9:19:12   FJM
 *
 *    Rev 1.44   27 May 1991 11:42:48   FJM
 *
 *    Rev 1.43   16 May 1991  8:43:04   FJM
 * Speedups to iChar()
 *
 *    Rev 1.40   17 Apr 1991 12:55:24   FJM
 *    Rev 1.35   11 Feb 1991 17:26:20   FJM
 * Made screen saver ignore modem responses with no carrier.
 *
 *    Rev 1.31   05 Feb 1991 14:31:28   FJM
 * Added screen saver routines.
 *
 *    Rev 1.30   28 Jan 1991 13:12:12   FJM
 * Added getASCString().
 *
 *    Rev 1.28   19 Jan 1991 14:15:22   FJM
 * Simplified normalizePW()
 *
 *    Rev 1.25   18 Jan 1991 16:50:40   FJM
 * Allow ANSI in getString
 *
 *    Rev 1.21   11 Jan 1991 12:43:12   FJM
 *    Rev 1.20   06 Jan 1991 16:50:30   FJM
 *    Rev 1.17   06 Jan 1991 12:44:50   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.16   27 Dec 1990 20:16:32   FJM
 * Changes for porting.
 *
 *    Rev 1.13   22 Dec 1990 13:39:00   FJM
 *    Rev 1.8   16 Dec 1990 18:13:08   FJM
 *    Rev 1.5   24 Nov 1990  3:07:30   FJM
 * Changes for shell/door mode.
 *
 *    Rev 1.4   23 Nov 1990 13:25:00   FJM
 * Header change
 *
 *    Rev 1.2   17 Nov 1990 16:11:34   FJM
 * Added version control log header
 *
 * --------------------------------------------------------------------
 *
 *  Early HISTORY:
 *
 *  05/26/89    (PAT)   Created from MISC.C to break that module into
 *                      more managable and logical pieces. Also draws
 *                      off MODEM.C
 *  03/12/90    FJM     Added IBM Graphics character translation
 *  03/15/90    {zm}    Use FILESIZE (20) instead of NAMESIZE.
 *  03/16/90    FJM     Fixed bug in iChar that prevented graphics entry
 *  03/18/90    FJM     Entry bug fix (again?)
 *  03/19/90    FJM     Linted & partial cleanup
 *  07/23/90    FJM     Prevent entry of 0xff in iChar
 *  08/06/90    FJM     Made "Sleeping? Call again :-)" hang up logged
 *                      in users in door mode.
 *  08/07/90    FJM     Corrected sleeping fix to exit cit, instead of
 *                      hanging up the phone line.
 *
 *  09/19/90    FJM     Made prompts more polite :)
 * -------------------------------------------------------------------- */

static int NEAR systimeout(long timer);

/* -------------------------------------------------------------------- */
/*  External data                                                       */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  getNormStr()    gets a string and normalizes it. No default.        */
/* -------------------------------------------------------------------- */
void getNormStr(char *prompt, char *buffer, int size, char doEcho)
{
    getString(prompt, buffer, size, FALSE, doEcho, "");
    normalizeString(buffer);
}

/* -------------------------------------------------------------------- */
/*  getNumber()     Get a number in range (top, bottom)                 */
/* -------------------------------------------------------------------- */
long getNumber(char *prompt, long bottom, long top, long dfaultnum)
{
    long try;
    char numstring[FILESIZE];
    char buffer[20];
    char *dfault;

    dfault = ltoa(dfaultnum, buffer, 10);

    if (dfaultnum == -1L)
        dfault[0] = '\0';

    do {
        getString(prompt, numstring, FILESIZE, FALSE, ECHO, dfault);
        try = atol(numstring);
        if (try < bottom)
            mPrintf("Sorry, must be at least %ld\n", bottom);
        if (try > top)
            mPrintf("Sorry, must be no more than %ld\n", top);
    } while ((try < bottom || try > top) && !ExitToMsdos &&
		(haveCarrier || onConsole));
    return (long) try;
}

/* -------------------------------------------------------------------- */
/*  getString()     gets a string from user w/ prompt & default, ext.   */
/* -------------------------------------------------------------------- */
void getString(char *prompt, char *buf, int lim, char QuestIsSpecial,
char doEcho, char *dfault)
{
	getASCString(prompt,buf,lim,QuestIsSpecial,doEcho,dfault,0);
}

/* --------------------------------------------------------------------
 *  getASCString()  gets a string from user w/ prompt & default, ext.
 *					w/option to inhibit non-ASCII entry
 * -------------------------------------------------------------------- */
void getASCString(char *prompt, char *buf, int lim, char QuestIsSpecial,
char doEcho, char *dfault, char ASCIIOnly)
/* char *prompt;           Enter PROMPT */
/* char *buf;              Where to put it */
/* char doEcho;            To echo, or not to echo, that is the question */
/* int  lim;               max # chars to read */
/* char QuestIsSpecial;    Return immediately on '?' input? */
/* char *dfault;           Default for the lazy. */
/* char ASCIIOnly;         Restrict to ' ' thru '~'? */
{
    char c, oldEcho, tmpEcho, errors = 0;
    int i;

    outFlag = IMPERVIOUS;

    if (strlen(prompt)) {
        doCR();

        if (strlen(dfault)) {
            sprintf(gprompt, "Please enter %s [%s]:", prompt, dfault);
        } else {
            sprintf(gprompt, "Please enter %s:", prompt);
        }

        mtPrintf(TERM_REVERSE, gprompt);
        mPrintf(" ");

        dowhat = PROMPT;
    } else {
        mPrintf(": ");
	}
    oldEcho = echo;

    if (!doEcho) {
        if (!cfg.nopwecho)
            echo = CALLER;

        else if (cfg.nopwecho == 1) {
            echo = NEITHER;
            echoChar = '\0';
        } else {
            echo = NEITHER;
            echoChar = cfg.nopwecho;
        }
    }
    i = 0;
    while (c = (char) iChar(), c != 10  /* NEWLINE */
    && i < lim && (!ExitToMsdos && (haveCarrier || onConsole))) {
        outFlag = OUTOK;

        if ((echoChar >= '0') && (echoChar <= '9')) {
            echoChar++;
            if (echoChar > '9')
                echoChar = '0';
        }
    /* handle delete chars: */
        if (c == '\b') {
            if ((echoChar >= '0') && (echoChar <= '9')) {
                echoChar--;
                if (echoChar < '0')
                    echoChar = '9';
            }
            if ((echo != NEITHER) || (cfg.nopwecho != 1)) {
                if (buf[i - 1] == '\t') {
                    while ((crtColumn % 8) != 1)
                        doBS();
                }
            }
            if (i > 0) {
                i--;

                if ((echoChar >= '0') && (echoChar <= '9')) {
                    echoChar--;
                    if (echoChar < '0')
                        echoChar = '9';
                }
            } else {
                if ((echo != NEITHER) || (cfg.nopwecho != 1))
                    oChar(' ');
                oChar('\a');
            }
        } else if (c == 1) {	/* Cit ANSI */
			if (gl_term.ansiOn && !ASCIIOnly) {
				if (doEcho == ECHO) {
					buf[i++] = c;
					tmpEcho = echo;
					echo = NEITHER;
					c = iChar();
					echo = tmpEcho;
					termCap(c);
					buf[i++] = c;
				} else {
					/* don't let them enter ANSI they can't see */
					oChar ('\a');
				}
			} else
                oChar('\a');
		} else if (!ASCIIOnly || ((c >= ' ') && (c <= '~'))) {
            buf[i++] = c;
        }

        if (i >= lim) {
            oChar(7 /* bell */ );
            if ((echo != NEITHER) || (cfg.nopwecho != 1)) {
                doBS();
            }
            i--;

            if ((echoChar >= '0') && (echoChar <= '9')) {
                echoChar--;
                if (echoChar < '0')
                    echoChar = '9';
            }
            errors++;
            if (errors > 15 && !onConsole) {
                drop_dtr();
            }
        }
    /* kludge to return immediately on single '?': */
        if (QuestIsSpecial && *buf == '?') {
            doCR();
            break;
        }
    }

    echo = oldEcho;
    buf[i] = '\0';
    echoChar = '\0';

    if (strlen(dfault) && !strlen(buf))
        strcpy(buf, dfault);

    dowhat = DUNO;
}


/* -------------------------------------------------------------------- */
/*  getYesNo()      Gets a yes/no/abort or the default                  */
/* -------------------------------------------------------------------- */
int getYesNo(char *prompt, char dfault)
{
    int toReturn;
    char c;
    char oldEcho;

/*    while (MIReady()) getMod(); */

    doCR();
    toReturn = ERROR;

    outFlag = IMPERVIOUS;
    sprintf(gprompt, "%s? ", prompt);

    switch (dfault) {
        case 0:
            strcat(gprompt, "(Y/N)[N]:");
            break;
        case 1:
            strcat(gprompt, "(Y/N)[Y]:");
            break;
        case 2:
            strcat(gprompt, "(Y/N/A)[A]:");
            break;
        case 3:
            strcat(gprompt, "(Y/N/A)[N]:");
            break;
        case 4:
            strcat(gprompt, "(Y/N/A)[Y]:");
            break;
        default:
            strcat(gprompt, "(Y/N)[N]:");
            dfault = 0;
            break;
    }

    mtPrintf(TERM_REVERSE, gprompt);
    mPrintf(" ");

    dowhat = PROMPT;

    do {
        oldEcho = echo;
        echo = NEITHER;
        c = (char) iChar();
        echo = oldEcho;

        if ((c == '\n') || (c == '\r')) {
            if (dfault == 1 || dfault == 4)
                c = 'Y';
            if (dfault == 0 || dfault == 3)
                c = 'N';
            if (dfault == 2)
                c = 'A';
        }
        switch (toupper(c)) {
            case 'Y':
                mPrintf(gl_str.yes);
                doCR();
                toReturn = 1;
                break;
            case 'N':
                mPrintf(gl_str.no);
                doCR();
                toReturn = 0;
                break;
            case 'A':
                if (dfault > 1) {
                    mPrintf("Abort");
                    doCR();
                    toReturn = 2;
                }
                break;
        }
    } while (toReturn == ERROR && (!ExitToMsdos && (haveCarrier || onConsole)));

    outFlag = OUTOK;
    dowhat = DUNO;
    return toReturn;
}

/* -------------------------------------------------------------------- */
/*  BBSCharReady()  Returns if char is avalible from modem or con       */
/* -------------------------------------------------------------------- */
int BBSCharReady(void)
{
    return ((haveCarrier && (whichIO == MODEM) && MIReady()) ||
    KBReady());
}

/* -------------------------------------------------------------------- */
/*  iChar()         Get a character from user. This also indicated      */
/*                  timeout, carrierdetect, and a host of other things  */
/* -------------------------------------------------------------------- */
int iChar(void)
{
    unsigned char c=0;	/* char from keyboard or modem				*/
    long idle_timer=0L;	/* how since last char, or 0L if not idle	*/
	long ad_timer=0L;	/* how long since last ad, 0L if not idle	*/
	long current;		/* current time for idle compare			*/
    char str[40];       /* for baud detect							*/

	/* detectflag is set while processing baud rate detect	*/
	
    if (justLostCarrier || ExitToMsdos)
        return 0;       /* ugly patch   */

    sysopkey = FALSE;       /* go into sysop mode from main()? */
    eventkey = FALSE;       /* fo an event? */

    while (TRUE) {
		int car;
		car = gotCarrier();
    /* just in case person hangs up in console mode */
        if (detectflag /* && !onConsole */ && !car) {
            Initport();
            detectflag = FALSE;
        }
		/* if not on console, not modem with carrier, or exiting; leave loop */
        if (!carrier() || ExitToMsdos) {
			c = 0;
			break;
		}

		/* got carrier in console mode, switched to modem 			*/
        if (detectflag && !onConsole && car) {
            carrdetect();		/* also resets idle display			*/
            detectflag = FALSE;	/* not detecting baud rate			*/
            c = 0;				/* no bbs char to process			*/
			break;
        } else if (KBReady()) {	/* have keyboard input				*/
			if (idle_timer)		/* reset idle display if already idle */
				do_idle(0);
            c = getch();		/* grab our character from keyboard	*/
            getkey = 0;			/* clear keyboard ready flag		*/
            ++received;     	/* increment received char count 	*/
            break;
        } else if (MIReady() && !detectflag) {	/* baud rate detect	*/
            if (!modStat && (cfg.dumbmodem == 0)) {
				/* seek baud rate */
                if (getModStr(str)) {
                    c = TRUE;

                    switch (atoi(str)) {
                        case 13:    /* 9600   13 or 17   */
                        case 17:
                            baud(4);
                            break;

                        case 10:    /* 2400   10 or 16   */
                        case 16:
                            baud(2);
                            break;

                        case 5: /* 1200   15 or  5   */
                        case 15:
                            baud(1);
                            break;

                        case 1: /* 300    1  */
                            baud(0);
                            break;

                        case 2: /* ring, hold cron event */
							ad_timer = idle_timer = 0L;
							do_idle(0);
                            c = FALSE;
                            break;

                        default:    /* something else */
                            c = FALSE;
                            break;
                    }

                    if (c) {    /* baud rate detected */
                        if (!onConsole) {
                            detectflag = FALSE;
                            carrdetect();
                            c = 0;
							break;
                        } else {	/* processing carrier detect */
                            detectflag = TRUE;
                            c = 0;
                            /* update25();	*/
							do_idle(0);
                        }
                    }
                } else {
            /* failed to adjust */
                    Initport();
                }
            } else {
                c = getMod();
            }

			/* we had input, if we have carrier ... */
            if (haveCarrier) {
				/* if we are on the modem, check idle sceen and leave loop */
                if (whichIO == MODEM) {
					/* if we were idle, reset the idle screen */
					if (idle_timer)
						do_idle(0);
                    break;
				}
            } else {			/* dump modem input - no carrier yet */
                c = 0;
            }
        }

		/* process cron, sleep, idle events		*/
		if (!BBSCharReady()) {	/* if we are still idle */
			if (idle_timer) {	/* if we have been idle for a time */
				/* get the time now			*/
				current = cit_timer();
				//time(&current);
				/* check for cron events	*/
				if (!loggedIn && !gotCarrier() && dowhat == MAINMENU &&
					((int) (current - idle_timer) / (int) 60) >= cfg.idle) {
					idle_timer = 0L;
					if (do_cron(CRON_TIMEOUT)) {
						c = 0;
						break;
					}
				/* else check for ads		*/
				} else if (cfg.ad_time &&
					((int) (current - ad_timer) >= cfg.ad_time)
					&& loggedIn && dowhat == MAINMENU) {
					//time(&ad_timer);
					ad_timer = cit_timer();
					doAd(1);
					givePrompt();
				}
				if (chatkey || sysopkey || eventkey) {
					do_idle(0);
					c = 0;
					break;
				}
			} else {			/* we just became idle, start timers */
				//time(&idle_timer);
				/* initialize cit_time() and cit_timer()	*/
				cit_init_timer();			
				idle_timer=cit_timer();
				ad_timer = idle_timer;
			}
		}

        if (chatkey && dowhat == PROMPT) {	/* chat key pressed */
            char oldEcho;

            oldEcho = echo;
            echo = BOTH;

            doCR();
            chat();
            doCR();
			mtPrintf(TERM_REVERSE, gprompt);
			mPrintf(" ");

            echo = oldEcho;

			ad_timer = idle_timer = 0L;

            chatkey = FALSE;
        }
		if (idle_timer) {
			if (systimeout(idle_timer)) {
				nextblurb("sleep", &(cfg.cnt.sleepcount), 1);
	
				if (parm.door)  /* supress initial hangup in door mode */
					ExitToMsdos = 1;
				else
					Initport();
					
			} else if (systimeout(idle_timer - 30L)) {
				nextblurb("sleepy", &(cfg.cnt.sleepycount), 1);
			}
			/* do the idle display */
			do_idle(1);
		}
    }
	

	if (c) {
		if (c < 128)
			c = filt_in[c];
		else if (c == 255)
			c = 0;
		/* don't print out the P at the main menu. & Don't print ^A's  */
		if (c != 1 && ((c != 'p' && c != 'P') || dowhat != MAINMENU))
			echocharacter(c);
	}

    return (c);
}

/* -------------------------------------------------------------------- */
/*  setio()         set io flags according to whicio, echo and outflag  */
/* -------------------------------------------------------------------- */
void setio(char whichio, char echo, char outflag)
{
    if  ((outflag != OUTOK) && (outFlag != IMPERVIOUS)) {
        modem = FALSE;
        console = FALSE;
    } else if (echo == BOTH) {
        modem = TRUE;
        console = TRUE;
    } else if (echo == CALLER) {
        if (whichio == MODEM) {
            modem = TRUE;
            console = FALSE;
        } else if (whichio == CONSOLE) {
            modem = FALSE;
            console = TRUE;
        }
    } else if (echo == NEITHER) {
        modem = TRUE;       /* FALSE; */
        console = TRUE;     /* FALSE; */
    }
    if (!haveCarrier)
        modem = FALSE;
}

/* -------------------------------------------------------------------- */
/*  systimeout()    returns 1 if time has been exceeded                 */
/* -------------------------------------------------------------------- */
static int NEAR systimeout(long timer)
{
    long currentime, min_used;

    if ((whichIO == MODEM) && haveCarrier) {
        //time(&currentime);
        currentime = cit_timer();
        min_used = (currentime - timer) / 60L;
        return ((loggedIn && (min_used >= cfg.timeout))
        || (!loggedIn && (min_used >= cfg.unlogtimeout)));
    } else {
        return 0;
    }
}

/* EOF */
