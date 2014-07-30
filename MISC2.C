/* -------------------------------------------------------------------- */
/*  MISC2.C                    Citadel                                   */
/* -------------------------------------------------------------------- */
/*                        Overlayed misc stuff                          */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Includes                                                            */
/* -------------------------------------------------------------------- */
#define MISC2
#include <string.h>
#include <time.h>
#include "ctdl.h"

#ifndef ATARI_ST
#include <alloc.h>
#include <bios.h>
#include <conio.h>
#include <dos.h>
#endif

#include "proto.h"
#include "global.h"



#define readnode()	(getnode(logBuf.lbname))


/* --------------------------------------------------------------------
 *                              Contents
 * --------------------------------------------------------------------
 *
 *  systat()        System status
 *  chat()          This is the chat mode
 *  ringSysop()     ring the sysop
 *  ringSystemREQ() signals a system request for 2 minutes.
 *  dial_out()      dial out to other boards
 *  logo()          prints out logo screen and quote at start-up
 * -------------------------------------------------------------------- */


/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/MISC2.C_V  $
 * 
 *    Rev 1.47   01 Nov 1991 11:20:30   FJM
 * Added gl_ structures
 *
 *    Rev 1.46   15 Jun 1991  8:42:10   FJM
 *
 *    Rev 1.45   15 Jun 1991  7:47:16   FJM
 * Created stripANSI()
 *
 *    Rev 1.44   06 Jun 1991  9:19:24   FJM
 *    Rev 1.43   27 May 1991 11:43:06   FJM
 *    Rev 1.42   22 May 1991  2:17:32   FJM
 *    Rev 1.39   17 Apr 1991 12:55:38   FJM
 *    Rev 1.22   11 Jan 1991 12:43:26   FJM
 *    Rev 1.18   06 Jan 1991 12:45:26   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.17   27 Dec 1990 20:15:52   FJM
 * Changes for porting.
 *
 *    Rev 1.14   22 Dec 1990 13:37:34   FJM
 *    Rev 1.8   09 Dec 1990 15:23:40   FJM
 *    Rev 1.7   07 Dec 1990 23:10:28   FJM
 *    Rev 1.5   24 Nov 1990  3:07:38   FJM
 * Changes for shell/door mode.
 *
 *    Rev 1.4   23 Nov 1990 13:19:46   FJM
 * Changed outFlag to Impervious in chat & dialout.
 *
 *    Rev 1.2   17 Nov 1990 16:11:52   FJM
 * Added version control log header
 * -------------------------------------------------------------------- */

/* --------------------------------------------------------------------
 *  History:
 *
 *  02/08/89    (PAT)   History Re-Started
 *                      InitAideMess and SaveAideMess added
 *  05/26/89    (PAT)   Many of the functions move to other modules
 *  03/07/90    {zm}    Add 'softname' as the name of the software.
 *  03/08/90    {zm}    Integrate FJM's fancy bell, make it an #ifdef.
 *
 *  03/12/90    FJM             Added IBM Grahics character translation
 *  03/18/90    FJM             Fixes for entry bug
 *  03/19/90    FJM     Linted & partial cleanup
 *  05/18/90    FJM     Added chat.blb for successful chat requests
 *  05/20/90    FJM             Made chat.blb cycle.  Added cyclic NOSOP.BLB
 *      06/06/90        FJM             Changed strftime to cit_strftime
 *
 * -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  External data                                                       */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Static Data definitions                                             */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  systat()        System status (.RS)                                 */
/* -------------------------------------------------------------------- */
void systat(void)
{
    unsigned int i;
    unsigned long average, work;
    char summary[256];
    char dtstr[80];
    int public = 0, private = 0, group = 0, problem = 0, moderated = 0;

    outFlag = OUTOK;

    /* even the name of the BBS program is not cast in concrete! */
    mPrintf("%s @ %s, %s ", softname, cfg.nodeTitle, cfg.nodeRegion);
    doCR();
    mPrintf(" Running Version %s Compiled on %s at %s",
    version, compdate, comptime);
    doCR();

    cit_strftime(dtstr, 79, cfg.vdatestamp, 0l);
    mPrintf(" %s", dtstr);
    doCR();
    mPrintf(" Up time: ");
    diffstamp(uptimestamp);

    if (gotCarrier()) {
        doCR();
        mPrintf(" Connect time: ");
        diffstamp(conntimestamp);
    }
    if (loggedIn) {
        doCR();
        mPrintf(" Logon time: ");
        diffstamp(logtimestamp);
    }
    doCR();
    mPrintf(" Maximum of %d log entries,", cfg.MAXLOGTAB);

    mPrintf(" Call number %lu", cfg.callno);

    doCR();
    mPrintf(" %ld messages, Last is %lu",
    cfg.newest - cfg.oldest + 1, cfg.newest);

    for (i = 0; i < sizetable(); ++i) {
        if (msgTab[i].mtmsgflags.PROBLEM)
            problem++;
        if (msgTab[i].mtmsgflags.MODERATED)
            moderated++;
        if (msgTab[i].mtmsgflags.LIMITED)
            group++;
        else if (msgTab[i].mtmsgflags.MAIL)
            private++;
        else
            public++;
    }

    doCR();

    mPrintf(" There are a total of ");

    if ((cfg.mtoldest - cfg.oldest) > 0)
        mPrintf("%d messages missing, ", (int) (cfg.mtoldest - cfg.oldest));

    mPrintf
    ("%d public messages, %d private messages, %d moderated messages, ",
    public, private, moderated);

    if (!gl_user.aide)
        mPrintf("and ");

    mPrintf("%d group only messages", group);

    if (gl_user.aide)
        mPrintf(", and %d problem user messages", problem);

    doCR();
    mPrintf(" %dK message space, ", cfg.messagek);

    if (cfg.oldest > 1)
        work = (long) ((long) cfg.messagek * 1024l);
    else
        work = cfg.catLoc;

    if (cfg.oldest > 1)
        average = (work) / (cfg.newest - cfg.oldest + 1);
    else
        average = (work) / (cfg.newest);

    mPrintf(" %ld bytes average message length", average);

    if (gl_user.sysop) {
        doCR();
#ifndef ATARI_ST
        mPrintf(" DOS version %d.%d,", _osmajor, _osminor);
        mPrintf(" %uK system memory,", biosmemory());
        if (farcoreleft() < 180000L)
            termCap(TERM_BOLD);
        mPrintf(" %lu ", farcoreleft());
        termCap(TERM_NORMAL);
        mPrintf("bytes free system memory.");
#endif
    }
    doCR();
    mPrintf(" Hallway %s", hallBuf->hall[thisHall].hallname);

    if (hallBuf->hall[thisHall].owned) {
        mPrintf(", owned by group %s",
        grpBuf.group[hallBuf->hall[thisHall].grpno].groupname);
    }
    doCR();
    formatSummary(summary);

    mPrintf("%s", summary);

    doCR();
    mPrintf(" %lu characters transmitted, %lu characters received",
    transmitted, received);

    if (cfg.accounting && !logBuf.lbflags.NOACCOUNT) {
        doCR();

        if (!specialTime) {
            mPrintf(" You have %.0f %s%s left today.",
            logBuf.credits,
            cfg.credit_name,
            ((int) logBuf.credits == 1) ? "" : "s");
        } else {
            mPrintf(" You have unlimited time.");
        }
    }
    doCR();
    mPrintf(" %s", copyright[0]);
    doCR();
    mPrintf(" %s", copyright[1]);
    doCR();
}

/* -------------------------------------------------------------------- */
/*  ringSystemREQ() signals a system request for 2 minutes.             */
/* -------------------------------------------------------------------- */
void ringSystemREQ(void)
{
    unsigned char row, col;
    char i;
    char answered = FALSE;
    char ringlimit = 120;

    doccr();
    doccr();
    readpos(&row, &col);
    (*stringattr) (row, " * System Available * ", cfg.wattr | 128);
    /* update25();	*/
	do_idle(0);
    doccr();

    answered = FALSE;
    for (i = 0; (i < ringlimit) && !answered; i++) {
        putch(7 /* BELL */ );
        delay(800);
        if (KBReady())
            answered = TRUE;
    }

    if (!KBReady() && i >= ringlimit)
        Initport();

    /* update25();	*/
	do_idle(0);
}

/* -------------------------------------------------------------------- */
/*  dial_out()      dial out to other boards                            */
/* -------------------------------------------------------------------- */
void dial_out(void)
{
    unsigned int con, mod;
    char fname[61];

    fname[0] = 60;

    outFlag = IMPERVIOUS;
    mPrintf("  Now in dial out mode, Control-Q to exit.\n\n ");
    /* outFlag = OUTOK;     - changed per Peter's suggestion    */

    Hangup();

    disabled = FALSE;

    baud(cfg.initbaud);

    /* update25();	*/
	do_idle(0);

    outstring(cfg.dialsetup);
    outstring("\r");

    delay(1000);

    callout = TRUE;

    do {
        con = 0;
        mod = 0;

        if (KBReady()) {
            con = getch();
            getkey = 0;
            if (debug)
                oChar(con);
            if (con != 17 && con != 2 && con != 5
                && con != 14 && con != 4 && con != 21)
                outMod(con);
            else
            switch (con) {
                case 2:
                    mPrintf("New baud rate [0-4]: ");
                    con = (char) getch();
                    doccr();
                    if (con > ('0' - 1) && con < '5')
                        baud(con - 48);
                    /* update25();	*/
					do_idle(0);
                    break;
                case 5:
                    if (gl_user.sysop || !ConLock)
                        shellescape(FALSE);
                    break;
                case 14:
                    readnode();
                    master();
                    slave();
                    cleanup();
                    break;
            /* WC isn't needed on DOS machines which use DSZ or equiv. */
#ifndef __MSDOS__
                case 4:
                    doccr();
                    cPrintf("Filename: ");
                    cgets(fname);
                    if (!fname[2])
                        break;
                    rxfile(fname + 2);
                    fname[2] = '\0';
                    break;
                case 21:
                    doccr();
                    cPrintf("Filename: ");
                    cgets(fname);
                    if (!fname[2])
                        break;
                    trans(fname + 2);
                    fname[2] = '\0';
                    break;
#endif
                default:
                    break;
            }
        }
        if (MIReady()) {
            mod = getMod();

            if (debug) {
                mod = (mod & 0x7F);
                if (mod < 128)
                    mod = filt_in[mod];
            }
            if (mod == '\n')
                doccr();
            else if (mod != '\r')
                oChar(mod);
        }
    } while (con != 17 /* ctrl-q */ );

    callout = FALSE;

    Initport();

    doCR();
    outFlag = OUTOK;
}

/*----------------------------------------------------------------------*/
/*      logo()  prints out system logo at startup                       */
/*----------------------------------------------------------------------*/
#define SHOW_CRC   0

void logo()
{
    int i;

    cls();

    for (i = 0; welcome[i]; i++)
        if (welcomecrcs[i] != stringcrc(welcome[i])) {
#if SHOW_CRC
            cCPrintf("Welcome CRC %d should be 0x%04x", i, stringcrc(welcome[i]));
            getch();
#else
            crashout("This program has been tampered with.");
#endif
        }
    for (i = 0; copyright[i]; i++)
        if (copycrcs[i] != stringcrc(copyright[i])) {
#if SHOW_CRC
            cCPrintf("Copyright CRC %d should be 0x%04x", i, stringcrc(copyright[i]));
            getch();
#else
            crashout("Someone tampered with the Copyright message!");
#endif
        }
    position(5, 0);     /* for FredCit */

    cCPrintf("%s v%s", softname, version);  /* it's in VERSION.C */
    doccr();
#ifdef ALPHA_TEST
    cCPrintf("Alpha Test Site");
    doccr();
#endif
#ifdef BETA_TEST
    cCPrintf("Beta Test Site");
    doccr();
#endif
    cCPrintf("Host is IBM, (TurboC)");

    position(9, 0);     /* for FredCit */

    for (i = 0; welcome[i]; i++) {
        cCPrintf(welcome[i]);
        doccr();
    }

    position(19, 0);

    for (i = 0; copyright[i]; i++) {
        cCPrintf(copyright[i]);
        doccr();
    }

    doccr();
}

#pragma warn -pia
void stripANSI(char *s)
{
	char *p;
	p=s;
	while(p = strchr(p,1)) {
		if (*(p+1))
			memcpy(p,p+2,strlen(p+2)+1); /* remove ^A and next char */
		else
			memcpy(p,p+1,strlen(p+1)+1); /* just remove ^A */
	}
}
/* EOF */
