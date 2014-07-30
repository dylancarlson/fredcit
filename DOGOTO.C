/************************************************************************
 * $Header:   D:/VCS/FCIT/DOGOTO.C_V   1.28   21 Sep 1991 10:18:40   FJM  $
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
 *      doGoto()                handles G(oto)          command
 *
 ************************************************************************/


/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/DOGOTO.C_V  $
 * 
 *    Rev 1.28   21 Sep 1991 10:18:40   FJM
 * FredCit release
 *
 *    Rev 1.27   06 Jun 1991  9:18:44   FJM
 *    Rev 1.12   18 Jan 1991 16:53:50   FJM
 *
 *    Rev 1.4   06 Jan 1991 12:45:12   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.3   27 Dec 1990 20:17:00   FJM
 * Changes for porting.
 *
 *    Rev 1.0   22 Dec 1990 13:40:14   FJM
 * Initial revision.
 *
 * -------------------------------------------------------------------- */

/************************************************************************/
/*      doGoto() handles G(oto) command                                 */
/************************************************************************/
void doGoto(expand, skip)
char expand;            /* TRUE to accept following parameters  */
{
    label roomName;

    if (!skip) {
        mPrintf("\bGoto");
        skiproom = FALSE;
    } else {
        mPrintf("\bBypass to");
        skiproom = TRUE;
    }

    if (!expand) {
		mPrintf(" ");
        gotoRoom("");
    } else {
		getString("", roomName, NAMESIZE, 1, ECHO, "");
		normalizeString(roomName);
	
		if (roomName[0] == '?') {
			listRooms(OLDNEW, FALSE, FALSE);
		} else {
			mPrintf(" ");
			gotoRoom(roomName);
		}
	}
}
