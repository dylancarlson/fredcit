/* -------------------------------------------------------------------- */
/*  LOG3.C                   Citadel                                    */
/* -------------------------------------------------------------------- */
/*                     Overlayed newuser log code                       */
/*                  and configuration / userlog edit                    */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Includes                                                            */
/* -------------------------------------------------------------------- */
#include <string.h>
#include <time.h>
#include "ctdl.h"
#include "proto.h"
#include "global.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  initroomgen()   initializes room gen# with log gen                  */
/*  newlog()        sets up a new log entry for new users returns ERROR */
/*                  if cannot find a usable slot                        */
/*  newslot()       attempts to find a slot for a new user to reside in */
/*                  puts slot in global var  thisSlot                   */
/*  newUser()       prompts for name and password                       */
/*  newUserFile()   Writes new user info out to a file                  */
/*  configure()     sets user configuration via menu                    */
/*  userEdit()      Edit a user via menu                                */
/* -------------------------------------------------------------------- */


/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/LOG3.C_V  $
 * 
 *    Rev 1.44   01 Nov 1991 11:20:22   FJM
 * Added gl_ structures
 *
 *    Rev 1.43   08 Jul 1991 16:19:18   FJM
 *
 *    Rev 1.42   15 Jun 1991  7:46:36   FJM
 * Strip ANSI from new user nyms.
 *
 *    Rev 1.41   27 May 1991 11:42:58   FJM
 *
 *    Rev 1.40   22 May 1991  2:17:12   FJM
 * Fixed log edit routines to use NAMESIZE
 *
 *    Rev 1.28   19 Jan 1991 14:16:10   FJM
 * Clean up.
 *
 *    Rev 1.25   18 Jan 1991 16:50:58   FJM
 * Improved .AL
 *
 *    Rev 1.17   06 Jan 1991 12:45:58   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.16   27 Dec 1990 20:16:04   FJM
 * Changes for porting.
 *
 *    Rev 1.13   22 Dec 1990 13:37:48   FJM
 *    Rev 1.5   24 Nov 1990  3:07:34   FJM
 * Changes for shell/door mode.
 *
 *    Rev 1.4   23 Nov 1990 13:25:06   FJM
 * Header change
 *
 *    Rev 1.2   17 Nov 1990 16:12:02   FJM
 * Added version control log header
 * -------------------------------------------------------------------- */

/* --------------------------------------------------------------------
 *  HISTORY:
 *
 *  06/14/89  (PAT)  Created from LOG.C to move some of the system
 *                   out of memory. Also cleaned up moved code to
 *                   -W3, ext.
 *  03/07/90  {zm}   "Enter your 'nym" instead of "Enter full name"
 *                   and clean up some other nuisances in #PRIVATE 4
 *  03/15/90  {zm}   Add [title] name [surname] everywhere.
 *  06/06/90  FJM    Changed strftime to cit_strftime
 *  06/16/90  FJM    Fixes to allow entry of 30 char nym & initials.
 *  06/16/90  FJM    Made IBM Graphics characters a seperate option.
 *  08/07/90  FJM    Made password blurb rotate.
 *  10/05/90  FJM    Made inits/password show in userlog edit (console only).
 *
 * -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Static Data                                                         */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  initroomgen()   initializes room gen# with log gen                  */
/* -------------------------------------------------------------------- */
void initroomgen(void)
{
    int i;

    for (i = 0; i < MAXROOMS; i++) {
    /* Clear mail and xclude flags in logbuff for every room */

        logBuf.lbroom[i].mail = FALSE;
        logBuf.lbroom[i].xclude = FALSE;

        if (roomTab[i].rtflags.PUBLIC == 1) {
        /* make public rooms known: */
            logBuf.lbroom[i].lbgen = roomTab[i].rtgen;
            logBuf.lbroom[i].lvisit = MAXVISIT - 1;

        } else {
        /* make private rooms unknown: */
            logBuf.lbroom[i].lbgen =
            (roomTab[i].rtgen + (MAXGEN - 1)) % MAXGEN;

            logBuf.lbroom[i].lvisit = MAXVISIT - 1;
        }
    }
}

/* -------------------------------------------------------------------- */
/*  newlog()        sets up a new log entry for new users,              */
/*                  returns ERROR if cannot find a usable slot          */
/* -------------------------------------------------------------------- */
int newlog(char *fullnm, char *in, char *pw)
{
    int ourSlot, i;

    /* get a new slot for this user */
    thisSlot = newslot();

    if (thisSlot == ERROR) {
        thisSlot = 0;
        return (ERROR);
    }
    ourSlot = logTab[thisSlot].ltlogSlot;

    getLog(&logBuf, ourSlot);

    /* copy info into record: */
    strcpy(logBuf.lbname, fullnm);
    strcpy(logBuf.lbin, in);
    strcpy(logBuf.lbpw, pw);
    logBuf.title[0] = '\0'; /* no starting title */
    logBuf.surname[0] = '\0';   /* no starting surname */
    logBuf.forward[0] = '\0';   /* no starting forwarding */
    strcpy(logBuf.tty, "TTY");

    logBuf.lbflags.L_INUSE = TRUE;
    logBuf.lbflags.PROBLEM = cfg.user[D_PROBLEM];
    logBuf.lbflags.PERMANENT = cfg.user[D_PERMANENT];
    logBuf.lbflags.NOACCOUNT = cfg.user[D_NOACCOUNT];
    logBuf.lbflags.NETUSER = cfg.user[D_NETWORK];
    logBuf.lbflags.NOMAIL = cfg.user[D_NOMAIL];
    logBuf.lbflags.AIDE = FALSE;
    logBuf.lbflags.NODE = FALSE;
    logBuf.lbflags.SYSOP = FALSE;

    logBuf.DUNGEONED = FALSE;
    logBuf.MSGAIDE = FALSE;
    logBuf.FORtOnODE = FALSE;
    logBuf.NEXTHALL = FALSE;
    logBuf.VERIFIED = FALSE;

    for (i = 1; i < MAXVISIT; i++)
        logBuf.lbvisit[i] = cfg.oldest;

    logBuf.lbvisit[0] = cfg.newest;
    logBuf.lbvisit[(MAXVISIT - 1)] = cfg.oldest;

    initroomgen();

    cleargroupgen();

    /* put user into group NULL */
    logBuf.groups[0] = grpBuf.group[0].groupgen;

    /* accurate read-userlog for first time call */
    logBuf.callno = cfg.callno + 1;
    logBuf.credits = (float) 0;
    logBuf.calltime=cit_time();

    /* trap it */
    sprintf(msgBuf->mbtext, "New user %s", logBuf.lbname);
    trap(msgBuf->mbtext, T_LOGIN);

    loggedIn = TRUE;
    slideLTab(thisSlot);
    storeLog();

    return (TRUE);
}

/* -------------------------------------------------------------------- */
/*  newslot()       attempts to find a slot for a new user to reside in */
/*                  puts slot in global var  thisSlot                   */
/* -------------------------------------------------------------------- */
int newslot(void)
{
    int i;
    int foundit = ERROR;

    for (i = cfg.MAXLOGTAB - 1; ((i > -1) && (foundit == ERROR)); --i) {
        if (!logTab[i].permanent)
            foundit = i;
    }
    if (foundit == ERROR) {
        mPrintf("\n All log slots taken.\n");
    }
    return foundit;
}

/* -------------------------------------------------------------------- */
/*  newUser()       prompts for name and password                       */
/* -------------------------------------------------------------------- */
void newUser(char *initials, char *password)
{
    label fullnm;
    char InitPw[NAMESIZE*2+2];
    char Initials[NAMESIZE*2+2];
    char passWord[NAMESIZE*2+2];
    char *semicolon;

    int abort, good = 0;
    char firstime = 1;

    if (justLostCarrier || ExitToMsdos)
        return;

	/* default to [Y] for list in userlog for new users */
    gl_user.unlisted = FALSE;
	/* default to [Y] for display of room descriptions  */
    gl_user.roomtell = TRUE;

    configure(TRUE);        /* make sure new users configure reasonably     */

    nextblurb("password", &(cfg.cnt.pcount), 1);

    do {
        do {
            getNormStr("your 'nym", fullnm, NAMESIZE, ECHO);    /* 03/07/90 */
			stripANSI(fullnm);
            if (!strlen(fullnm)) {
                mPrintf("Blank names are not allowed\n");
                good = FALSE;
            } else if ((personexists(fullnm) != ERROR)
                || (strcmpi(fullnm, "Sysop") == SAMESTRING)
				|| !strlen(fullnm)) {
                mPrintf("We already have a %s, try again\n", fullnm);
                good = FALSE;
            } else
                good = TRUE;
        } while (!good && (!ExitToMsdos && (haveCarrier || whichIO == CONSOLE)));

        if (firstime)
            strcpy(Initials, initials);
        else {
            getNormStr("your initials", InitPw, NAMESIZE*2+2, NO_ECHO);
            dospCR();

            semicolon = strchr(InitPw, ';');

            if (semicolon) {
                normalizepw(InitPw, Initials, passWord, semicolon);
            } else
                strncpy(Initials, InitPw, NAMESIZE);
			Initials[NAMESIZE] = '\0';
        }

        do {
            if (firstime)
                strcpy(passWord, password);
            else if (!semicolon) {
                getNormStr("password", passWord, NAMESIZE, NO_ECHO);
                dospCR();
            }
            firstime = FALSE;   /* keeps from going in infinite loop */
            semicolon = FALSE;

            if (pwexists(passWord) != ERROR || strlen(passWord) < 2) {
                good = FALSE;
                mPrintf("\n Poor password\n ");
            } else
                good = TRUE;
        } while (!good && (!ExitToMsdos && (haveCarrier || whichIO == CONSOLE)));

        displaypw(fullnm, Initials, passWord);

        abort = getYesNo("OK", 2);

        if (abort == 2)
            return;     /* check for Abort at (Y/N/A)[A]: */
    } while ((!abort) && (!ExitToMsdos && (haveCarrier || whichIO == CONSOLE)));

    if (!ExitToMsdos && (haveCarrier || whichIO == CONSOLE))
        if (newlog(fullnm, Initials, passWord) == ERROR)
            return;
}

/* -------------------------------------------------------------------- */
/*  newUserFile()   Writes new user info out to a file                  */
/*  as used by #PRIVATE 3 or 4, the most common selections.             */
/* -------------------------------------------------------------------- */
void newUserFile(void)
{
    FILE *fl;
    char name[40];
    char phone[30];
    char title[31];
    char surname[31];
    char temp[60];
    char dtstr[80];
    int tempmaxtext;
    int clm = 0;
    int l = 0;

    *name = '\0';
    *phone = '\0';
    *surname = '\0';

    if (cfg.surnames) {
        getNormStr("the title you desire", title, 30, ECHO);
        getNormStr("the surname you desire", surname, 30, ECHO);
    }
    getNormStr("your REAL name", name, 40, ECHO);

    if (name[0])
        getNormStr("your phone number [(xxx)xxx-xxxx]", phone, 30, ECHO);

    strcpy(msgBuf->mbto, cfg.sysop);
    strcpy(msgBuf->mbauth, logBuf.lbname);
    msgBuf->mbtext[0] = 0;
    msgBuf->mbtitle[0] = EOS;
    msgBuf->mbsur[0] = EOS;
    tempmaxtext = cfg.maxtext;
    cfg.maxtext = 2048;     /* often had problems with 1024 being too small! */

    getText();

    cfg.maxtext = tempmaxtext;

    if (changedir(cfg.homepath) == ERROR)
        return;

    fl = fopen("newuser.log", "at");
    cit_strftime(dtstr, 79, cfg.vdatestamp, 0l);

    sprintf(temp, "\n %s\n", dtstr);
    fwrite(temp, strlen(temp), 1, fl);

    if (surname[0]) {       /* argh... */
        sprintf(temp, " Nym:       [%s] %s\n", surname, logBuf.lbname);
    } else {
        sprintf(temp, " Nym:       %s\n", logBuf.lbname);
    }
    fwrite(temp, strlen(temp), 1, fl);

    sprintf(temp, " Real name: %s\n", name);
    fwrite(temp, strlen(temp), 1, fl);

    sprintf(temp, " Phone:     %s\n", phone);
    fwrite(temp, strlen(temp), 1, fl);

    sprintf(temp, " Baud:      %d\n", bauds[speed]);
    fwrite(temp, strlen(temp), 1, fl);

    sprintf(temp, "\n");

    if (msgBuf->mbtext[0]) {    /* xPutStr(fl, msgBuf->mbtext); */
        do {
            if ((msgBuf->mbtext[l] == 32 || msgBuf->mbtext[l] == 9) && clm > 73) {
                fwrite(temp, strlen(temp), 1, fl);
                clm = 0;
                l++;
            } else {
                fputc(msgBuf->mbtext[l], fl);
                clm++;
                if (msgBuf->mbtext[l] == 10)
                    clm = 0;
                if (msgBuf->mbtext[l] == 9)
                    clm = clm + 7;
                l++;
            }
        } while (msgBuf->mbtext[l]);
    }
    fclose(fl);
    doCR();
}

/* -------------------------------------------------------------------- */
/*  configure()     sets user configuration via menu                    */
/* -------------------------------------------------------------------- */
void configure(BOOL new)
{
    BOOL prtMess = TRUE;
    BOOL quit = FALSE;
    int c;
    char temp[30];
    char oldEcho;

    doCR();

    do {
        if (prtMess) {
            doCR();
            outFlag = OUTOK;
            mpPrintf("Screen <W>idth....... %d", gl_term.termWidth);
            doCR();
            mpPrintf("Height of <S>creen... %s", logBuf.linesScreen
				? itoa(logBuf.linesScreen, temp, 10) : "Screen Pause Off");
            doCR();
            mpPrintf("<U>ppercase only..... %s", gl_term.termUpper ?
				gl_str.on : gl_str.off);
            doCR();
            mpPrintf("<L>inefeeds.......... %s", gl_term.termLF ?
				gl_str.on : gl_str.off);
            doCR();
            mpPrintf("<T>abs............... %s", gl_term.termTab ?
				gl_str.on : gl_str.off);
            doCR();
            mpPrintf("<N>ulls.............. %s", gl_term.termNulls ?
				itoa(gl_term.termNulls, temp, 10) : gl_str.off);
            doCR();
            mpPrintf("Terminal <E>mulation. %s", gl_term.ansiOn ?
				"ANSI-BBS": gl_str.off);
            doCR();
            mpPrintf("IBM <G>raphics....... %s", gl_term.IBMOn ?
				gl_str.on : gl_str.off);
            doCR();
            mpPrintf("<H>elpful Hints...... %s", !gl_user.expert ?
				gl_str.on : gl_str.off);
            doCR();
            mpPrintf("l<I>st in userlog.... %s", !gl_user.unlisted ?
				gl_str.yes : gl_str.no);
            doCR();
            mpPrintf("Last <O>ld on New.... %s", gl_user.oldToo ?
				gl_str.on : gl_str.off);
            doCR();
            mpPrintf("<R>oom descriptions.. %s", gl_user.roomtell ?
				gl_str.on : gl_str.off);
            doCR();
            mpPrintf("<A>uto-next hall..... %s", logBuf.NEXTHALL ?
				gl_str.on : gl_str.off);
            doCR();

            doCR();
            mpPrintf("<Q> to save and quit.");
            doCR();
            prtMess = (BOOL) (!gl_user.expert);
        }
        if (new) {
            if (getYesNo("Is this OK", 1)) {
                quit = TRUE;
                continue;
            }
            new = FALSE;
        }
        outFlag = IMPERVIOUS;

        doCR();
        mPrintf("Change: ");

        oldEcho = echo;
        echo = NEITHER;
        c = iChar();
        echo = oldEcho;

        if (!(onConsole || gotCarrier()))
            return;

        switch (toupper(c)) {
            case 'W':
                mPrintf("Screen Width");
                doCR();
                gl_term.termWidth =
                (uchar) getNumber("Screen width", 10l, 255l,
					(long) gl_term.termWidth);
        /* kludge for carr-loss */
                if (gl_term.termWidth < 10)
                    gl_term.termWidth = cfg.width;
                break;

            case 'S':
                if (!logBuf.linesScreen) {
                    mPrintf("Pause on full screen");
                    doCR();
                    logBuf.linesScreen =
                    (uchar) getNumber("Lines per screen", 10L, 80L, 21L);
                } else {
                    mPrintf("Pause on full screen off");
                    doCR();
                    logBuf.linesScreen = 0;
                }
                break;

            case 'U':
                gl_term.termUpper = (BOOL) (!gl_term.termUpper);
                mPrintf("Uppercase only %s", gl_term.termUpper ?
					gl_str.on : gl_str.off);
                doCR();
                break;

            case 'L':
                gl_term.termLF = (BOOL) (!gl_term.termLF);
                mPrintf("Linefeeds %s", gl_term.termLF ?
					gl_str.on : gl_str.off);
                doCR();
                break;

            case 'T':
                gl_term.termTab = (BOOL) (!gl_term.termTab);
                mPrintf("Tabs %s", gl_term.termTab ?
					gl_str.on : gl_str.off);
                doCR();
                break;

            case 'N':
                if (!gl_term.termNulls) {
                    mPrintf("Nulls");
                    doCR();
                    gl_term.termNulls =
						(uchar) getNumber("number of Nulls", 0L, 255L, 5L);
                } else {
                    mPrintf("Nulls off");
                    doCR();
                    gl_term.termNulls = 0;
                }
                break;

            case 'E':
                gl_term.ansiOn = (BOOL) (!gl_term.ansiOn);
                mPrintf("Terminal Emulation %s", gl_term.ansiOn ?
					gl_str.on : gl_str.off);
                doCR();
                break;

            case 'G':
                gl_term.IBMOn = !gl_term.IBMOn;
                mPrintf("IBM Character Graphics %s", gl_term.IBMOn ?
					gl_str.on : gl_str.off);
                doCR();
                break;

            case 'H':
                gl_user.expert = (BOOL) (!gl_user.expert);
                mPrintf("Helpful Hints %s", !gl_user.expert ?
					gl_str.on : gl_str.off);
                doCR();
                break;

            case 'I':
                gl_user.unlisted = (BOOL) (!gl_user.unlisted);
                mPrintf("List in userlog %s", !gl_user.unlisted ?
					gl_str.yes : gl_str.no);
                doCR();
                break;

            case 'O':
                gl_user.oldToo = (BOOL) (!gl_user.oldToo);
                mPrintf("Last Old on New %s", gl_user.oldToo ?
					gl_str.on : gl_str.off);
                doCR();
                break;

            case 'R':
                gl_user.roomtell = (BOOL) (!gl_user.roomtell);
                mPrintf("Room descriptions %s", gl_user.roomtell ?
					gl_str.on : gl_str.off);
                doCR();
                break;

            case 'A':
                logBuf.NEXTHALL = (BOOL) (!logBuf.NEXTHALL);
                mPrintf("Auto-next hall %s", logBuf.NEXTHALL ?
					gl_str.on : gl_str.off);
                doCR();
                break;

            case 'Q':
                mPrintf("Save changes");
                doCR();
                quit = TRUE;
                break;

            case '\r':
            case '\n':
            case '?':
                mPrintf("Menu");
                doCR();
                prtMess = TRUE;
                break;

            default:
                mPrintf("%c ? for help", c);
                doCR();
                break;
        }

    } while (!quit);
}

/* -------------------------------------------------------------------- */
/*  userEdit()      Edit a user via menu                                */
/* -------------------------------------------------------------------- */
void userEdit(void)
{
    label who;

    getNormStr("who", who, NAMESIZE, ECHO);
	userEdit2(who);
}

void userEdit2(label who)
{
    BOOL prtMess = TRUE;
    BOOL quit = FALSE;
    int c;
    char string[200];
	char temp[80];
    char oldEcho;
    int logNo, ltabSlot, tsys;
    BOOL editSelf = FALSE;
	int reset = 0;
	
    logNo = findPerson(who, &lBuf);
    ltabSlot = personexists(who);

    if (!strlen(who) || logNo == ERROR) {
        mPrintf("No \'%s\' known. \n ", who);
		return;
    }
    /* make sure we use current info */
    if (strcmpi(who, logBuf.lbname) == SAMESTRING) {
        tsys = logBuf.lbflags.SYSOP;
        setlogconfig();     /* update current user */
        logBuf.lbflags.SYSOP = tsys;
        lBuf = logBuf;      /* use their online logbuffer */
        editSelf = TRUE;
    }
    doCR();

    do {
        if (prtMess) {
            doCR();
            outFlag = OUTOK;
            mpPrintf("<N>ame............... %s", lBuf.lbname);
            doCR();
            cPrintf("Initials............. %s\n", lBuf.lbin);
            cPrintf("Password............. %s\n", lBuf.lbpw);
            mpPrintf("<Z> (Title).......... %s", lBuf.title);
            doCR();     /* temp. */
            mpPrintf("s<U>rname............ %s", lBuf.surname);
            doCR();
            mpPrintf("    <L>ocked......... %s", lBuf.SURNAMLOK ?
				gl_str.yes : gl_str.no);
            doCR();
            mpPrintf("<S>ysop.............. %s", lBuf.lbflags.SYSOP
            ? gl_str.yes : gl_str.no);
            doCR();
            mpPrintf("<A>ide............... %s", lBuf.lbflags.AIDE
            ? gl_str.yes : gl_str.no);
            doCR();
            mpPrintf("n<O>de............... %s", lBuf.lbflags.NODE
				? gl_str.yes : gl_str.no);
            doCR();
            if (cfg.accounting) {
                mpPrintf("m<I>nutes............ ");

                if (lBuf.lbflags.NOACCOUNT)
                    mPrintf("N/A");
                else
                    mPrintf("%.0f", lBuf.credits);

                doCR();
            }
            mpPrintf("p<E>rmanent.......... %s", lBuf.lbflags.PERMANENT
				? gl_str.yes : gl_str.no);
            doCR();
            mpPrintf("ne<T>user............ %s", lBuf.lbflags.NETUSER
				? gl_str.yes : gl_str.no);
            doCR();
            mpPrintf("<P>roblem User....... %s", lBuf.lbflags.PROBLEM
				? gl_str.yes : gl_str.no);
            doCR();
            mpPrintf("No <M>ail............ %s", lBuf.lbflags.NOMAIL
				? gl_str.yes : gl_str.no);
            doCR();
            mpPrintf("<V>erified........... %s", !lBuf.VERIFIED ?
				gl_str.yes : gl_str.no);
            doCR();
            mpPrintf("<R>eset messages..... %s", reset ?
				gl_str.yes : gl_str.no);
            doCR();

            doCR();
            mpPrintf("<Q> to save and quit.");
            doCR();
            prtMess = (BOOL) (!gl_user.expert);
        }
        outFlag = IMPERVIOUS;

        doCR();
        mPrintf("Change: ");

        oldEcho = echo;
        echo = NEITHER;
        c = iChar();
        echo = oldEcho;

        if (!(onConsole || gotCarrier()))
            return;

        switch (toupper(c)) {
            case 'N':
                mPrintf("Name");
                doCR();
                strcpy(temp, lBuf.lbname);
                getString("new name", lBuf.lbname, NAMESIZE, FALSE, ECHO, temp);
                normalizeString(lBuf.lbname);
                if (!strlen(lBuf.lbname))
                    strcpy(lBuf.lbname, temp);
                break;

            case 'Z':
                mPrintf("Title");
                doCR();
                if (lBuf.lbflags.SYSOP && lBuf.SURNAMLOK && !editSelf) {
                    doCR();
                    mPrintf("User has locked their title!");
                    doCR();
                } else {
                    strcpy(temp, lBuf.title);
                    getString("new title", lBuf.title, NAMESIZE, FALSE,
						ECHO, temp);
                    normalizeString(lBuf.title);
                    if (!strlen(lBuf.title))
                        strcpy(lBuf.title, temp);
                }
                break;

            case 'U':
                mPrintf("sUrname");
                doCR();
                if (lBuf.lbflags.SYSOP && lBuf.SURNAMLOK && !editSelf) {
                    doCR();
                    mPrintf("User has locked their surname!");
                    doCR();
                } else {
                    strcpy(temp, lBuf.surname);
                    getString("new surname", lBuf.surname, NAMESIZE,
						FALSE, ECHO, temp);
                    normalizeString(lBuf.surname);
                    if (!strlen(lBuf.surname))
                        strcpy(lBuf.surname, temp);
                }
                break;

            case 'L':
                if (!(lBuf.lbflags.SYSOP && lBuf.SURNAMLOK && !editSelf)) {
                    lBuf.SURNAMLOK = (BOOL) (!lBuf.SURNAMLOK);
                }
                mPrintf("Surname Locked %s", lBuf.SURNAMLOK ?
					gl_str.yes : gl_str.no);
                doCR();
                if (lBuf.lbflags.SYSOP && lBuf.SURNAMLOK && !editSelf) {
                    doCR();
                    mPrintf("You can not change that!");
                    doCR();
                }
                break;

            case 'S':
                lBuf.lbflags.SYSOP = (BOOL) (!lBuf.lbflags.SYSOP);
                mPrintf("Sysop %s", lBuf.lbflags.SYSOP ?
					gl_str.yes : gl_str.no);
                doCR();
                break;

            case 'A':
                lBuf.lbflags.AIDE = (BOOL) (!lBuf.lbflags.AIDE);
                mPrintf("Aide %s", lBuf.lbflags.AIDE ?
					gl_str.yes : gl_str.no);
                doCR();
                break;

            case 'O':
                lBuf.lbflags.NODE = (BOOL) (!lBuf.lbflags.NODE);
                mPrintf("nOde %s", lBuf.lbflags.NODE ?
					gl_str.yes : gl_str.no);
                doCR();
                break;

            case 'I':
                mPrintf("mInutes");
                doCR();
                if (cfg.accounting) {
                    lBuf.lbflags.NOACCOUNT =
                    getYesNo("Disable user's accounting",
                    (BOOL) lBuf.lbflags.NOACCOUNT);

                    if (!lBuf.lbflags.NOACCOUNT) {
                        sprintf(temp, "%ss left", cfg.credit_name);
                        lBuf.credits = (float) getNumber(temp, 0L,
                        (long) cfg.maxbalance, (long) lBuf.credits);
                    }
                } else {
                    doCR();
                    mPrintf("Accounting turned off for system.");
                }
                break;

            case 'E':
                lBuf.lbflags.PERMANENT = (BOOL) (!lBuf.lbflags.PERMANENT);
                mPrintf("pErmanent %s", lBuf.lbflags.PERMANENT ?
					gl_str.yes : gl_str.no);
                doCR();
                break;

            case 'T':
                lBuf.lbflags.NETUSER = (BOOL) (!lBuf.lbflags.NETUSER);
                mPrintf("neTuser %s", lBuf.lbflags.NETUSER ?
					gl_str.yes : gl_str.no);
                doCR();
                break;

            case 'P':
                lBuf.lbflags.PROBLEM = (BOOL) (!lBuf.lbflags.PROBLEM);
                mPrintf("Problem user %s", lBuf.lbflags.PROBLEM ?
					gl_str.yes : gl_str.no);
                doCR();
                break;

            case 'M':
                lBuf.lbflags.NOMAIL = (BOOL) (!lBuf.lbflags.NOMAIL);
                mPrintf("no Mail %s", lBuf.lbflags.NOMAIL ?
					gl_str.yes : gl_str.no);
                doCR();
                break;

            case 'V':
                lBuf.VERIFIED = (BOOL) (!lBuf.VERIFIED);
                mPrintf("Verified %s", !lBuf.VERIFIED ?
					gl_str.yes : gl_str.no);
                doCR();
                break;

            case 'Q':
                mPrintf("Quit");
                doCR();
                quit = TRUE;
                break;

			case 'R':
				if(getYesNo("Reset all messages to new",0)) {
					int i;

					reset = 1;
					mPrintf("Resetting all messages to new");
					doCR();
					/* reset room count */
					for(i=0; i<MAXROOMS; ++i)
						lBuf.lbroom[i].lvisit = 0;

					/* reset visit counts */
					for(i=0; i<MAXVISIT; ++i)
						lBuf.lbvisit[i] = 0;
				}
				break;
            case '\r':
            case '\n':
            case '?':
                mPrintf("Menu");
                doCR();
                prtMess = TRUE;
                break;

            default:
                mPrintf("%c ? for help", c);
                doCR();
                break;
        }

    } while (!quit);

    if (!getYesNo("Save changes", 0))
        return;

    /* trap it */
    sprintf(string, "%s has:", who);
    if (lBuf.lbflags.SYSOP)
        strcat(string, " Sysop Priv:");
    if (lBuf.lbflags.AIDE)
        strcat(string, " Aide Priv:");
    if (lBuf.lbflags.NODE)
        strcat(string, " Node status:");
    if (cfg.accounting)
        if (lBuf.lbflags.NOACCOUNT)
            strcat(string, " No Accounting:");
        else {
            sprintf(temp, " %.0f %s:", lBuf.credits);
            strcat(string, temp);
        }

    if (lBuf.lbflags.PERMANENT)
        strcat(string, " Permanent Log Entry:");
    if (lBuf.lbflags.NETUSER)
        strcat(string, " Network User:");
    if (lBuf.lbflags.PROBLEM)
        strcat(string, " Problem User:");
    if (lBuf.lbflags.NOMAIL)
        strcat(string, " No Mail:");
    if (lBuf.VERIFIED)
        strcat(string, " Un-Verified:");

/*  trap(temp, T_SYSOP); -- for a long time, this incredible bug was here. */
    trap(string, T_SYSOP);

    /* see if it is us: */
    if (loggedIn && editSelf) {
    /* move it back */
        logBuf = lBuf;

    /* make our environment match */
        setsysconfig();
    }
    putLog(&lBuf, logNo);
    logTab[ltabSlot].permanent = (BOOL) lBuf.lbflags.PERMANENT;
    logTab[ltabSlot].ltnmhash = hash(lBuf.lbname);
}

/* EOF */
