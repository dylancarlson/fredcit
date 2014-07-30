/************************************************************************
 * $Header:   D:/VCS/FCIT/DOAIDE.C_V   1.31   01 Nov 1991 11:19:52   FJM  $
 *                              doaide.c
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
 *      doAide()                handles Aide-only       commands
 *
 ************************************************************************/


/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/DOAIDE.C_V  $
 * 
 *    Rev 1.31   01 Nov 1991 11:19:52   FJM
 * Added gl_ structures
 *
 *    Rev 1.30   21 Sep 1991 10:18:36   FJM
 * FredCit release
 *
 *    Rev 1.29   06 Jun 1991  9:18:40   FJM
 *
 *    Rev 1.28   27 May 1991 11:41:58   FJM
 *
 *    Rev 1.17   28 Jan 1991 13:14:00   FJM
 * Made room edit prompt inverse.
 *
 *    Rev 1.12   18 Jan 1991 16:53:44   FJM
 *    Rev 1.4   06 Jan 1991 12:45:08   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.3   27 Dec 1990 20:16:08   FJM
 * Changes for porting.
 *
 *    Rev 1.0   22 Dec 1990 13:38:28   FJM
 * Initial revision.
 *
 * -------------------------------------------------------------------- */

/************************************************************************
 *      doAide() handles the aide-only menu
 *          return FALSE to fall invisibly into default error msg
 ************************************************************************/

void doAide(moreYet, first)
char moreYet;
char first;         /* first parameter if TRUE */
{
    int roomExists();
    char oldchat;

    if (moreYet)
        first = '\0';

    mtPrintf(TERM_REVERSE, "\bAide special fn:");
    mPrintf(" ");

    if (first)
        oChar(first);

    switch (toupper(first ? (char) first : (char) iChar())) {
        case 'A':
            mPrintf("\bAttributes ");
            if (roomBuf.rbflags.MSDOSDIR != 1) {
                if (gl_user.expert)
                    mPrintf("? ");
                else
                    mPrintf("\n Not a directory room.");
            } else
                attributes();
            break;
        case 'C':
            chatReq = TRUE;
            oldchat = (char) cfg.noChat;
            cfg.noChat = FALSE;
            mPrintf("\bChat\n");
            if (whichIO == MODEM)
                ringSysop();
            else
                chat();
            cfg.noChat = oldchat;
            break;
        case 'E':
            mPrintf("\bEdit room\n  \n");
            renameRoom();
            break;
        case 'F':
            mPrintf("\bFile set\n  \n");
            batchinfo(2);
            break;
        case 'G':
            mPrintf("\bGroup membership\n  \n");
            groupfunc();
            break;
        case 'H':
            mPrintf("\bHallway changes\n  \n");
            if (!cfg.aidehall && !gl_user.sysop)
                mPrintf(" Must be a Sysop!\n");
            else
                hallfunc();
            break;
        case 'I':
            mPrintf("\bInsert message\n ");
            insert();
            break;
        case 'K':
            mPrintf("\bKill room\n ");
            killroom();
            break;
        case 'L':
            mPrintf("\bList group");
            listgroup();
            break;
        case 'M':
            mPrintf("\bMove file ");
            moveFile();
            break;
        case 'R':
            mPrintf("\bRename file ");
            if (roomBuf.rbflags.MSDOSDIR != 1) {
                if (gl_user.expert)
                    mPrintf("? ");
                else
                    mPrintf("\n Not a directory room.");
            } else
                renamefile();
            break;

        case 'S':
            mPrintf("\bSet file info\n ");
            if (roomBuf.rbflags.MSDOSDIR != 1) {
                if (gl_user.expert)
                    mPrintf("? ");
                else
                    mPrintf("\n Not a directory room.");
            } else
                setfileinfo();
            break;

        case 'U':
            mPrintf("\bUnlink file\n ");
            if (roomBuf.rbflags.MSDOSDIR != 1) {
                if (gl_user.expert)
                    mPrintf("? ");
                else
                    mPrintf("\n Not a directory room.");
            } else
                unlinkfile();
            break;
        case 'V':
            mPrintf("\bView Help Text File\n ");
            nextblurb("aide", &(cfg.cnt.acount), 1);
            break;
        case 'W':
            mPrintf("\bWindow into hallway\n ");
            if (!cfg.aidehall && !gl_user.sysop)
                mPrintf(" Must be a Sysop!\n");
            else
                windowfunc();
            break;
        case '?':
            nextmenu("aide", &(cfg.cnt.aidetut), 1);
            break;
        default:
            if (!gl_user.expert)
                mPrintf("\n '?' for menu.\n ");
            else
                mPrintf(" ?\n ");
            break;
    }
}
