/************************************************************************
 *                              command.c
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
 *      mAbort()        		returns TRUE if the user has aborted typeout
 *      exclude()
 *      doHelp()                handles H(elp)          command
 *      doIntro()               handles I(ntro)         command
 *      doKnown()               handles K(nown rooms)   command
 *      doLogin()               handles L(ogin)         command
 *      doLogout()              handles T(erminate)     command
 *      doXpert()
 *      doRegular()             fanout for above commands
 *      doNext()                handles '+' next room
 *      doPrevious()            handles '-' previous room
 *      doNextHall()            handles '>' next room
 *      doPreviousHall()        handles '<' previous room
 *      doAd(void)                              show and ad n% of the time.
 *      getCommand()            prints prompt and gets command char
 *      greeting()              System-entry blurb etc
 *      updatebalance()         updates user's accounting balance
 *
 ************************************************************************/


/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/COMMAND.C_V  $
 *
 *    Rev 1.52   01 Nov 1991 11:19:44   FJM
 * Added gl_ structures
 *
 *    Rev 1.51   21 Sep 1991 10:18:24   FJM
 * FredCit release
 *
 *    Rev 1.50   15 Jun 1991  8:36:52   FJM
 *
 *    Rev 1.48   14 Jun 1991  7:42:12   FJM
 * Enhanced list_externs().
 *
 *    Rev 1.47   06 Jun 1991  9:18:26   FJM
 *
 *    Rev 1.46   27 May 1991 11:40:50   FJM
 * Added a confirm to the C (Chat) command.
 *
 *    Rev 1.43   17 Apr 1991 12:54:54   FJM
 *    Rev 1.42   10 Apr 1991  8:43:50   FJM
 * Inhibit .RB in anon rooms.
 *
 *    Rev 1.37   10 Feb 1991 17:32:32   FJM
 *    Rev 1.34   28 Jan 1991 13:09:54   FJM
 *    Rev 1.32   19 Jan 1991 14:14:32   FJM
 * Added time display.
 *
 *    Rev 1.29   18 Jan 1991 16:49:02   FJM
 * Message for screen pause.
 *
 *    Rev 1.27   13 Jan 1991  0:30:18   FJM
 * Name overflow fixes
 *
 *    Rev 1.24   06 Jan 1991 16:49:32   FJM
 *    Rev 1.21   06 Jan 1991 12:45:12   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.20   27 Dec 1990 20:16:56   FJM
 * Changes for porting.
 *
 *    Rev 1.16   22 Dec 1990 13:39:34   FJM
 *    Rev 1.10   16 Dec 1990 18:12:40   FJM
 *    Rev 1.9   09 Dec 1990 15:21:46   FJM
 * Pretty prompts.
 *
 *    Rev 1.8   08 Dec 1990 12:23:20   FJM
 * Changed #EXTERNAL commands so expanded commands don't run shell program.
 *
 *    Rev 1.7   08 Dec 1990 12:07:14   FJM
 * Made mAbort set normal ANSI attribute when message read aborted.
 *
 *    Rev 1.5   24 Nov 1990  3:07:18   FJM
 * Changes for shell/door mode.
 *
 *    Rev 1.4   23 Nov 1990 13:22:12   FJM
 * Cleanup.
 *
 *    Rev 1.2   17 Nov 1990 16:12:20   FJM
 * Added version control log header
 *
 * --------------------------------------------------------------------
 *
 *  Early History:
 *
 *  06/30/90    FJM     New module.
 *  07/26/90    FJM     Moved mAbort here to save on overlay thrashing
 *  08/07/90    FJM     Improved ad_chance method.
 *  10/15/90    FJM     Added console only protocol flag (field #1)
 *  11/04/90    FJM     Added '\b' alias for '-'
 *
 * -------------------------------------------------------------------- */

/************************************************************************
 *              External function definitions
 ************************************************************************/

static int doPause(void);
static void listExterns(void);

/* --------------------------------------------------------------------
 * doPause - wait for user to cancel pause.
 * -------------------------------------------------------------------- */

static int doPause(void)
{
    char c, rc;
	char *msg;
	int i;
	int showmsg;

	showmsg = (outFlag == OUTPAUSE);
    outFlag = OUTOK;
	/* show the pause message */
	if (!gl_user.expert && showmsg) {
		if (gl_user.aide)
			msg = " <[Enter] J/K/M/N/R/C/~/!/S> ";
		else if ((cfg.kill &&
				(strcmpi(logBuf.lbname, msgBuf->mbauth) == SAMESTRING)
				&& loggedIn))
			msg = " <[Enter], J, K, N, R, or S> ";
		else
			msg = " <[Enter], J, N, R, or S> ";
		mPrintf(msg);
	}
	c = (char) toupper(iChar());    /* wait to resume */
	switch (c) {
		case 'J':       /* jump paragraph: */
			outFlag = OUTPARAGRAPH;
			rc = FALSE;
			break;
		case 'K':       /* kill:          */
			if (gl_user.aide ||
				(cfg.kill &&
				(strcmpi(logBuf.lbname, msgBuf->mbauth) == SAMESTRING)
				&& loggedIn))
				dotoMessage = PULL_IT;
			rc = FALSE;
			break;
		case 'M':       /* mark:          */
			if (gl_user.aide)
				dotoMessage = MARK_IT;
			rc = FALSE;
			break;
		case 'N':       /* next:          */
			outFlag = OUTNEXT;
			rc = TRUE;
			break;
		case 'S':       /* skip:          */
			outFlag = OUTSKIP;
			rc = TRUE;
			break;
		case 'R':
			dotoMessage = REVERSE_READ;
			rc = FALSE;
			break;
		case 'C':
			dotoMessage = COPY_IT;
			rc = FALSE;
			break;
		case '~':
			termCap(TERM_NORMAL);
			gl_term.ansiOn = !gl_term.ansiOn;
			break;
		case '!':
			gl_term.IBMOn = !gl_term.IBMOn;
			break;
		default:
			rc = FALSE;
			break;
	}
	if (!gl_user.expert && showmsg)
		for (i=strlen(msg); i; --i)
			doBS();
	return rc;
}
/* -------------------------------------------------------------------- */
/*  mAbort()        returns TRUE if the user has aborted typeout        */
/* -------------------------------------------------------------------- */
BOOL mAbort(void)
{
    char c, rc, oldEcho, tmpOutFlag;

    /* Check for abort/pause from user */
    if (outFlag == IMPERVIOUS)
        return FALSE;

    if (!BBSCharReady() && outFlag != OUTPAUSE) {
    /* Let modIn() report the problem */
        if (haveCarrier && !gotCarrier())
            iChar();
        rc = FALSE;
    } else {
        oldEcho = echo;
        echo = NEITHER;
        echoChar = 0;

        if (outFlag == OUTPAUSE) {
            c = 'P';
        } else {
            c = (char) toupper(iChar());
        }

        switch (c) {
            case 19:        /* XOFF */
            case 'P':       /* pause:         */
				rc = doPause();
                break;
            case 'C':
                dotoMessage = COPY_IT;
                rc = FALSE;
                break;
            case 'J':       /* jump paragraph: */
                outFlag = OUTPARAGRAPH;
                rc = FALSE;
                break;
            case 'K':       /* kill:          */
                if (gl_user.aide ||
                    (cfg.kill && (strcmpi(logBuf.lbname, msgBuf->mbauth) == SAMESTRING)
                    && loggedIn))
                    dotoMessage = PULL_IT;
                rc = FALSE;
                break;
            case 'M':       /* mark:          */
                if (gl_user.aide)
                    dotoMessage = MARK_IT;
                rc = FALSE;
                break;
            case 'N':       /* next:          */
                outFlag = OUTNEXT;
                rc = TRUE;
                break;
            case 'S':       /* skip:          */
                outFlag = OUTSKIP;
                rc = TRUE;
                break;
            case 'R':
                dotoMessage = REVERSE_READ;
                rc = FALSE;
                break;
            case '~':
                termCap(TERM_NORMAL);
                gl_term.ansiOn = !gl_term.ansiOn;
                break;
            case '!':
                gl_term.IBMOn = !gl_term.IBMOn;
                break;
            default:
                rc = FALSE;
                break;
        }
        echo = oldEcho;
    }
    if (rc) {
		tmpOutFlag = outFlag;
		outFlag = OUTOK;
        termCap(TERM_NORMAL);
		outFlag = tmpOutFlag;
	}
    return rc;
}


/************************************************************************/
/*      exclude() handles X>clude room,  toggles the bit                */
/************************************************************************/
void exclude(void)
{
    if  (!logBuf.lbroom[thisRoom].xclude) {
        mPrintf("\n \n Room now excluded from G)oto loop.\n ");
        logBuf.lbroom[thisRoom].xclude = TRUE;
    } else {
        mPrintf("\n \n Room now in G)oto loop.\n ");
        logBuf.lbroom[thisRoom].xclude = FALSE;
    }
}

/************************************************************************/
/*      doHelp() handles H(elp) command                                 */
/************************************************************************/
void doHelp(char expand)	/* TRUE to accept following parameters  */
{
    label fileName;

    mPrintf("\bHelp File(s)");
    if (!expand) {
        mPrintf("\n\n");
        nexthelp("dohelp", &(cfg.cnt.dohelptut), 1);
        return;
    }
    getString("", fileName, 9, 1, ECHO, "");
    normalizeString(fileName);

    if (strlen(fileName) == 0)
        strcpy(fileName, "dohelp");

    if (fileName[0] == '?') {
        nexthelp("helpopt", &(cfg.cnt.helpopttut), 1);
    } else {
        nexthelp(fileName, &(cfg.cnt.fileNametut), 1);
    }
}

/************************************************************************/
/*      doIntro() handles Intro to ....  command.                       */
/************************************************************************/
void doIntro(void)
{
    mPrintf("\bIntro to %s\n ", cfg.nodeTitle);
    nexthelp("intro", &(cfg.cnt.introtut), 1);
}

/***********************************************************************
 *      doKnown() handles K(nown rooms) command
 *
 *	Parameters:
 *
 * 		char moreYet	TRUE to accept following parameters
 * 		char first		first parameter if true
 *
 ***********************************************************************/
void doKnown(char moreYet, char first)
{
    char letter;
    char verbose = FALSE;
    char numMess = FALSE;
    char done;

    if (moreYet)
        first = '\0';

    mPrintf("\bKnown ");

    if (first)
        oChar(first);

    do {
        outFlag = IMPERVIOUS;
        done = TRUE;

        letter = (char) (toupper(first ? (char) first : (char) iChar()));
        switch (letter) {
            case 'A':
                mPrintf("\bApplication Rooms ");
                mPrintf("\n ");
                listRooms(APLRMS, verbose, numMess);
				listExterns();
                break;
            case 'D':
                mPrintf("\bDirectory Rooms ");
                mPrintf("\n ");
                listRooms(DIRRMS, verbose, numMess);
                break;
            case 'H':
                mPrintf("\bHallways ");
                knownhalls();
                break;
            case 'L':
                mPrintf("\bLimited Access Rooms ");
                mPrintf("\n ");
                listRooms(LIMRMS, verbose, numMess);
                break;
            case 'N':
                mPrintf("\bNew Rooms ");
                mPrintf("\n ");
                listRooms(NEWRMS, verbose, numMess);
                break;
            case 'O':
                mPrintf("\bOld Rooms ");
                mPrintf("\n ");
                listRooms(OLDRMS, verbose, numMess);
                break;
            case 'M':
                mPrintf("\bMail Rooms ");
                mPrintf("\n ");
                listRooms(MAILRM, verbose, numMess);
                break;
            case 'S':
                mPrintf("\bShared Rooms ");
                mPrintf("\n ");
                listRooms(SHRDRM, verbose, numMess);
                break;
            case 'I':
                mPrintf("\bRoom Info");
                mPrintf("\n ");
                RoomStatus();
                break;
            case '\r':
            case '\n':
                listRooms(OLDNEW, verbose, numMess);
                break;
            case 'R':
                mPrintf("\bRooms ");
                mPrintf("\n ");
                listRooms(OLDNEW, verbose, numMess);
                break;
            case 'V':
                mPrintf("\bVerbose ");
                done = FALSE;
                verbose = TRUE;
                break;
            case 'W':
                mPrintf("\bWindows ");
                mPrintf("\n ");
                listRooms(WINDWS, verbose, numMess);
                break;
            case 'X':
                mPrintf("\beXcluded Rooms ");
                mPrintf("\n ");
                listRooms(XCLRMS, verbose, numMess);
                break;
			case 'Y':
				mPrintf("\banonYmous Rooms ");
                mPrintf("\n ");
                listRooms(ANONRM, verbose, numMess);
                break;
            case '#':
                mPrintf(" of Messages ");
                done = FALSE;
                numMess = TRUE;
                break;
            default:
                mPrintf("? ");
                if (gl_user.expert)
                    break;
            case '?':
                nextmenu("known", &(cfg.cnt.knowntut), 1);
                break;
        }
    }
    while (!done && moreYet);
}

/************************************************************************
 *      listExterns()	Display external commands
 ************************************************************************/

static void listExterns(void)
{
	int i;
	int cmnds_visable=0;
	char *p;

	doCR();
	if (onConsole && extCmd[0].name[0]) {
			cmnds_visable = 1;
	} else {
		for (i = 0; i < MAXEXTERN && extCmd[i].name[0]; ++i)
			if (!extCmd[i].local) {
				cmnds_visable = 1;
				break;
			}
	}

	outFlag = OUTOK;
	if (cmnds_visable) {
		mtPrintf(TERM_BOLD,"External Commands:");
		doCR();
		doCR();
		for (i = 0; i < MAXEXTERN && extCmd[i].name[0]; ++i) {
			if (onConsole || !extCmd[i].local) {
				p = extCmd[i].name;
				mtPrintf(TERM_BOLD,"%c",*p);
				++p;
				if (*p)
					mPrintf("%s", p);
				if (extCmd[i].local)
					mPrintf(" (local)");
				doCR();
			}
		}
	} else {
		mtPrintf(TERM_BOLD,"No external commands.");
		doCR();
	}
}

/************************************************************************/
/*      doLogin() handles L(ogin) command                               */
/************************************************************************/
void doLogin(char moreYet)	/* TRUE to accept following parameters  */
{
    char InitPw[NAMESIZE*2+2];
    char passWord[NAMESIZE*2+2];
    char Initials[NAMESIZE*2+2];
    char *semicolon;

    if (justLostCarrier || ExitToMsdos)
        return;

    if (moreYet == 2)
        moreYet = FALSE;
    else
        mPrintf("\bLogin");

    /* we want to be in console mode when we log in from local */
    if (!gotCarrier() && !loggedIn) {
        whichIO = CONSOLE;
        onConsole = (char) (whichIO == CONSOLE);
        /* update25();	*/
		do_idle(0);
        if (cfg.offhook)
            offhook();
    }
    if (loggedIn) {
        mPrintf("\n Already logged in!\n ");
        return;
    }
    getNormStr((moreYet) ? "" : "your initials", InitPw, NAMESIZE*2+2, NO_ECHO);
    dospCR();

    semicolon = strchr(InitPw, ';');

    if (!semicolon) {
        strncpy(Initials, InitPw,NAMESIZE);
		Initials[NAMESIZE] = '\0';
        getNormStr("password", passWord, NAMESIZE, NO_ECHO);
        dospCR();
    } else
        normalizepw(InitPw, Initials, passWord, semicolon);

    /* don't allow anything over NAMESIZE characters */
    Initials[NAMESIZE] = '\0';

    login(Initials, passWord);
}

/************************************************************************
 *
 *      doLogout() handles T(erminate) command
 *
 *	Parameters:
 *
 * 		char expand		TRUE to accept following parameters
 * 		char first		first parameter if TRUE
 *
 ************************************************************************/
void doLogout(char expand, char first)
{
    char done = FALSE, verbose = FALSE;

    if (expand)
        first = '\0';

    mPrintf("\bTerminate ");

    if (first)
        oChar(first);

    if (first == 'q')
        verbose = 1;

    while (!done) {
        done = TRUE;

        switch (toupper(first ? (int) first : (int) iChar())) {
            case '?':
                mPrintf("\n Logout options:\n \n ");

                mPrintf("Q>uit-also\n ");
                mPrintf("S>tay on line\n ");
                mPrintf("V>erbose\n ");
                mPrintf("? -- this\n ");
                break;
            case 'Y':
            case 'Q':
                mPrintf("\bQuit-also\n ");
                if (!expand) {
                    if (!getYesNo(confirm, 0))
                        break;
                }
                if (!(haveCarrier || onConsole))
                    break;
                terminate( /* hangUp == */ TRUE, verbose);
                break;
            case 'S':
                mPrintf("\bStay\n ");
                terminate( /* hangUp == */ FALSE, verbose);
                break;
            case 'V':
                mPrintf("\bVerbose ");
                verbose = 2;
                done = FALSE;
                break;
            default:
                if (gl_user.expert)
                    mPrintf("? ");
                else
                    mPrintf("? for help");
                break;
        }
        first = '\0';
    }
}

/************************************************************************/
/*      doXpert                                                         */
/************************************************************************/
void doXpert(void)
{
    mPrintf("\beXpert %s", (gl_user.expert) ? gl_str.off : gl_str.on);
    doCR();
    gl_user.expert = (char) (!gl_user.expert);
}

/************************************************************************/
/*      doRegular()                                                     */
/************************************************************************/
char doRegular(char expand, char c)
{
    char toReturn;
    int i;
    int done = 0;
    label doorinfo;

    toReturn = FALSE;

    for (i = 0; !expand && i < MAXEXTERN && extCmd[i].name[0]; ++i) {
        if (c == toupper(extCmd[i].name[0]) && (onConsole || !extCmd[i].local)) {
            done = 1;
            mPrintf("\b%s", extCmd[i].name);
            doCR();
            if (changedir(cfg.aplpath) == ERROR) {
                mPrintf("  -- Can't find application directory.\n\n");
                changedir(cfg.homepath);
            }
        /* apsystem(extCmd[i].command); */
            sprintf(doorinfo, "DORINFO%d.DEF", onConsole ? 0 : userdat.apl_com);
            extFmtRun(extCmd[i].command, doorinfo);
        }
    }
    if (!done) {
        switch (c) {

            case 'S':
                if (gl_user.sysop && expand) {
                    mPrintf("\b\bSysop Menu");
                    doCR();
                    doSysop();
                } else {
                    toReturn = TRUE;
                }
                break;

            case 'A':
                if (gl_user.aide) {
                    doAide(expand, 'E');
                } else {
                    toReturn = TRUE;
                }
                break;

            case 'C':
                doChat(expand, '\0');
                break;
            case 'D':
                doDownload(expand);
                break;
            case 'E':
                doEnter(expand, 'm');
                break;
            case 'F':
                doRead(expand, 'f');
                break;
            case 'G':
                doGoto(expand, FALSE);
                break;
            case 'H':
                doHelp(expand);
                break;
            case 'I':
                doIntro();
                break;
            case 'J':
                mPrintf("\bJump back to ");
                unGotoRoom();
                break;
            case 'K':
                doKnown(expand, 'r');
                break;
            case 'L':
                if (!loggedIn) {
                    doLogin(expand);
                } else {
                    if (!getYesNo(confirm, 0))
                        break;
                    doLogout(expand, 's');
                    doLogin(expand);
                }
                break;
            case 'N':
            case 'O':
            case 'R':
                doRead(expand, tolower(c));
                break;

            case 'B':
                doGoto(expand, TRUE);
                break;
            case 'T':
                doLogout(expand, 'q');
                break;
            case 'U':
                doUpload(expand);
                break;
            case 'X':
                if (!expand) {
                    doEnter(expand, 'x');
                } else {
                    doXpert();
                }
                break;

            case '=':
            case '+':
                doNext();
                break;
            case '\b':
                mPrintf("  ");
            case '-':
                doPrevious();
                break;

            case ']':
            case '>':
                doNextHall();
                break;
            case '[':
            case '<':
                doPreviousHall();
                break;
            case '~':
                mPrintf("\bAnsi %s\n ", gl_term.ansiOn ?
					gl_str.off : gl_str.on);
                gl_term.ansiOn = !gl_term.ansiOn;
                break;

            case '!':
                mPrintf("\bIBM Graphics %s\n ", gl_term.IBMOn ?
					gl_str.off:gl_str.on);
                gl_term.IBMOn = !gl_term.IBMOn;
                break;

            case '?':
                nextmenu("mainopt", &(cfg.cnt.mainopttut), 1);
				listExterns();
                break;

            case 0:     /* never gets here in shell mode... */
                if (newCarrier) {
                    greeting();

                    if (cfg.forcelogin) {
                        doCR();
                        doCR();
                        i = 0;
                        while (!loggedIn && gotCarrier()) {
                            doLogin(2);
                            if (++i > 3) {
                                Initport();
                                toReturn = TRUE;
                                break;
                            }
                        }
                    }
                    newCarrier = FALSE;
                }
                if (logBuf.lbflags.NODE && loggedIn) {
                    net_slave();

                    haveCarrier = FALSE;
                    modStat = FALSE;
                    newCarrier = FALSE;
                    justLostCarrier = FALSE;
                    onConsole = FALSE;
                    disabled = FALSE;
                    callout = FALSE;

                    delay(2000);

                    Initport();

                    cfg.callno++;
                    terminate(FALSE, FALSE);
                }
                if (justLostCarrier || ExitToMsdos) {
                    justLostCarrier = FALSE;
                    if (loggedIn)
                        terminate(FALSE, FALSE);
                }
                break;      /* irrelevant value */

            default:
                toReturn = TRUE;
                break;
        }
    }
    /* if they get unverified online */
    if (logBuf.VERIFIED)
        terminate(FALSE, FALSE);

    /* update25();	*/
	do_idle(0);
    return toReturn;
}

/************************************************************************/
/*     doNext() handles the '+' for next room                           */
/************************************************************************/
void doNext(void)
{
    mPrintf("\bEntering Next Room: ");
    stepRoom(1);
}

/************************************************************************/
/*     doPrevious() handles the '-' for previous room                   */
/************************************************************************/
void doPrevious(void)
{
    mPrintf("\bEntering Previous Room: ");
    stepRoom(0);
}

/************************************************************************/
/*     doNextHall() handles the '>' for next hall                       */
/************************************************************************/
void doNextHall(void)
{
    mPrintf("\bEntering Next Hall: ");
    stephall(1);
}

/************************************************************************/
/*     doPreviousHall() handles the '<' for previous hall               */
/************************************************************************/
void doPreviousHall(void)
{
    mPrintf("\bEntering Previous Hall: ");
    stephall(0);
}

/************************************************************************
 *
 * doAd() shows an ad/blurb cfg.ad_chance % of the time
 *
 ************************************************************************/

void doAd(int force)
{
    if ((cfg.ad_chance && (random(100) + 1 <= cfg.ad_chance)) || force)
        nextblurb("AD", &(cfg.cnt.count), 0);
}

/************************************************************************/
/*      getCommand() prints menu prompt and gets command char           */
/*      Returns: char via parameter and expand flag as value  --        */
/*               i.e., TRUE if parameters follow else FALSE.            */
/************************************************************************/
char getCommand(char *c)
{
    char expand;

    outFlag = IMPERVIOUS;

    /* update user's balance */
    if (cfg.accounting && !logBuf.lbflags.NOACCOUNT)
        updatebalance();

    doAd(0);

    givePrompt();

    do {
        dowhat = MAINMENU;
        *c = (char) toupper(iChar());
        dowhat = DUNO;
    } while (*c == 'P');

    expand = (char)
    ((*c == ' ') || (*c == '.') || (*c == ',') || (*c == '/'));

    if (expand) {
        *c = (char) toupper(iChar());
    }
    if (justLostCarrier || ExitToMsdos) {
        justLostCarrier = FALSE;
        if (loggedIn)
            terminate(FALSE, FALSE);
    }
    return expand;
}

/************************************************************************/
/*      greeting() gives system-entry blurb etc                         */
/************************************************************************/
void greeting(void)
{
    int messages;
    char dtstr[80];

    if (loggedIn)
        terminate(FALSE, FALSE);
    echo = BOTH;

    setdefaultconfig();
    initroomgen();
    cleargroupgen();
    if (cfg.accounting)
        unlogthisAccount();

    delay(100);

    if (newCarrier)
        hello();

    mPrintf("\n Welcome to %s, %s", cfg.nodeTitle, cfg.nodeRegion);
    mPrintf("\n Running %s v%s", softname, version);
# ifdef ALPHA_TEST
    mPrintf("\n Alpha Test Site");
# endif
# ifdef BETA_TEST
    mPrintf("\n Beta Test Site");
# endif
#ifdef FLOPPY
	mPrintf("\n Floppy edition");
#endif
    doCR();
    doCR();

    cit_strftime(dtstr, 79, cfg.vdatestamp, 0L);
    mPrintf(" %s", dtstr);

    if (!cfg.forcelogin) {
        mPrintf("\n H for Help");
        mPrintf("\n ? for Menu");
        mPrintf("\n L to Login");
    }
    getRoom(LOBBY);

    messages = talleyBuf.room[thisRoom].messages;

    doCR();

    mPrintf("  %d %s ", messages,
    (messages == 1) ? "message" : "messages");

    doCR();

    while (MIReady())
        getMod();
    logBuf.linesScreen = 23;
}


/************************************************************************
 *
 *      updatebalance()  updates user's accounting balance
 *      This routine will warn the user of excessive use, and terminate
 *      user when warnings have run out
 *
 *      This needs to become kernal code, called in getCommand().
 *
 ************************************************************************/

void updatebalance(void)
{
    double drain;
    long timestamp, diff;

    if (thisAccount.special[hour()] && !specialTime) {
        specialTime = TRUE;
        doCR();
        if (loggedIn) {
            mPrintf("Free time is now beginning, you have no time limit.");
            doCR();
        }
    }
    /* if it's no longer special time....                     */
    if (specialTime && !thisAccount.special[hour()]) {

        doCR();

        if (loggedIn) {
            mPrintf("Free time is over. You have %.0f %s%s left today.",
            cfg.credit_name, logBuf.credits, (logBuf.credits == 1) ? "" : "s");
            doCR();
        }
        specialTime = FALSE;

        lasttime=cit_time();
    }
    if (specialTime)        /* don't charge them for FREE time!             */
        return;


    /* get current time stamp */
    timestamp=cit_time();

    diff = timestamp - lasttime;

    /* If the time was set wrong..... */
    if (diff < 0)
        diff = 0;

    drain = (double) diff / 60.0;

    logBuf.credits = logBuf.credits - (float) drain;

    lasttime=cit_time();

    if (!gotCarrier() || onConsole)
        return;

    if (logBuf.credits < 5.0) {
        doCR();
        mPrintf(" Only %.0f %s%s left today!", logBuf.credits, cfg.credit_name,
        ((int) logBuf.credits == 1) ? "" : "s");
        doCR();
    }
    if (!thisAccount.days[dayofweek()]  /* if times up of it's no  */
        ||!thisAccount.hours[hour()]    /* login time for them..   */
    ||logBuf.credits <= 0.0) {
        nextblurb("goodbye", &(cfg.cnt.gcount), 1);
        terminate(TRUE, 1);
    }
}

/* EOF */
