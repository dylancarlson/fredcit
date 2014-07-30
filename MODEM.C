/* -------------------------------------------------------------------- */
/*  MODEM.C                   Citadel                                   */
/* -------------------------------------------------------------------- */
/*  High level modem code, should not need to be changed for porting(?) */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Includes                                                            */
/* -------------------------------------------------------------------- */
#include <string.h>
#include <time.h>
#include "ctdl.h"

#ifndef ATARI_ST
#include <conio.h>
#include <alloc.h>
#include <dos.h>
#include "keydefs.h"
#endif

#include "proto.h"
#include "global.h"

/* --------------------------------------------------------------------
 *                              Contents
 * --------------------------------------------------------------------
 *  carrier()       checks carrier
 *  carrdetect()    sets global flags for carrier detect
 *  carrloss()      sets global flags for carrier loss
 *  checkCR()       Checks for CRs from the data port for half a second.
 *  doccr()         Do CR on console, used to not scroll the window
 *  domcr()         print cr on modem, nulls and lf's if needed
 *  findbaud()      Finds the baud from sysop and user supplied data.
 *  fkey()          Deals with function keys from console
 *  KBReady()       returns TRUE if a console char is ready
 *  offhook()       sysop fn: to take modem off hook
 *  outCon()        put a character out to the console
 *  outstring()     push a string directly to the modem
 *  ringdetectbaud()    sets baud rate according to ring detect
 *  verbosebaud()   sets baud rate according to verbodse codes
 *  getModStr()     get a string from the modem, waiting for upto 3 secs
 * -------------------------------------------------------------------- */


/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/MODEM.C_V  $
 * 
 *    Rev 1.47   01 Nov 1991 11:20:30   FJM
 * Added gl_ structures
 *
 *    Rev 1.46   08 Jul 1991 16:19:26   FJM
 *
 *    Rev 1.45   06 Jun 1991  9:19:28   FJM
 *
 *    Rev 1.44   27 May 1991 11:43:08   FJM
 *
 *    Rev 1.41   17 Apr 1991 12:55:40   FJM
 *    Rev 1.24   11 Jan 1991 12:43:28   FJM
 *    Rev 1.20   06 Jan 1991 12:45:26   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.19   27 Dec 1990 20:16:54   FJM
 * Changes for porting.
 *
 *    Rev 1.16   22 Dec 1990 13:39:32   FJM
 *    Rev 1.9   07 Dec 1990 23:10:30   FJM
 *    Rev 1.7   24 Nov 1990  3:44:54   FJM
 * Modified to eat characters after a function key.
 *
 *    Rev 1.6   24 Nov 1990  3:07:40   FJM
 * Changes for shell/door mode.
 *
 *    Rev 1.5   23 Nov 1990 13:25:10   FJM
 * Header change
 *
 *    Rev 1.3   17 Nov 1990 22:43:12   FJM
 * Made F2 drop to DOS in Shell mode.
 *
 *    Rev 1.2   17 Nov 1990 16:11:56   FJM
 * Added version control log header
 * -------------------------------------------------------------------- */

/* --------------------------------------------------------------------
 *  HISTORY:
 *
 *  05/26/89    (PAT)   Code moved to Input.c, output.c, and timedate.c
 *  02/07/89    (PAT)   Hardeware dependant code moved to port.c,
 *                      History recreated. PLEASE KEEP UP-TO-DATE
 *  05/11/82    (CrT)   Created
 *
 *  03/12/90    FJM             Added IBM Grahics character translation
 *  03/19/90    FJM     Linted & partial cleanup
 *  04/02/90    FJM     Added ALT-M memory/stach check
 *
 * -------------------------------------------------------------------- */

/* extern unsigned _stklen; */

/* --------------------------------------------------------------------
 *  carrier()       checks carrier
 * -------------------------------------------------------------------- */

int carrier(void)
{
    unsigned char c;

    if (            /* (whichIO==MODEM) && */
        (c = (uchar) gotCarrier()) != modStat
        && (!detectflag)
    ) {
    /* carrier changed   */
        if (c) {        /* carrier present   */
            switch (cfg.dumbmodem) {
                case 0:     /* numeric */
        /* do not use this routine for carrier detect */
                    return (1);

                case 1:     /* returns */
                    if (!findbaud()) {
                        Initport();
                        return TRUE;
                    }
                    break;

                case 2:     /* HS on RI */
                    ringdetectbaud();
                    break;

                case 3:     /* verbose */
                    verbosebaud();
                    break;

                default:
                case 4:     /* forced */
                    baud(cfg.initbaud);
                    break;
            }

            if (!onConsole) {
                carrdetect();
                detectflag = FALSE;
                return (0);
            } else {
                detectflag = TRUE;
                /* update25();	*/
				do_idle(0);
                return (1);
            }
        } else {
            delay(2000);    /* confirm it's not a glitch */
            if (!gotCarrier()) {/* check again */
                carrloss();

                return (0);
            }
        }
    }
    return (1);
}

/* -------------------------------------------------------------------- */
/*  carrdetect()    sets global flags for carrier detect                */
/* -------------------------------------------------------------------- */
void carrdetect(void)
{
    char temp[30];

    haveCarrier = TRUE;
    modStat = TRUE;
    newCarrier = TRUE;
    justLostCarrier = FALSE;

    conntimestamp = cit_time();

    connectcls();
    /* update25();	*/
	do_idle(0);

    sprintf(temp, "Carrier-Detect (%d)", bauds[speed]);
    trap(temp, T_CARRIER);

    logBuf.credits = cfg.unlogbal;
}

/* -------------------------------------------------------------------- */
/*  carrloss()      sets global flags for carrier loss                  */
/* -------------------------------------------------------------------- */
void carrloss(void)
{
    outFlag = OUTSKIP;
    haveCarrier = FALSE;
    modStat = FALSE;
    justLostCarrier = TRUE;
    if (parm.door)
        ExitToMsdos = TRUE;
    Initport();

    trap("Carrier-Loss", T_CARRIER);
}

/* -------------------------------------------------------------------- */
/*  checkCR()       Checks for CRs from the data port for half a second.*/
/* -------------------------------------------------------------------- */
BOOL checkCR(void)
{
    int i;

    for (i = 0; i < 50; i++) {
        delay(10);
        if (MIReady())
            if (getMod() == '\r')
                return FALSE;
    }
    return TRUE;
}

/* -------------------------------------------------------------------- */
/*  doccr()         Do CR on console, used to not scroll the window     */
/* -------------------------------------------------------------------- */
void doccr(void)
{
    unsigned char row, col;


    if (!console)
        return;
    if (!anyEcho)
        return;

    readpos(&row, &col);

    if (row == (scrollpos + 1)) {
        position(0, 0);     /* clear screen if we hit our window */
    }
    if (row >= scrollpos) {
        scroll(scrollpos, 1, cfg.attr);
        position(scrollpos, 0);
    } else {
        putch('\n');
        putch('\r');
    }
}

/* -------------------------------------------------------------------- */
/*  domcr()         print cr on modem, nulls and lf's if needed         */
/* -------------------------------------------------------------------- */
void domcr(void)
{
    int i;

    outMod('\r');
    for (i = gl_term.termNulls; i; i--)
        outMod(0);
    if (gl_term.termLF)
        outMod('\n');
}

/* -------------------------------------------------------------------- */
/*  findbaud()      Finds the baud from sysop and user supplied data.   */
/* -------------------------------------------------------------------- */
int findbaud(void)
{
    char noGood = TRUE;
    int Time = 0;
    int baudRunner;     /* Only try for 60 seconds      */

    while (MIReady())
        getMod();       /* Clear garbage        */
    baudRunner = 0;
    while (gotCarrier() && noGood && Time < 120) {
        Time++;
        baud(baudRunner);
        noGood = checkCR();
        if (noGood)
            baudRunner = (baudRunner + 1) % (3 /* 2400 too */ );
    }
    return !noGood;
}

/* -------------------------------------------------------------------- */
/*  fkey()          Deals with function keys from console               */
/* -------------------------------------------------------------------- */
void fkey(void)
{
    char key;
    int oldIO, i;
    label string;
    unsigned space;

    key = (char) getch();
    while (kbhit())     /* flush keyboard buffer */
        getch();
    if (strcmpi(cfg.f6pass, "f6disabled") != SAMESTRING)
        if (ConLock == TRUE && key == K_A_L &&
        strcmpi(cfg.f6pass, "disabled") != SAMESTRING) {
            ConLock = FALSE;

            oldIO = whichIO;
            whichIO = CONSOLE;
            onConsole = TRUE;
            /* update25();	*/
			do_idle(0);
            string[0] = 0;
            getNormStr("System Password", string, NAMESIZE, NO_ECHO);
            if (strcmpi(string, cfg.f6pass) != SAMESTRING)
                ConLock = TRUE;
            whichIO = (BOOL) oldIO;
            onConsole = (BOOL) (whichIO == CONSOLE);
            /* update25();	*/
			do_idle(0);
            givePrompt();
            return;
        }
    if (ConLock && !gl_user.sysop && strcmpi(cfg.f6pass, "f6disabled") != SAMESTRING)
        return;

    switch (key) {
        case K_F1:
            drop_dtr();
            detectflag = FALSE;
            break;

        case K_F2:
            if (parm.door)
                ExitToMsdos = 1;
            else
                Initport();
            detectflag = FALSE;
            break;

        case K_F3:
            sysReq = (BOOL) (!sysReq);
            break;

        case K_F4:
            anyEcho = (BOOL) (!anyEcho);
            break;

        case K_F5:
            if (whichIO == CONSOLE)
                whichIO = MODEM;
            else
                whichIO = CONSOLE;

            onConsole = (BOOL) (whichIO == CONSOLE);
            break;

        case K_S_F6:
            if (!ConLock)
                gl_user.aide = (BOOL) (!gl_user.aide);
            break;

        case K_A_F6:
            if (!ConLock)
                gl_user.sysop = (BOOL) (!gl_user.sysop);
            break;

        case K_F6:
            if (gl_user.sysop || !ConLock)
                sysopkey = TRUE;
            break;

        case K_F7:
            cfg.noBells = !cfg.noBells;
            break;

        case K_A_C:
        case K_F8:
            chatkey = !chatkey;    /* will go into chat from main() */
            break;

        case K_F9:
            cfg.noChat = !cfg.noChat;
            chatReq = FALSE;
            break;

        case K_F10:
            help();
            break;

        case K_A_B:
            backout = !backout;
            break;

        case K_A_D:
            debug = !debug;
            break;

        case K_A_E:
            eventkey = TRUE;
            break;

        case K_A_L:
            if (cfg.f6pass[0] && strcmpi(cfg.f6pass, "f6disabled") != SAMESTRING)
                ConLock = (BOOL) (!ConLock);
            break;

        case K_A_M:
#ifndef ATARI_ST
            cPrintf("Free heap    %lXh (%lu.)\n", farcoreleft(), farcoreleft());
            space = _stklen - ((unsigned) 0xffff - _SP);
            cPrintf("Free stack   %Xh (%u.)\n", space, space);
#endif
            break;

        case K_A_P:
            if (printing) {
                printing = FALSE;
                fclose(printfile);
            } else {
                printfile = fopen(cfg.printer, "a");
                if (printfile) {
                    printing = TRUE;
                } else {
                    printing = FALSE;
                }
            }
            break;

        case K_A_T:
            gl_user.twit = (BOOL) (!gl_user.twit);
            break;

        case K_A_X:
            if (dowhat == MAINMENU || dowhat == SYSOPMENU) {
                if (loggedIn) {
                    i = getYesNo("Exit to MS-DOS", 0);
                } else {
                    doCR();
                    doCR();
                    mPrintf("Exit to MS-DOS");
                    doCR();
                    i = TRUE;
                }

                if (!i) {
                    if (dowhat == MAINMENU) {
                        givePrompt();
                    } else {
                        doCR();
                        mPrintf("Privileged function: ");
                    }
                    break;
                }
                ExitToMsdos = TRUE;
            }
            break;

		case K_PgDn:
			logBuf.credits -= 10;
            break;
			
		case K_PgUp:
			logBuf.credits += 10;
            break;
			
		case K_Lft:
            ungetch('-');
			getkey = 1;
            break;
			
		case K_Rgt:
            ungetch('=');
			getkey = 1;
            break;
			
		case K_Up:
            ungetch(']');
			getkey = 1;
            break;
			
		case K_Dwn:
            ungetch('[');
			getkey = 1;
            break;
			
        default:
            break;
    }

    /* update25();	*/
	do_idle(0);
}

/* -------------------------------------------------------------------- */
/*  KBReady()       returns TRUE if a console char is ready             */
/* -------------------------------------------------------------------- */
BOOL KBReady(void)
{
    int c;

    if (getkey)
        return (TRUE);

    if (kbhit()) {
        c = getch();

        if (!c) {
            fkey();
            return (FALSE);
        } else
            ungetch(c);

        getkey = 1;

        return (TRUE);
    } else
        return (FALSE);
}

/* -------------------------------------------------------------------- */
/*  offhook()       sysop fn: to take modem off hook                    */
/* -------------------------------------------------------------------- */
void offhook(void)
{
    Initport();
    outstring("ATM0H1\r");
}

/* -------------------------------------------------------------------- */
/*  outCon()        put a character out to the console                  */
/* -------------------------------------------------------------------- */
void outCon(char c)
{
    unsigned char row, col;
    static char escape = FALSE;

    if (!console)
        return;

    if (c == '\a' /* BELL */ && cfg.noBells)
        return;
    if (c == 27 || escape) {    /* ESC || ANSI sequence */
        escape = ansi(c);
        return;
    }
    if (c == 0x1a)      /* CT-Z */
        return;

    if (!anyEcho)
        return;

    /* if we dont have carrier then count what goes to console */
    if (!gotCarrier())
        transmitted++;

    if (c == '\n')
        doccr();
    else if (c == '\r') {
        putch(c);
    } else {
        readpos(&row, &col);
        if (c == '\b' || c == 7) {
            if (c == '\b' && col == 0 && prevChar != '\n')
                position(row - 1, 80);
            putch(c);
        } else {
            (*charattr) (c, ansiattr);
            if (col == 79) {
                position(row, col);
                doccr();
            }
        }
    }
}

/* -------------------------------------------------------------------- */
/*  outstring()     push a string directly to the modem                 */
/* -------------------------------------------------------------------- */
void outstring(char *string)
{
    int mtmp;

    mtmp = modem;
    modem = TRUE;

    while (*string) {
        outMod(*string++);  /* output string */
    }

    modem = (uchar) mtmp;
}

/* -------------------------------------------------------------------- */
/*  ringdetectbaud()    sets baud rate according to ring detect         */
/* -------------------------------------------------------------------- */
void ringdetectbaud(void)
{
    baud(ringdetect());
}

/* -------------------------------------------------------------------- */
/*  verbosebaud()   sets baud rate according to verbodse codes          */
/* -------------------------------------------------------------------- */
void verbosebaud(void)
{
    char c, f = 0;
    long t;

    if (debug)
        outCon('[');

    t = cit_timer();

    while (gotCarrier() && cit_timer() < (t + 6) && !KBReady()) {
        if (MIReady()) {
            c = (char) getMod();
        } else {
            c = 0;
        }

        if (debug && c) {
            outCon(c);
        }
        if (f) {
            switch (c) {
                case '\n':
                case '\r':      /* CONNECT */
                    baud(0);
                    if (debug)
                        outCon(']');
                    return;
                case '1':       /* CONNECT 1200 */
                    baud(1);
                    if (debug)
                        outCon(']');
                    return;
                case '2':       /* CONNECT 2400 */
                    baud(2);
                    if (debug)
                        outCon(']');
                    return;
                case '4':       /* CONNECT 4800 */
                    baud(3);
                    if (debug)
                        outCon(']');
                    return;
                case '9':       /* CONNECT 9600 */
                    baud(4);
                    if (debug)
                        outCon(']');
                    return;
                default:
                    break;
            }
        }
        if (c == 'C') {
            if (debug) {
                outCon('@');
            }
            f = 1;
        }
    }
}

/* -------------------------------------------------------------------- */
/*  getModStr()     get a string from the modem, waiting for upto 3 secs*/
/*                  for it. Returns TRUE if it gets one.                */
/* -------------------------------------------------------------------- */
int getModStr(char *str)
{
    long tm;
    int l = 0, c;

    tm = cit_timer();

    if (debug)
        cPrintf("[");

    while (
        (cit_timer() - tm) < 4
        && !KBReady()
        && l < 40
    ) {
        if (MIReady()) {
            c = getMod();

            if (c == 13 || c == 10) {   /* CR || LF */
                str[l] = EOS;
                if (debug)
                    cPrintf("]\n");
                return TRUE;
            } else {
                if (debug)
                    cPrintf("%c", c);
                str[l] = (char) c;
                l++;
            }
        }
    }

    if (debug)
        cPrintf(":F]\n");

    str[0] = EOS;

    return FALSE;
}
