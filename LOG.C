/* -------------------------------------------------------------------- */
/*  LOG.C                     Citadel                                   */
/* -------------------------------------------------------------------- */
/*                           Local log code                             */
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
/*  findPerson()    loads log record for named person.                  */
/*                  RETURNS: ERROR if not found, else log record #      */
/*  personexists()  returns slot# of named person else ERROR            */
/*  setdefaultconfig()  this sets the global configuration variables    */
/*  setlogconfig()  this sets the configuration in current logBuf equal */
/*                  to the global configuration variables               */
/*  setsysconfig()  this sets the global configuration variables equal  */
/*                  to the the ones in logBuf                           */
/*  showconfig()    displays user configuration                         */
/*  slideLTab()     crunches up slot, then frees slot at beginning,     */
/*                  it then copies information to first slot            */
/*  storeLog()      stores the current log record.                      */
/*  displaypw()     displays callers name, initials & pw                */
/*  normalizepw()   This breaks down inits;pw into separate strings     */
/*  pwexists()      returns TRUE if password exists in logtable         */
/* -------------------------------------------------------------------- */


/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/LOG.C_V  $
 * 
 *    Rev 1.40   01 Nov 1991 11:20:18   FJM
 * Added gl_ structures
 *
 *    Rev 1.39   19 Apr 1991 23:39:02   FJM
 * No change.
 *
 *    Rev 1.38   19 Apr 1991 23:26:58   FJM
 * No change.
 *
 *    Rev 1.37   17 Apr 1991 12:55:28   FJM
 *
 *    Rev 1.35   04 Apr 1991 14:13:22   FJM
 *    Rev 1.31   10 Feb 1991 17:33:32   FJM
 *    Rev 1.29   05 Feb 1991 14:31:40   FJM
 *    Rev 1.27   19 Jan 1991 14:15:48   FJM
 * Cleaned up login.
 *
 *    Rev 1.16   06 Jan 1991 12:45:50   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.15   27 Dec 1990 20:15:56   FJM
 * Changes for porting.
 *
 *    Rev 1.12   22 Dec 1990 13:37:40   FJM
 *    Rev 1.4   23 Nov 1990 13:25:02   FJM
 * Header change
 *
 *    Rev 1.2   17 Nov 1990 16:11:42   FJM
 * Added version control log header
 * -------------------------------------------------------------------- */

/* --------------------------------------------------------------------
 *  HISTORY:
 *
 *  06/14/89  (PAT)  Created from LOG.C to move some of the system
 *                   out of memory. Also cleaned up moved code to
 *                   -W3, ext.
 *  03/15/90  {zm}   Add [title] name [surname] everywhere.
 *  06/16/90  FJM    Fixes to allow entry of 30 char nym & initials.
 *  06/16/90  FJM    Made IBM Graphics characters a seperate option.
 * -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Static Data                                                         */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  findPerson()    loads log record for named person.                  */
/*                  RETURNS: ERROR if not found, else log record #      */
/* -------------------------------------------------------------------- */
int findPerson(char *name, struct logBuffer * l_buf)
{
    int slot, logno;

    slot = personexists(name);

    if (slot == ERROR)
        return (ERROR);

    getLog(l_buf, logno = logTab[slot].ltlogSlot);

    return (logno);
}

/* -------------------------------------------------------------------- */
/*  personexists()  returns slot# of named person else ERROR            */
/* -------------------------------------------------------------------- */
int personexists(char *name)
{
    int i, namehash;
    struct logBuffer logRead;

    namehash = hash(name);

    /* check to see if name is in log table */

    for (i = 0; i < cfg.MAXLOGTAB; i++) {
        if (namehash == logTab[i].ltnmhash) {
            getLog(&logRead, logTab[i].ltlogSlot);

            if (strcmpi(name, logRead.lbname) == SAMESTRING)
                return (i);
        }
    }

    return (ERROR);
}

/* -------------------------------------------------------------------- */
/*  setdefaultconfig()  this sets the global configuration variables    */
/* -------------------------------------------------------------------- */
/*very strange, sets odd locations.  i.e. some, but not all logBuf vars	*/
/* -------------------------------------------------------------------- */
void setdefaultconfig(void)
{
    prevChar = ' ';
	
    logBuf.linesScreen = 0;
    gl_term.termWidth = cfg.width;
	/* logBuf.lbwidth	*/
    gl_term.termNulls = cfg.nulls;
	/* logBuf.lbnulls	*/
	/* logBuf.lbflags.L_INUSE = 1; */
    gl_term.termUpper = (BOOL) cfg.uppercase;
    /* logBuf.lbflags.UCMASK = gl_term.termUpper; */
    gl_term.termLF = (BOOL) cfg.linefeeds;
    /* logBuf.lbflags.LFMASK = gl_term.termLF;	*/
    gl_user.expert = FALSE;
    /* logBuf.lbflags.EXPERT = gl_user.expert;	*/
    gl_user.aide = FALSE;
    /* logBuf.lbflags.AIDE = gl_user.aide;	*/
    gl_term.termTab = (BOOL) cfg.tabs;
    /* logBuf.lbflags.TABS = gl_term.termTab;	*/
    gl_user.oldToo = TRUE;     /* later a cfg.lastold */
    /* logBuf.lbflags.OLDTOO = gl_user.oldToo;	*/
    gl_user.twit = cfg.user[D_PROBLEM];
    /* logBuf.lbflags.PROBLEM = gl_user.twit;	*/
    gl_user.unlisted = FALSE;
    /* logBuf.lbflags.UNLISTED = gl_user.unlisted;	*/
	/* logBuf.lbflags.PERMANENT = ??? NB: affects lTable	*/
    gl_user.sysop = FALSE;
    /* logBuf.lbflags.SYSOP = gl_user.sysop;	*/
	/* logBuf.lbflags.NODE = ???	*/
	/* logBuf.lbflags.NETUSER = ???	*/
	/* logBuf.lbflags.NOACCOUNT = ???	*/
	/* logBuf.lbflags.NOMAIL = ???	*/
    gl_user.roomtell = TRUE;
    /* logBuf.lbflags.ROOMTELL = gl_user.roomtell;	*/
    logBuf.DUNGEONED = FALSE;
    logBuf.MSGAIDE = FALSE;
	/* logBuf.DUNGEONED	*/
	/* logBuf.MSGAIDE	*/
    logBuf.FORtOnODE = FALSE;
    logBuf.NEXTHALL = FALSE;
	/* logBuf.OTHERNAV	*/
	/* logBuf.VERIFIED	*/
	/* logBuf.SURNAMLOK	*/
    logBuf.IBMGRAPH = gl_term.IBMOn;
    strcpy(logBuf.tty, "TTY");
}

/* -------------------------------------------------------------------- */
/*  setlogconfig()  this sets the configuration in current logBuf equal */
/*                  to the global configuration variables               */
/* -------------------------------------------------------------------- */
void setlogconfig(void)
{
	/* logBuf.lbname NB: affects lTable	*/
	/* logBuf.lbin NB: affects lTable	*/
	/* logBuf.lbpw NB: affects lTable	*/
	/* logBuf.forward	*/
	/* logBuf.surname	*/
	/* logBuf.title	*/
	/* logBuf.hallhash	*/
    logBuf.lbwidth = gl_term.termWidth;
    logBuf.lbnulls = gl_term.termNulls;
	/* logBuf.calltime	*/
	/* logBuf.callno	*/
	/* logBuf.credits	*/
	/* logBuf.groups	*/
	/* logBuf.lbroomdata	*/
	/* logBuf.lbvisit	*/
	/* logBuf.lastRoom	*/
	/* logBuf.lastHall	*/
    /* logBuf.linesScreen = 23; */
    if (gl_term.ansiOn)
        strcpy(logBuf.tty, "ANSI-BBS");
    else
        strcpy(logBuf.tty, "TTY");
		
	/* logBuf.lbflags.L_INUSE = 1; */
    logBuf.lbflags.UCMASK = gl_term.termUpper;
    logBuf.lbflags.LFMASK = gl_term.termLF;
    logBuf.lbflags.EXPERT = gl_user.expert;
    logBuf.lbflags.AIDE = gl_user.aide;
    logBuf.lbflags.TABS = gl_term.termTab;
    logBuf.lbflags.OLDTOO = gl_user.oldToo;
    logBuf.lbflags.PROBLEM = gl_user.twit;
    logBuf.lbflags.UNLISTED = gl_user.unlisted;
	/* logBuf.lbflags.PERMANENT = ??? NB: affects lTable	*/
    logBuf.lbflags.SYSOP = gl_user.sysop;
	/* logBuf.lbflags.NODE = ???	*/
	/* logBuf.lbflags.NETUSER = ???	*/
	/* logBuf.lbflags.NOACCOUNT = ???	*/
	/* logBuf.lbflags.NOMAIL = ???	*/
    logBuf.lbflags.ROOMTELL = gl_user.roomtell;
	/* logBuf.DUNGEONED	*/
	/* logBuf.MSGAIDE	*/
	/* logBuf.FORtOnODE	*/
	/* logBuf.NEXTHALL	*/
	/* logBuf.OTHERNAV	*/
	/* logBuf.VERIFIED	*/
	/* logBuf.SURNAMLOK	*/
    logBuf.IBMGRAPH = gl_term.IBMOn;
}

/* -------------------------------------------------------------------- */
/*  setsysconfig()  this sets the global configuration variables equal  */
/*                  to the the ones in logBuf                           */
/* -------------------------------------------------------------------- */
/* NB: Does not include all values in logBuf; error?                    */
/* -------------------------------------------------------------------- */
void setsysconfig(void)
{
    gl_term.termWidth = logBuf.lbwidth;
    gl_term.termNulls = logBuf.lbnulls;
    gl_term.termLF = (BOOL) logBuf.lbflags.LFMASK;
    gl_term.termUpper = (BOOL) logBuf.lbflags.UCMASK;
    gl_user.expert = (BOOL) logBuf.lbflags.EXPERT;
    gl_user.aide = (BOOL) logBuf.lbflags.AIDE;
    gl_user.sysop = (BOOL) logBuf.lbflags.SYSOP;
    gl_term.termTab = (BOOL) logBuf.lbflags.TABS;
    gl_user.oldToo = (BOOL) logBuf.lbflags.OLDTOO;
    gl_user.twit = (BOOL) logBuf.lbflags.PROBLEM;
    gl_user.unlisted = (BOOL) logBuf.lbflags.UNLISTED;
    gl_user.roomtell = (BOOL) logBuf.lbflags.ROOMTELL;
    gl_term.IBMOn = (BOOL) logBuf.IBMGRAPH;
    gl_term.ansiOn = (BOOL) (strcmpi(logBuf.tty, "ANSI-BBS") == SAMESTRING);
}

/* -------------------------------------------------------------------- */
/*  showconfig()    displays user configuration                         */
/* -------------------------------------------------------------------- */
void showconfig(struct logBuffer * l_buf)
{
    int i;

    if (loggedIn) {
        setlogconfig();

        if (cfg.surnames && l_buf->title[0]) {
            mPrintf("\n User [%s] %s", l_buf->title, l_buf->lbname);
        } else {
            mPrintf("\n User ");
			mtPrintf(TERM_BOLD,"%s", l_buf->lbname);
        }
        if (cfg.surnames && l_buf->surname[0]) {
            mPrintf(" [%s]", l_buf->surname);
        }
        if (
			l_buf->lbflags.UNLISTED ||
			/* l_buf->lbflags.PERMANENT */
            l_buf->lbflags.SYSOP ||
            l_buf->lbflags.NODE ||
            l_buf->lbflags.NETUSER ||
			/* l_buf->lbflags.NOACCOUNT */
			/* l_buf->lbflags.NOMAIL	*/
			/* l_buf->lbflags.ROOMTELL	*/
            l_buf->DUNGEONED ||
			l_buf->MSGAIDE ||
            l_buf->lbflags.AIDE
			/* l_buf->FORtOnODE	*/
			/* l_buf->NEXTHALL	*/
			/* l_buf->OTHERNAV	*/
			/* l_buf->VERIFIED	*/
			/* l_buf->SURNAMLOK	*/
			/* l_buf->IBMGRAPH	*/
			) {
            doCR();
			/* show Sysop, Aide first */
            if (l_buf->lbflags.SYSOP)
                mPrintf(" Sysop");
            if (l_buf->lbflags.AIDE)
                mPrintf(" Aide");
			
            if (l_buf->lbflags.UNLISTED)
                mPrintf(" Unlisted");
			/* l_buf->lbflags.PERMANENT */
            if (l_buf->lbflags.NODE)
                mPrintf(" Node");
            if (l_buf->lbflags.NETUSER)
                mPrintf(" Netuser");
			/* l_buf->lbflags.NOACCOUNT */
			/* l_buf->lbflags.NOMAIL	*/
			/* l_buf->lbflags.ROOMTELL	*/
            if (l_buf->MSGAIDE)
                mPrintf(" Moderator");
			/* l_buf->FORtOnODE */
			/* l_buf->NEXTHALL */
            if (l_buf->DUNGEONED)
                mPrintf(" Dungeoned");
        }
		/* NB: still need to check the remainder of this */
        if (l_buf->forward[0]) {
            doCR();
            mPrintf(" Private mail forwarded to ");
			/* l_buf->FORtOnODE */
            if (personexists(l_buf->forward) != ERROR)
                mPrintf("%s", l_buf->forward);
        }
        if (l_buf->hallhash) {
            doCR();
            mPrintf(" Default hallway: ");

            for (i = 1; i < MAXHALLS; ++i) {
                if (hash(hallBuf->hall[i].hallname) == l_buf->hallhash) {
                    if (groupseeshall(i))
                        mPrintf("%s", hallBuf->hall[i].hallname);
                }
            }
        }
        doCR();
        mPrintf(" Groups: ");

        for (i = 0; i < MAXGROUPS; ++i) {
            if (grpBuf.group[i].g_inuse
                && (l_buf->groups[i] == grpBuf.group[i].groupgen))
                mPrintf("%s ", grpBuf.group[i].groupname);
        }
    }
    if (cfg.accounting && !l_buf->lbflags.NOACCOUNT && loggedIn) {
        doCR();
        mPrintf(" Number of %s%s remaining %.0f",
        cfg.credit_name, ((int) l_buf->credits == 1) ? "" : "s",
        l_buf->credits);
    }
    mPrintf("\n Width %d, ", l_buf->lbwidth);

    if (l_buf->lbflags.UCMASK)
        mPrintf("UPPERCASE ONLY, ");

    if (!l_buf->lbflags.LFMASK)
        mPrintf("%s ",gl_str.no);

    mPrintf("Linefeeds, ");

    mPrintf("%d nulls, ", l_buf->lbnulls);

    if (!l_buf->lbflags.TABS)
        mPrintf("%s ",gl_str.no);

    mPrintf("Tabs");

    if (!l_buf->lbflags.OLDTOO)
        mPrintf("\n Do not print");
    else
        mPrintf("\n Print");
    mPrintf(" last Old message on N>ew Message request.");

    if (loggedIn)
        mPrintf("\n Terminal type: %s", l_buf->tty);

    if (l_buf->NEXTHALL)
        mPrintf("\n Auto-next hall on.");

    if (cfg.roomtell && loggedIn) {
        if (!l_buf->lbflags.ROOMTELL)
            mPrintf("\n Do not display");
        else
            mPrintf("\n Display");
        mPrintf(" room descriptions.");
    }
    doCR();
}

/* -------------------------------------------------------------------- */
/*  slideLTab()     crunches up slot, then frees slot at beginning,     */
/*                  it then copies information to first slot            */
/* -------------------------------------------------------------------- */
void slideLTab(int slot)
{               /* slot is current tabslot being moved */
    int ourSlot, i;

    people = slot;      /* number of people since last call */

    if (!slot)
        return;

    ourSlot = logTab[slot].ltlogSlot;

    /* Gee, this works.. */
    for (i = slot; i > 0; i--)
        logTab[i] = logTab[i - 1];

/*
    hmemcpy((char HUGE *)(&logTab[1]), (char HUGE *)(&logTab[0]),
       (long)(sizeof(*logTab) * slot));
 */

    thisSlot = 0;

    /* copy info to beginning of table */
    logTab[0].ltpwhash = hash(logBuf.lbpw);
    logTab[0].ltinhash = hash(logBuf.lbin);
    logTab[0].ltnmhash = hash(logBuf.lbname);
    logTab[0].ltlogSlot = ourSlot;
    logTab[0].ltcallno = logBuf.callno;
    logTab[0].permanent = (BOOL) logBuf.lbflags.PERMANENT;
}


/* -------------------------------------------------------------------- */
/*  storeLog()      stores the current log record.                      */
/* -------------------------------------------------------------------- */
void storeLog(void)
{
    /* make log configuration equal to our environment */
    setlogconfig();

    putLog(&logBuf, thisLog);
}

/* -------------------------------------------------------------------- */
/*  displaypw()     displays callers name, initials & pw                */
/* -------------------------------------------------------------------- */
void displaypw(char *name, char *in, char *pw)
{
    mPrintf("\n nm: %s", name);
    mPrintf("\n in: ");
    echo = CALLER;
    mPrintf("%s", in);
    echo = BOTH;
    mPrintf("\n pw: ");
    echo = CALLER;
    mPrintf("%s", pw);
    echo = BOTH;
    doCR();
}


/* -------------------------------------------------------------------- */
/*  normalizepw()   This breaks down inits;pw into separate strings     */
/* -------------------------------------------------------------------- */
void normalizepw(char *InitPw, char *Initials, char *passWord, char *semi)
{
    char *p;

	p = semi;
	*p = '\0';
	strncpy(Initials,InitPw,NAMESIZE);
	Initials[NAMESIZE]='\0';
	++p;
	strncpy(passWord,p,NAMESIZE);
	passWord[NAMESIZE]='\0';
	normalizeString(Initials);
	normalizeString(passWord);
}

/* -------------------------------------------------------------------- */
/*  pwexists()      returns TRUE if password exists in logtable         */
/* -------------------------------------------------------------------- */
int pwexists(char *pw)
{
    int i, pwhash;

    pwhash = hash(pw);

    for (i = 0; i < cfg.MAXLOGTAB; i++) {
        if (pwhash == logTab[i].ltpwhash)
            return (i);
    }
    return (ERROR);
}

/* EOF */
