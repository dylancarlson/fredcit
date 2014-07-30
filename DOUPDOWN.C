/************************************************************************
 * $Header:   D:/VCS/FCIT/DOUPDOWN.C_V   1.29   01 Nov 1991 11:19:58   FJM  $
 *                              doupdown.c
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
 *              doDownload()
 *              doUpload()
 *
 ************************************************************************/


/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/DOUPDOWN.C_V  $
 * 
 *    Rev 1.29   01 Nov 1991 11:19:58   FJM
 * Added gl_ structures
 *
 *    Rev 1.28   21 Sep 1991 10:18:44   FJM
 * FredCit release
 *
 *    Rev 1.27   06 Jun 1991  9:18:48   FJM
 *    Rev 1.12   18 Jan 1991 16:53:48   FJM
 *    Rev 1.4   06 Jan 1991 12:45:16   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.3   27 Dec 1990 20:17:00   FJM
 * Changes for porting.
 *
 *    Rev 1.0   22 Dec 1990 13:39:52   FJM
 * Initial revision.
 *
 * -------------------------------------------------------------------- */

/************************************************************************/
/*      doDownload()                                                    */
/************************************************************************/
void doDownload(char ex)
{
    ex = ex;

    mPrintf("\bDownload ");

    if (!loggedIn && !cfg.unlogReadOk) {
        mPrintf("\n --Must log in to download.\n ");
        return;
    }
    /* handle uponly flag! */
    if (roomTab[thisRoom].rtflags.UPONLY && !groupseesroom(thisRoom)) {
        mPrintf("\n --Room is upload only.\n ");
        return;
    }
    if (!roomBuf.rbflags.MSDOSDIR) {
        if (gl_user.expert)
            mPrintf("? ");
        else
            mPrintf("\n Not a directory room.");
        return;
    }
    download('\0');
}

/************************************************************************/
/*      doUpload()                                                      */
/************************************************************************/
void doUpload(char ex)
{
    ex = ex;

    mPrintf("\bUpload ");

    /* handle downonly flag! */
    if (roomTab[thisRoom].rtflags.DOWNONLY && !groupseesroom(thisRoom)) {
        mPrintf("\n\n  --Room is download only.\n ");
        return;
    }
    if (!loggedIn && !cfg.unlogEnterOk) {
        mPrintf("\n\n  --Must log in to upload.\n ");
        return;
    }
    if (!roomBuf.rbflags.MSDOSDIR) {
        if (gl_user.expert)
            mPrintf("? ");
        else
            mPrintf("\n Not a directory room.");
        return;
    }
    upload('\0');
    return;
}
