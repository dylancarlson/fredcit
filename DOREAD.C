/************************************************************************
 * $Header:   D:/VCS/FCIT/DOREAD.C_V   1.32   01 Nov 1991 11:19:56   FJM  $
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
 *      doRead()                handles R(ead)          command
 *
 ************************************************************************/


/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/DOREAD.C_V  $
 * 
 *    Rev 1.32   01 Nov 1991 11:19:56   FJM
 * Added gl_ structures
 *
 *    Rev 1.31   21 Sep 1991 10:18:42   FJM
 * FredCit release
 *
 *    Rev 1.30   06 Jun 1991  9:18:46   FJM
 *    Rev 1.29   27 May 1991 11:42:20   FJM
 *    Rev 1.26   17 Apr 1991 12:55:10   FJM
 *    Rev 1.25   10 Apr 1991  8:44:18   FJM
 * Fixed (Node) bug for unlisted nodes.
 *
 *    Rev 1.17   28 Jan 1991 13:14:14   FJM
 *    Rev 1.12   18 Jan 1991 16:53:52   FJM
 *    Rev 1.4   06 Jan 1991 12:44:36   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.3   27 Dec 1990 20:17:02   FJM
 * Changes for porting.
 *
 *    Rev 1.0   22 Dec 1990 13:40:24   FJM
 * Initial revision.
 *
 * -------------------------------------------------------------------- */


/************************************************************************/
/*      doRead() handles R(ead) command                                 */
/************************************************************************/
void doRead(moreYet, first)
char moreYet;           /* TRUE to accept following parameters */
char first;         /* first parameter if TRUE             */
{
    char abort, done, letter;
    char whichMess, revOrder, verbose;

    if (moreYet)
        first = '\0';

    mPrintf("\bRead ");

    abort = FALSE;
    revOrder = FALSE;
    verbose = FALSE;
    whichMess = NEWoNLY;
    mf.mfPub = FALSE;
    mf.mfMai = FALSE;
    mf.mfLim = FALSE;
    mf.mfUser[0] = FALSE;
    mf.mfGroup[0] = FALSE;

    if (!loggedIn && !cfg.unlogReadOk) {
        mPrintf("\n --Must log in to read.\n ");
        return;
    }
    if (first)
        oChar(first);

    do {
        outFlag = IMPERVIOUS;
        done = TRUE;

        letter = (char) (toupper(first ? (int) first : (int) iChar()));

    /* handle uponly flag! */
        if (roomTab[thisRoom].rtflags.UPONLY && !groupseesroom(thisRoom)
        && ((letter == 'T') || (letter == 'W'))) {
            mPrintf("\b\n\n  --Room is upload only.\n ");
            break;
        }
        switch (letter) {
            case '\n':
            case '\r':
                moreYet = FALSE;
                break;
            case 'B':
                mPrintf("\bBy-User ");
				if (!roomBuf.rbflags.ANONYMOUS || gl_user.sysop) {
					mf.mfUser[0] = TRUE;
					done = FALSE;
				} else {
					mPrintf("\n Not allowed in anonymous rooms.\n");
					abort = TRUE;
				}
                break;
            case 'C':
                mPrintf("\bConfiguration ");
                showconfig(&logBuf);
                abort = TRUE;
                break;
            case 'D':
                mPrintf("\bDirectory");
                if (!roomBuf.rbflags.MSDOSDIR) {
                    if (gl_user.expert)
                        mPrintf("? ");
                    else
                        mPrintf("\n Not a directory room.");
                } else
                    readdirectory(verbose);
                abort = TRUE;
                break;
            case 'E':
                mPrintf("\bExclusive ");
                mf.mfMai = TRUE;
                done = FALSE;
                break;
            case 'F':
                mPrintf("\bForward ");
                revOrder = FALSE;
                whichMess = OLDaNDnEW;
                done = FALSE;
                break;
            case 'H':
                mPrintf("\bHallways ");
                readhalls();
                abort = TRUE;
                break;
            case 'I':
                mPrintf("\bInfo file(s)");
                if (!roomBuf.rbflags.MSDOSDIR) {
                    if (gl_user.expert)
                        mPrintf("? ");
                    else
                        mPrintf("\n Not a directory room.");
                } else
                    readinfofile(verbose);
                abort = TRUE;
                break;
            case 'L':
                mPrintf("\bLimited-access ");
                mf.mfLim = TRUE;
                done = FALSE;
                break;
            case 'N':
                mPrintf("\bNew ");
                whichMess = NEWoNLY;
                done = FALSE;
                break;
            case 'O':
                mPrintf("\bOld ");
                revOrder = TRUE;
                whichMess = OLDoNLY;
                done = FALSE;
                break;
            case 'P':
                mPrintf("\bPublic ");
                mf.mfPub = TRUE;
                done = FALSE;
                break;
            case 'R':
                mPrintf("\bReverse ");
                revOrder = TRUE;
                whichMess = OLDaNDnEW;
                done = FALSE;
                break;
            case 'S':
                mPrintf("\bStatus\n ");
                systat();
                abort = TRUE;
                break;
            case 'T':
                mPrintf("\bTextfile: ");
                if (!roomBuf.rbflags.MSDOSDIR) {
                    if (gl_user.expert)
                        mPrintf("? ");
                    else
                        mPrintf("\n Not a directory room.");
                } else
                    readtextfile();
                abort = TRUE;
                break;
            case 'U':
                mPrintf("\bUser log ");
                Readlog(verbose);
                abort = TRUE;
                break;
            case 'V':
                mPrintf("\bVerbose ");
                verbose = TRUE;
                done = FALSE;
                break;

        /* WC isn't needed on DOS machines which use DSZ or equiv. */
#ifndef __MSDOS__
            case 'W':
                mPrintf("\bWC-protocol download ");
                if (!roomBuf.rbflags.MSDOSDIR) {
                    if (gl_user.expert)
                        mPrintf("? ");
                    else
                        mPrintf("\n Not a directory room.");
                } else
                    readwc();
                abort = TRUE;
                break;
#endif

            case 'Z':
                mPrintf("\bZIP-file(s)");
                if (!roomBuf.rbflags.MSDOSDIR) {
                    if (gl_user.expert)
                        mPrintf("? ");
                    else
                        mPrintf("\n Not a directory room.");
                } else
                    readzip(verbose);
                abort = TRUE;
                break;
            case '?':
                nextmenu("readopt", &(cfg.cnt.readopttut), 1);
                abort = TRUE;
                break;
            default:
                mPrintf("? ");
                abort = TRUE;
                if (!gl_user.expert)
                    nextmenu("readopt", &(cfg.cnt.readopttut), 1);
                break;
        }
        first = '\0';

    } while (!done && moreYet && !abort);

    if (abort)
        return;

    showMessages(whichMess, revOrder, verbose);
}
