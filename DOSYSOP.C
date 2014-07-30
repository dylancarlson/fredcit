/************************************************************************
 * $Header:   D:/VCS/FCIT/DOSYSOP.C_V   1.35   01 Nov 1991 11:19:56   FJM  $
 *              Command-interpreter code for Citadel
 ************************************************************************/

#include <string.h>
#include <time.h>
#include "ctdl.h"

#ifndef ATARI_ST
#include <dos.h>
//#include <conio.h>
#endif

#include "keywords.h"
#include "proto.h"
#include "global.h"

/************************************************************************
 *                              Contents
 *
 *     doSysop()        handles sysop-only      commands
 *     do_SysopGroup()
 *     do_SysopHall()
 *     do_SysopEnter() 	Handle .SE functions
 *
 ************************************************************************/


/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/DOSYSOP.C_V  $
 * 
 *    Rev 1.35   01 Nov 1991 11:19:56   FJM
 * Added gl_ structures
 *
 *    Rev 1.34   21 Sep 1991 10:18:42   FJM
 * FredCit release
 *
 *    Rev 1.33   08 Jul 1991 16:18:26   FJM
 * Fix for Priv funct. prompt on aborted .sx
 *
 *    Rev 1.32   11 Jun 1991  8:17:10   FJM
 *
 *    Rev 1.31   06 Jun 1991  9:18:46   FJM
 *    Rev 1.30   27 May 1991 11:42:26   FJM
 *    Rev 1.29   22 May 1991  2:15:30   FJM
 *    Rev 1.26   17 Apr 1991 12:55:12   FJM
 *    Rev 1.17   28 Jan 1991 13:14:24   FJM
 * Improved some of the submenus.
 *
 *    Rev 1.12   18 Jan 1991 16:53:56   FJM
 *    Rev 1.8   11 Jan 1991 12:44:30   FJM
 *    Rev 1.4   06 Jan 1991 12:45:16   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.3   27 Dec 1990 20:17:12   FJM
 * Changes for porting.
 *
 *    Rev 1.0   22 Dec 1990 13:40:52   FJM
 * Initial revision.
 *
 * -------------------------------------------------------------------- */

/************************************************************************
 *              External function definitions
 ************************************************************************/

static void do_SysopGroup(void);
static void do_SysopHall(void);
static void do_SysopEnter(void);

/************************************************************************/
/*      doSysop() handles the sysop-only menu                           */
/*          return FALSE to fall invisibly into default error msg       */
/************************************************************************/
char doSysop(void)
{
    char oldIO;
    int c;

    oldIO = whichIO;

    /* we want to be in console mode when we go into sysop menu */
    if (!gotCarrier() || !gl_user.sysop) {
        whichIO = CONSOLE;
        onConsole = (char) (whichIO == CONSOLE);
    }
    gl_user.sysop = TRUE;

    /* update25();	*/
	do_idle(0);

    while (!ExitToMsdos && (onConsole || gotCarrier())) {
        amZap();

        outFlag = IMPERVIOUS;
        doCR();
        mtPrintf(TERM_REVERSE, "Privileged function:");
        mPrintf(" ");

        dowhat = SYSOPMENU;
        c = iChar();
        dowhat = DUNO;

        switch (toupper(c)) {
            case 'A':
                mPrintf("\bAbort\n ");
        /* restore old mode */
                whichIO = oldIO;
                gl_user.sysop = (char) (loggedIn ? logBuf.lbflags.SYSOP : 0);
                onConsole = (char) (whichIO == CONSOLE);
                /* update25();	*/
				do_idle(0);
                return FALSE;
            case 'C':
                mPrintf("\bCron special: ");
                cron_commands();
                break;
            case 'D':
                mPrintf("\bDate change\n ");
                changeDate();
                break;
            case 'E':
				mPrintf("\bEnter file ");
				do_SysopEnter();
                break;
            case 'F':
                doAide(1, 'E');
                break;
            case 'G':
                mPrintf("\bGroup special: ");
                do_SysopGroup();
                break;
            case 'H':
                mPrintf("\bHallway special: ");
                do_SysopHall();
                break;
            case 'K':
                mPrintf("\bKill account\n ");
                killuser();
                break;
            case 'L':
                mPrintf("\bLogin enabled\n ");
                sysopNew = TRUE;
                break;
            case 'M':
                mPrintf("\bMass delete\n ");
                massdelete();
                break;
            case 'N':
                mPrintf("\bNew user Verification\n ");
                globalverify();
                break;
            case 'O':
                mPrintf("\bOff hook\n ");
                if (!onConsole)
                    break;
                offhook();
                break;
            case 'R':
                mPrintf("\bReset file info\n ");
                if (roomBuf.rbflags.MSDOSDIR != 1) {
                    if (gl_user.expert)
                        mPrintf("? ");
                    else
                        mPrintf("\n Not a directory room.");
                } else
                    updateinfo();
                break;
            case 'S':
                mPrintf("\bShow user\n ");
                showuser();
                break;
            case 'U':
                mPrintf("\bUserlog edit\n ");
                userEdit();
                break;
            case 'V':
                mPrintf("\bView Help Text File\n ");
                nextblurb("sysop", &(cfg.cnt.shelpcount), 1);
                break;
            case 'X':
                mPrintf("\bExit to MS-DOS\n ");
                if (!onConsole) {
					mPrintf(" Remote sysops can not exit to DOS\n");
					doCR();
                    break;
				}
                if (!getYesNo(confirm, 0)) {
					whichIO = oldIO;
					gl_user.sysop = (char) (loggedIn ? logBuf.lbflags.SYSOP : 0);
					onConsole = (char) (whichIO == CONSOLE);
					/* update25();	*/
					do_idle(0);
					return FALSE;
				}
                ExitToMsdos = TRUE;
                return FALSE;
            case 'Z':
                mPrintf("\bZap empty rooms\n ");
                killempties();
                break;
            case '!':
                mPrintf("\b ");
                doCR();
                if (!onConsole)
                    break;
                shellescape(0);
                break;
            case '@':
                mPrintf("\b ");
                doCR();
                if (!onConsole)
                    break;
                shellescape(1);
                break;
            case '#':
                mPrintf("\bRead by message number\n ");
                readbymsgno();
                break;
            case '*':
                mPrintf("\bUnlink file(s)\n ");
                if (roomBuf.rbflags.MSDOSDIR != 1) {
                    if (gl_user.expert)
                        mPrintf("? ");
                    else
                        mPrintf("\n Not a directory room.");
                } else
                    sysopunlink();
                break;
            case '?':
                nextmenu("sysop", &(cfg.cnt.sysoptut), 1);
                break;
            default:
                if (!gl_user.expert)
                    mPrintf("\n '?' for menu.\n ");
                else
                    mPrintf(" ?\n ");
                break;
        }
    }
    return FALSE;
}

/************************************************************************/
/*     do_SysopGroup() handles doSysop() Group functions                */
/************************************************************************/
static void do_SysopGroup()
{
    switch (toupper(iChar())) {
        case 'G':
            mPrintf("\bGlobal Group membership\n  \n");
            globalgroup();
            break;
        case 'K':
            mPrintf("\bKill group");
            killgroup();
            break;
        case 'N':
            mPrintf("\bNew group");
            newgroup();
            break;
        case 'U':
            mPrintf("\bGlobal user membership\n  \n");
            globaluser();
            break;
        case 'R':
            mPrintf("\bRename group");
            renamegroup();
            break;
        case '?':
            doCR();
            mpPrintf(" <K>ill group\n");
            mpPrintf(" <N>ew group\n");
            mpPrintf(" <G>lobal membership\n");
            mpPrintf(" <U>ser global membership\n");
            doCR();
            mpPrintf(" <R>ename group");
            break;
        default:
            if (!gl_user.expert)
                mPrintf("\n '?' for menu.\n ");
            else
                mPrintf(" ?\n ");
            break;
    }
}

/************************************************************************/
/*     do_SysopHall() handles the doSysop hall functions                */
/************************************************************************/
static void do_SysopHall()
{

    switch (toupper(iChar())) {
        case 'F':
            mPrintf("\bForce access");
            force();
            break;
        case 'K':
            mPrintf("\bKill hallway");
            killhall();
            break;
        case 'L':
            mPrintf("\bList halls");
            listhalls();
            break;
        case 'N':
            mPrintf("\bNew hall");
            newhall();
            break;
        case 'R':
            mPrintf("\bRename hall");
            renamehall();
            break;
        case 'G':
            mPrintf("\bGlobal Hall func");
            doCR();
            globalhall();
            break;
        case '?':
            doCR();
            mpPrintf(" <F>orce\n");
            mpPrintf(" <G>lobal hall func\n");
            mpPrintf(" <K>ill\n");
            mpPrintf(" <L>ist\n");
            mpPrintf(" <N>ew\n");
            mpPrintf(" <R>ename ");
            break;
        default:
            if (!gl_user.expert)
                mPrintf("\n '?' for menu.\n ");
            else
                mPrintf(" ?\n ");
            break;
    }
}

/************************************************************************
 *     do_SysopEnter() 	Handle .SE functions
 ************************************************************************/
static void do_SysopEnter(void)
{
	switch (toupper(iChar())) {
		case 'C':
			mPrintf("\bconfig.cit\n");
			readconfig(0);
			break;
		case 'E':
			mPrintf("\bexternal.cit\n");
            readprotocols();
			break;
		case 'G':
			mPrintf("\bgrpdata.cit\n");
            readaccount();
			break;
        case '?':
            doCR();
            mpPrintf(" <C>onfiguration file\n");
            mpPrintf(" <E>xternal file\n");
            mpPrintf(" <G>roup file\n");
            break;
        default:
            if (!gl_user.expert)
                mPrintf("\n '?' for menu.\n ");
            else
                mPrintf(" ?\n ");
            break;
	}
}
