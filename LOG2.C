/* -------------------------------------------------------------------- */
/*  LOG2.C                    Citadel                                   */
/* -------------------------------------------------------------------- */
/*                       Overlayed login log code                       */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Includes                                                            */
/* -------------------------------------------------------------------- */
#include <string.h>
#include <time.h>
#include "ctdl.h"

#ifndef ATARI_ST
//#include <conio.h>
#endif

#include "proto.h"
#include "global.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  forwardaddr()   sets up forwarding address for private mail         */
/*  killuser()      sysop special to kill a log entry                   */
/*  login()         is the menu-level routine to log someone in         */
/*  minibin()       minibin log-in stats                                */
/*  newPW()         is menu-level routine to change password & initials */
/*  pwslot()        returns logtab slot password is in, else ERROR      */
/*  Readlog()       handles read userlog                                */
/*  setalloldrooms()    set all rooms to be old.                        */
/*  setlbvisit()    sets lbvisit at log-in                              */
/*  setroomgen()    sets room gen# with log gen                         */
/*  showuser()      aide fn: to display any user's config.              */
/*  terminate()     is menu-level routine to exit system                */
/* -------------------------------------------------------------------- */


/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/LOG2.C_V  $
 * 
 *    Rev 1.44   01 Nov 1991 11:20:20   FJM
 * Added gl_ structures
 *
 *    Rev 1.43   06 Jun 1991  9:19:16   FJM
 *
 *    Rev 1.42   27 May 1991 11:42:52   FJM
 *
 *    Rev 1.39   17 Apr 1991 12:55:30   FJM
 *    Rev 1.38   10 Apr 1991  9:05:20   FJM
 *    Rev 1.29   19 Jan 1991 14:16:04   FJM
 * Clean up.
 *
 *    Rev 1.24   13 Jan 1991  0:31:40   FJM
 * Name overflow fixes.
 *
 *    Rev 1.21   06 Jan 1991 16:50:40   FJM
 * Modified login() to recognize ; in Initials.
 *
 *    Rev 1.18   06 Jan 1991 12:45:22   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.17   27 Dec 1990 20:16:00   FJM
 * Changes for porting.
 *
 *    Rev 1.14   22 Dec 1990 13:37:44   FJM
 *    Rev 1.6   24 Nov 1990  3:07:32   FJM
 * Changes for shell/door mode.
 *
 *    Rev 1.5   23 Nov 1990 18:24:46   FJM
 * Changes to login & terminate for shell/door mode.
 *
 *    Rev 1.4   23 Nov 1990 13:25:04   FJM
 * Header change
 *
 *    Rev 1.2   17 Nov 1990 16:11:54   FJM
 * Added version control log header
 * --------------------------------------------------------------------
 *  EARLY HISTORY:
 *
 *  06/14/89  (PAT)     Created from LOG.C to move some of the system out
 *                      of memory. Also cleaned up moved code to -W3, ext.
 *  03/07/90  {zm}      Change "No record..." (login) default to [N]
 *  03/15/90  {zm}      Add [title] name [surname] everywhere.
 *  03/19/90    FJM     Linted & partial cleanup
 *  05/19/90    FJM     cleanup
 *  06/06/90    FJM     Changed strftime to cit_strftime
 *  06/16/90    FJM     Made citadel exit if logout and run as door,
 *                      in terminate.
 *                      Cleanup in minibin().
 *  06/16/90    FJM     Fixes to allow entry of 30 char nym & initials.
 *  06/16/90    FJM     Made IBM Graphics characters a seperate option.
 *  07/30/90    FJM     Added time/date to logout message.
 *  08/07/90    FJM     Made verified,bulettin,userinfo,closesys
 *                      blurbs rotate.
 *  09/06/90    FJM     Changed logout time message (which I added some
 *                      time back, but didn't note here).
 *  10/13/90    FJM     Added config settings for limiting read user log
 *                      for both regular users & aides.
 *
 * -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Static Data                                                         */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  forwardaddr()   sets up forwarding address for private mail         */
/* -------------------------------------------------------------------- */
void forwardaddr(void)
{
    label name;
    int logno;

    getNormStr("forwarding name", name, NAMESIZE, ECHO);

    if (!strlen(name)) {
        mPrintf(" Private mail now routed to you");
        logBuf.forward[0] = '\0';
    } else {
        logno = findPerson(name, &lBuf);

        if (logno == ERROR) {
            mPrintf("No '%s' known.", name);
            return;
        }
        mPrintf(" Private mail now routed to %s", lBuf.lbname);
        strcpy(logBuf.forward, lBuf.lbname);
    }
    /* save it */
    if (loggedIn)
        storeLog();
}

/* -------------------------------------------------------------------- */
/*  killuser()      sysop special to kill a log entry                   */
/* -------------------------------------------------------------------- */
void killuser(void)
{
    label who;
    int logno, tabslot;

    getNormStr("who", who, NAMESIZE, ECHO);

    logno = findPerson(who, &lBuf);

    if (logno == ERROR || !strlen(who)) {
        mPrintf("No \'%s\' known. \n ", who);
        return;
    }
    if (strcmpi(logBuf.lbname, who) == SAMESTRING) {
        mPrintf("Cannot kill your own account, log out first.\n");
        return;
    }
    if (!getYesNo(confirm, 0))
        return;

    mPrintf("\'%s\' terminated.\n ", who);

    /* trap it */
    sprintf(msgBuf->mbtext, "User %s terminated", who);
    trap(msgBuf->mbtext, T_SYSOP);

    /* get log tab slot for person */
    tabslot = personexists(who);

    logTab[tabslot].ltpwhash = 0;
    logTab[tabslot].ltinhash = 0;
    logTab[tabslot].ltnmhash = 0;
    logTab[tabslot].permanent = 0;

    lBuf.lbname[0] = '\0';
    lBuf.lbin[0] = '\0';
    lBuf.lbpw[0] = '\0';
    lBuf.lbflags.L_INUSE = FALSE;
    lBuf.lbflags.PERMANENT = FALSE;

    putLog(&lBuf, logno);
}

/* -------------------------------------------------------------------- */
/*  login()         is the menu-level routine to log someone in         */
/* -------------------------------------------------------------------- */
void login(char *initials, char *password)
{
    int foundIt;
    int loop;
	char *p;

	if (!password) {
		p = strchr(initials,';');
		if (p) {
			*p = '\0';
			password = ++p;
		} else {
			return;
		}
	}

    if (justLostCarrier || ExitToMsdos)
        return;

    /* reset transmitted & received */
    transmitted = 0l;
    received = 0l;

    /* reset read & entered */
    mread = 0;
    entered = 0;

    /* Clear message per room array */
    for (loop = 0; loop < MAXROOMS; loop++) {
        MessageRoom[loop] = 0;
    }

    foundIt = ((pwslot(initials, password)) != ERROR);

    if (foundIt && *password) {
    /* update userlog entries: */

        loggedIn = TRUE;
        heldMessage = FALSE;

        setsysconfig();
        setgroupgen();
        setroomgen();
        setlbvisit();

        slideLTab(thisSlot);

        /* update25();	*/
		do_idle(0);

    /* trap it */
        if (!logBuf.lbflags.NODE) {
            sprintf(msgBuf->mbtext, "Login %s", logBuf.lbname);
            trap(msgBuf->mbtext, T_LOGIN);
        } else {
            sprintf(msgBuf->mbtext, "NetLogin %s", logBuf.lbname);
            trap(msgBuf->mbtext, T_NETWORK);
        }

    /* can't log in now. */
        if (cfg.accounting && !logBuf.lbflags.NOACCOUNT) {
            negotiate();
            logincrement();
            if (!logincheck()) {
                Initport();
                justLostCarrier = TRUE;
                if (parm.door)
                    ExitToMsdos = TRUE;
                return;
            }
        }
    /* can't log in now. */
        if (logBuf.VERIFIED && !onConsole) {
            nextblurb("verified", &(cfg.cnt.vcount), 1);
            Initport();
            justLostCarrier = TRUE;
            if (parm.door)
                ExitToMsdos = TRUE;
            return;
        }
    /* reverse engineering Minibin?!?! */
        if (cfg.loginstats && !logBuf.lbflags.NODE)
            minibin();

        if (!logBuf.lbflags.NODE) {
            changedir(cfg.helppath);

            nextblurb("bulletin", &(cfg.cnt.bcount), 0);
        }
        gotodefaulthall();

        if (logBuf.lbflags.NODE) {
            logtimestamp=cit_time();
            return;
        }
        roomtalley();

        mf.mfLim = 0;       /* just to make sure. */
        mf.mfMai = 0;
        mf.mfPub = 0;
        mf.mfUser[0] = 0;

        showMessages(NEWoNLY, FALSE, FALSE);

        if (gl_user.expert)
            listRooms(NEWRMS, FALSE, FALSE);
        else
            listRooms(OLDNEW, FALSE, FALSE);

        outFlag = OUTOK;

    } else {
        if ((cfg.private != 0) && whichIO == MODEM && !sysopNew) {
            if (getYesNo(" No record: Request access", 0)) {
        /* default [N] */
                if (justLostCarrier || ExitToMsdos)
                    return;
                if (cfg.private < 5)
                    nextblurb("userinfo", &(cfg.cnt.ucount), 1);
                switch (cfg.private) {
                    case 10:
                    case 9:
                        nextblurb("closesys", &(cfg.cnt.ccount), 1);
                        break;
                    case 8:
                    case 7:
                    case 6:
                    case 5:
                    case 4:
                    case 3:
                        if (cfg.private == 3 || cfg.private == 4
                        || cfg.private == 7 || cfg.private == 8) {
                            newUser(initials, password);
                            if (!loggedIn)
                                break;
                            logBuf.VERIFIED = TRUE;
                            newaccount();
                            /* update25();	*/
							do_idle(0);
                        }
                        if (cfg.private == 3 || cfg.private == 4) {
                            newUserFile();
                        }
                        if (cfg.private == 5 || cfg.private == 6
                        || cfg.private == 7 || cfg.private == 8) {
                            if (changedir(cfg.aplpath) == ERROR) {
                                mPrintf("  -- Can't find application directory.\n\n");
                                changedir(cfg.homepath);
                                break;
                            }
                            apsystem(cfg.newuserapp);
                            changedir(cfg.homepath);
                            break;
                        }
                        if (cfg.private == 3 || cfg.private == 4
                        || cfg.private == 7 || cfg.private == 8) {
                            logtimestamp=cit_time();
                            cfg.callno++;
                            storeLog();
                            terminate(FALSE, FALSE);
                        }
                        break;
                    case 2:
                    case 1:
                    default:
                        mailFlag = TRUE;
                        oldFlag = FALSE;
                        limitFlag = FALSE;
                        linkMess = FALSE;
                        makeMessage();
                        break;
                }
                if ((cfg.private == 2)
                    || (cfg.private == 4)
                    || (cfg.private == 6)
                    || (cfg.private == 8)
                || (cfg.private == 10)) {
                    mPrintf("\n Thank you, Good-Bye.\n");
                    Initport();
                    justLostCarrier = TRUE;
                    if (parm.door)
                        ExitToMsdos = TRUE;
                }
            }
            return;
        } else if (getYesNo(" No record: Enter as new user", 0)) {
        /* default [N] */
            newUser(initials, password);
            if (!loggedIn)
                return;
            newaccount();
            /* update25();	*/
			do_idle(0);
            if (cfg.accounting && !logBuf.lbflags.NOACCOUNT) {
                negotiate();
                if (!logincheck()) {
                    Initport();
                    justLostCarrier = TRUE;
                    if (parm.door)
                        ExitToMsdos = TRUE;
                    return;
                }
            }
            roomtalley();
            listRooms(OLDNEW, FALSE, FALSE);
        } else {
            if (whichIO == CONSOLE) {
                whichIO = MODEM;
                onConsole = (BOOL) (whichIO == CONSOLE);
                Initport();
            }
        }
    }

    if (!loggedIn)
        return;

    /* record login time, date */
    logtimestamp=cit_time();

    cfg.callno++;

    storeLog();
}

/* -------------------------------------------------------------------- */
/*  minibin()       minibin log-in stats                                */
/* -------------------------------------------------------------------- */
void minibin(void)
{
    int calls, messages;
    char dtstr[80];

    messages = (int) (cfg.newest - logBuf.lbvisit[1]);
    calls = (int) (cfg.callno - logBuf.callno);

    if (!gl_user.expert)
        mPrintf(" \n \n <J>ump <N>ext <P>ause <S>top");
    doCR();

    /* print out name      */
    mPrintf("Welcome back, ");
    if (cfg.surnames && logBuf.title[0]) {
        mPrintf("[%s] ", logBuf.title);
    }
    mPrintf("%s", logBuf.lbname);
    if (cfg.surnames && logBuf.surname[0]) {
        mPrintf(" [%s]", logBuf.surname);
    }
    mPrintf("!");
    doCR();

    mPrintf("You are position # %d in the userlog.", thisLog);
    doCR();
    if (calls == 0) {
        mPrintf("You were just here.");
        doCR();
    } else {
        cit_strftime(dtstr, 79, cfg.vdatestamp, logBuf.calltime);
        mPrintf("You last called on: %s", dtstr);
        doCR();
        mPrintf("You are caller %lu", (cfg.callno + 1l));
        doCR();
        mPrintf("%d %s made ", people,
        (people == 1) ? "person has" : "people have");
/*      doCR();   (save a line)  */
        mPrintf("%d %s and left", calls, (calls == 1) ? "call" : "calls");
        doCR();
        mPrintf("%d new %s since you were last here.", messages,
        (messages == 1) ? "Message" : "Messages");
        doCR();
    }

    if (cfg.accounting && !logBuf.lbflags.NOACCOUNT) {
        if (!specialTime) {
            mPrintf("You have %.1f %s%sleft today.", logBuf.credits,
            cfg.credit_name, (logBuf.credits == 1.0) ? " " : "s ");
        } else {
            mPrintf("You have unlimited time.");
        }

        doCR();
    }
    outFlag = OUTOK;
}

/* -------------------------------------------------------------------- */
/*  newPW()         is menu-level routine to change password & initials */
/* -------------------------------------------------------------------- */
void newPW(void)
{
    char InitPw[NAMESIZE*2+2];
    char passWord[NAMESIZE*2+2];
    char Initials[NAMESIZE*2+2];
    char oldPw[NAMESIZE*2+2];
    char *semicolon;

    int goodpw;

    if (!loggedIn) {
        mPrintf("\n --Must be logged in.\n ");
        return;
    }
    /* display old pw & initials */
    displaypw(logBuf.lbname, logBuf.lbin, logBuf.lbpw);

    if (!getYesNo("Change", 0))
        return;

    strcpy(oldPw, logBuf.lbpw);

    getNormStr("your new initials", InitPw, 40, NO_ECHO);
    dospCR();

    semicolon = strchr(InitPw, ';');

    if (semicolon) {
        normalizepw(InitPw, Initials, passWord, semicolon);
    } else
        strcpy(Initials, InitPw);

    /* dont allow anything over NAMESIZE characters */
    Initials[NAMESIZE] = '\0';

    do {
        if (!semicolon) {
            getNormStr("new password", passWord, NAMESIZE, NO_ECHO);
            dospCR();
        }
        goodpw = (((pwexists(passWord) == ERROR) && strlen(passWord) >= 2)
        || (strcmpi(passWord, oldPw) == SAMESTRING));

        if (!goodpw)
            mPrintf("\n Poor password\n ");
        semicolon = FALSE;
    } while (!goodpw && (!ExitToMsdos && (haveCarrier || whichIO == CONSOLE)));

    strcpy(logBuf.lbin, Initials);
    strcpy(logBuf.lbpw, passWord);

    /* insure against loss of carrier */
    if (!ExitToMsdos && (haveCarrier || whichIO == CONSOLE)) {
        logTab[0].ltinhash = hash(Initials);
        logTab[0].ltpwhash = hash(passWord);

        storeLog();
    }
    /* display new pw & initials */
    displaypw(logBuf.lbname, logBuf.lbin, logBuf.lbpw);

    /* trap it */
    trap("Password changed", T_PASSWORD);
}

/* -------------------------------------------------------------------- */
/*  pwslot()        returns logtab slot password is in, else ERROR      */
/* -------------------------------------------------------------------- */
int pwslot(char *in, char *pw)
{
    int slot;

    if (strlen(pw) < 2)
        return ERROR;       /* Don't search for these pwds */

    slot = pwexists(pw);

    if (slot == ERROR)
        return ERROR;

    /* initials must match too */
    if ((logTab[slot].ltinhash) != hash(in))
        return ERROR;

    getLog(&lBuf, logTab[slot].ltlogSlot);

    if ((strcmpi(pw, lBuf.lbpw) == SAMESTRING)
    && (strcmpi(in, lBuf.lbin) == SAMESTRING)) {
        memcpy(&logBuf, &lBuf, sizeof logBuf);
        thisSlot = slot;
        thisLog = logTab[slot].ltlogSlot;
        return (slot);
    } else
        return ERROR;
}

/* -------------------------------------------------------------------- */
/*  Readlog()       handles read userlog                                */
/* -------------------------------------------------------------------- */
void Readlog(char verbose)
{
    int i, grpslot;
    char dtstr[80];
    char flags[11];
    char wild = FALSE;
    char buser = FALSE;

    grpslot = ERROR;

    if (!cfg.readuser && !gl_user.sysop && !(gl_user.aide && cfg.aidereaduser))
        return;
    if (mf.mfUser[0]) {
        getNormStr("user", mf.mfUser, NAMESIZE, ECHO);

        if (personexists(mf.mfUser) == ERROR) {
            if (strpos('?', mf.mfUser)
                || strpos('*', mf.mfUser)
            || strpos('[', mf.mfUser)) {
                wild = TRUE;
            } else {
                mPrintf(" \nNo such user!\n ");
                return;
            }
        } else {
            buser = TRUE;
        }
    }
    outFlag = OUTOK;

    if (mf.mfLim && (cfg.readluser || gl_user.sysop || gl_user.aide)) {
        doCR();
        getgroup();
        if (!mf.mfLim)
            return;
        grpslot = groupexists(mf.mfGroup);
    } else
        mf.mfLim = FALSE;

    if (!gl_user.expert)
        mPrintf(" \n \n <J>ump <N>ext <P>ause <S>top");

    for (i = 0; ((i < cfg.MAXLOGTAB) && (outFlag != OUTSKIP)); i++) {
        if (BBSCharReady())
            if (mAbort())
                return;

        if (logTab[i].ltpwhash != 0 &&
        logTab[i].ltnmhash != 0) {
            if (buser && hash(mf.mfUser) != logTab[i].ltnmhash)
                continue;

            getLog(&lBuf, logTab[i].ltlogSlot);

            if (buser && strcmpi(mf.mfUser, lBuf.lbname) != SAMESTRING)
                continue;

            if (wild && !u_match(lBuf.lbname, mf.mfUser))
                continue;

            if (mf.mfLim
                && lBuf.groups[grpslot] != grpBuf.group[grpslot].groupgen)
                continue;

        /* Show yourself even if unlisted */
            if ((!i && loggedIn && verbose) ||
                (verbose && lBuf.lbflags.L_INUSE
            && (gl_user.aide || !lBuf.lbflags.UNLISTED))) {
                cit_strftime(dtstr, 79, cfg.vdatestamp, lBuf.calltime);

                if (cfg.surnames) {
                    doCR();
/* (old vers.)    mPrintf(" [%-20s] %-20s",lBuf.surname, lBuf.lbname);   */
                    mPrintf(" [%s] %s [%s]", lBuf.title, lBuf.lbname, lBuf.surname);
                    doCR();
                    mPrintf(" #%lu %s", lBuf.callno, dtstr);
                } else {
                    doCR();
                    mPrintf(" %-20s #%lu %s", lBuf.lbname, lBuf.callno, dtstr);
                }
            } else if ((!i && loggedIn) || (lBuf.lbflags.L_INUSE &&
            (gl_user.aide || !lBuf.lbflags.UNLISTED))) {
                doCR();
                mPrintf(" %-20s", lBuf.lbname);
            }
            if (gl_user.aide) {     /* A>ide T>wit P>erm U>nlist N>etuser S>ysop */
                if (cfg.accounting) {
                    if (lBuf.lbflags.NOACCOUNT)
                        mPrintf(" %10s", "N/A");
                    else
                        mPrintf(" %10.2f", lBuf.credits);
                }
                strcpy(flags, "         ");

                if (lBuf.lbflags.AIDE)
                    flags[0] = 'A';
                if (lBuf.lbflags.PROBLEM)
                    flags[1] = 'T';
                if (lBuf.lbflags.PERMANENT)
                    flags[2] = 'P';
                if (lBuf.lbflags.NETUSER)
                    flags[4] = 'N';
                if (lBuf.lbflags.UNLISTED)
                    flags[3] = 'U';
                if (lBuf.lbflags.SYSOP)
                    flags[5] = 'S';
                if (lBuf.lbflags.NOMAIL)
                    flags[6] = 'M';
                if (lBuf.VERIFIED)
                    flags[7] = 'V';
                if (lBuf.DUNGEONED)
                    flags[8] = 'D';
                if (lBuf.MSGAIDE)
                    flags[9] = 'm';

                mPrintf(" %s", flags);
            }
            if (lBuf.lbflags.NODE && (gl_user.aide || !lBuf.lbflags.UNLISTED)) {
                mPrintf(" (Node) ");
            }
            if (verbose)
                doCR();
        }
    }
    doCR();
}

/* -------------------------------------------------------------------- */
/*  setalloldrooms()    set all rooms to be old.                        */
/* -------------------------------------------------------------------- */
void setalloldrooms(void)
{
    int i;

    for (i = 1; i < MAXVISIT; i++)
        logBuf.lbvisit[i] = cfg.newest;

    logBuf.lbvisit[0] = cfg.newest;
}

/* -------------------------------------------------------------------- */
/*  setlbvisit()    sets lbvisit at log-in                              */
/* -------------------------------------------------------------------- */
void setlbvisit(void)
{
    int i;

    /* see if the message base was cleared since last call */
    for (i = 1; i < MAXVISIT; i++) {
        if (logBuf.lbvisit[i] > cfg.newest) {
            for (i = 1; i < MAXVISIT; i++)
                logBuf.lbvisit[i] = cfg.oldest;
            logBuf.lbvisit[0] = cfg.newest;
            logBuf.lbvisit[(MAXVISIT - 1)] = cfg.oldest;
            doCR();
            mPrintf("Message base destroyed since last call!");
            doCR();
            mPrintf("All message pointers reset.");
            doCR();
            return;
        }
    }

    /* slide lbvisit array down and change lbgen entries to match: */
    for (i = (MAXVISIT - 2); i; i--) {
        logBuf.lbvisit[i] = logBuf.lbvisit[i - 1];
    }
    logBuf.lbvisit[(MAXVISIT - 1)] = cfg.oldest;
    logBuf.lbvisit[0] = cfg.newest;

    for (i = 0; i < MAXROOMS; ++i) {
        if ((logBuf.lbroom[i].lvisit) < (MAXVISIT - 2)) {
            ++logBuf.lbroom[i].lvisit;
        }
    }
}

/* -------------------------------------------------------------------- */
/*  setroomgen()    sets room gen# with log gen                         */
/* -------------------------------------------------------------------- */
void setroomgen(void)
{
    int i;

    /* set gen on all unknown rooms  --  INUSE or no: */
    for (i = 0; i < MAXROOMS; i++) {

    /* Clear mail and xclude flags in logbuff for any  */
    /* rooms created since last call                   */

        if (logBuf.lbroom[i].lbgen != roomTab[i].rtgen) {
            logBuf.lbroom[i].mail = FALSE;
            logBuf.lbroom[i].xclude = FALSE;
        }
    /* if not a public room */
        if (roomTab[i].rtflags.PUBLIC == 0) {
        /* if you don't know about the room */
            if (((logBuf.lbroom[i].lbgen) != roomTab[i].rtgen) ||
            (!gl_user.aide && i == AIDEROOM)) {
        /* mismatch gen #'s properly */
                logBuf.lbroom[i].lbgen
                = (roomTab[i].rtgen + (MAXGEN - 1)) % MAXGEN;

                logBuf.lbroom[i].lvisit = MAXVISIT - 1;

            }
        } else if ((logBuf.lbroom[i].lbgen) != roomTab[i].rtgen) {
        /* newly created public room -- remember to visit it; */
            logBuf.lbroom[i].lbgen = roomTab[i].rtgen;
            logBuf.lbroom[i].lvisit = 1;
        }
    }
}

/* -------------------------------------------------------------------- */
/*  showuser()      aide fn: to display any user's config.              */
/* -------------------------------------------------------------------- */
void showuser(void)
{
    label who;
    int logno, oldloggedIn, oldthisLog;

    oldloggedIn = loggedIn;
    oldthisLog = thisLog;

    loggedIn = TRUE;

    getNormStr("who", who, NAMESIZE, ECHO);

    if (strcmpi(who, logBuf.lbname) == SAMESTRING) {
        showconfig(&logBuf);
    } else {
        logno = findPerson(who, &lBuf);

        if (!strlen(who) || logno == ERROR)
            mPrintf("No \'%s\' known. \n ", who);
        else
            showconfig(&lBuf);
    }

    /* memcpy(&logBuf, &lBuf, sizeof logBuf); */

    loggedIn = (BOOL) oldloggedIn;
    thisLog = oldthisLog;
}

/* -------------------------------------------------------------------- */
/*  terminate()     is menu-level routine to exit system                */
/* -------------------------------------------------------------------- */
void terminate(char discon, char verbose)
{
    float balance;
    char doStore;
    int traptype;
    char dtstr[80];

    chatReq = FALSE;

    doStore = (BOOL) (!ExitToMsdos && (haveCarrier || onConsole));

    if (discon || !doStore)
        sysopNew = FALSE;

    balance = logBuf.credits;
    /* save screen height since it's reset to ensure no pause in logout */

    outFlag = OUTOK;

    if (doStore && verbose == 2) {
        doCR();
        mPrintf(" You were caller %lu", cfg.callno);
        doCR();
        mPrintf(" You were logged in for: ");
        diffstamp(logtimestamp);
        doCR();
        mPrintf(" You entered %d messages", entered);
        doCR();
        mPrintf(" and read %d.", mread);
        doCR();
        if (cfg.accounting && !logBuf.lbflags.NOACCOUNT) {
            mPrintf(" %.1f %s%s used this call", startbalance - logBuf.credits,
            cfg.credit_name,
            ((int) (startbalance - logBuf.credits) == 1) ? "" : "s");
            doCR();
            mPrintf(" Your balance is %.0f %s%s", logBuf.credits,
            cfg.credit_name,
            ((int) logBuf.credits == 1) ? "" : "s");
            doCR();
        }
    }
    if (doStore && verbose)
        goodbye();

    outFlag = IMPERVIOUS;

    cit_strftime(dtstr, 79, cfg.vdatestamp, 0);
    if (loggedIn)
        mPrintf(" %s logged out %s\n ", logBuf.lbname, dtstr);

    thisHall = 0;       /* go to ROOT hallway */

    if (discon) {
        if (parm.door) {
            ExitToMsdos = 1;    /* drop back to DOS */
        } else {
            switch (whichIO) {
                case MODEM:
                    Hangup();
                    iChar();    /* And now detect carrier loss  */
                    break;
                case CONSOLE:
                    whichIO = MODEM;
                    if (!gotCarrier())
                        Initport();
                    break;
            }
        }
    }
    if (!doStore) {     /* if carrier dropped */
    /* trap it */
        sprintf(msgBuf->mbtext, "Carrier dropped");
        trap(msgBuf->mbtext, T_CARRIER);
    } else {            /* update JL properly at status line */
        justLostCarrier = FALSE;
    }

    /* update new pointer only if carrier not dropped */
    if (loggedIn && doStore) {
        logBuf.lbroom[thisRoom].lbgen = roomBuf.rbgen;
        logBuf.lbroom[thisRoom].lvisit = 0;
        logBuf.lbroom[thisRoom].mail = 0;
    }
    if (loggedIn) {
        logBuf.callno = cfg.callno;
        logBuf.calltime = logtimestamp;
        logBuf.lbvisit[0] = cfg.newest;
        logTab[0].ltcallno = cfg.callno;

        storeLog();
        loggedIn = FALSE;

    /* trap it */
        if (!logBuf.lbflags.NODE) {
            sprintf(msgBuf->mbtext, "Logout %s", logBuf.lbname);
            trap(msgBuf->mbtext, T_LOGIN);
        } else {
            sprintf(msgBuf->mbtext, "NetLogout %s", logBuf.lbname);
            trap(msgBuf->mbtext, T_NETWORK);
        }

        if (cfg.accounting)
            unlogthisAccount();
        heldMessage = FALSE;
        cleargroupgen();
        initroomgen();

        logBuf.lbname[0] = 0;

        setalloldrooms();
    }
    gl_term.ansiOn = FALSE;
    gl_term.IBMOn = FALSE;

    /* update25();	*/
	do_idle(0);

    setdefaultconfig();
    roomtalley();
    getRoom(LOBBY);

    if (!logBuf.lbflags.NODE)
        traptype = T_ACCOUNT;
    else
        traptype = T_NETWORK;


    sprintf(msgBuf->mbtext, "  ----- %4d messages entered", entered);
    trap(msgBuf->mbtext, traptype);

    sprintf(msgBuf->mbtext, "  ----- %4d messages read", mread);
    trap(msgBuf->mbtext, traptype);

    if (logBuf.lbflags.NODE) {
        sprintf(msgBuf->mbtext, "  ----- %4d messages expired", xpd);
        trap(msgBuf->mbtext, T_NETWORK);

        sprintf(msgBuf->mbtext, "  ----- %4d messages duplicate", duplic);
        trap(msgBuf->mbtext, T_NETWORK);
    }
    sprintf(msgBuf->mbtext, "Cost was %ld", (long) startbalance - (long) balance);
    trap(msgBuf->mbtext, T_ACCOUNT);
}

/* EOF */
