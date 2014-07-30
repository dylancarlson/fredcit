/************************************************************************
 * $Header:   D:/VCS/FCIT/DOENTER.C_V   1.31   01 Nov 1991 11:19:54   FJM  $
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
 *      doEnter()               handles E(nter)         command
 *
 ************************************************************************/


/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/DOENTER.C_V  $
 * 
 *    Rev 1.31   01 Nov 1991 11:19:54   FJM
 * Added gl_ structures
 *
 *    Rev 1.30   21 Sep 1991 10:18:38   FJM
 * FredCit release
 *
 *    Rev 1.29   06 Jun 1991  9:18:42   FJM
 *
 *    Rev 1.28   27 May 1991 11:42:12   FJM
 *
 *    Rev 1.27   16 May 1991  8:42:44   FJM
 * Speed ups for message entry.
 *
 *    Rev 1.12   18 Jan 1991 16:53:54   FJM
 *    Rev 1.8   11 Jan 1991 12:44:28   FJM
 *    Rev 1.4   06 Jan 1991 12:44:36   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.3   27 Dec 1990 20:17:02   FJM
 * Changes for porting.
 *
 *    Rev 1.0   22 Dec 1990 13:40:30   FJM
 * Initial revision.
 *
 * -------------------------------------------------------------------- */

/***********************************************************************/
/*     doEnter() handles E(nter) command                               */
/***********************************************************************/
void doEnter(char moreYet, char first)
/* moreYet;           TRUE to accept following parameters */
/* first;             first parameter if true             */
{
    char done;
    char letter;

    if (moreYet)
        first = '\0';

    done = TRUE;
    mailFlag = FALSE;
    oldFlag = FALSE;
    limitFlag = FALSE;
    linkMess = FALSE;

    mPrintf("\bEnter ");

    if (first)
        oChar(first);

    do {
        outFlag = IMPERVIOUS;
        done = TRUE;

        letter = (char) (toupper(first ? (char) first : (char) iChar()));

    /* allow exclusive mail entry only */
        if (!loggedIn && !cfg.unlogEnterOk && (letter != 'E')) {
            mPrintf("\b\n  --Must log in to enter.\n ");
            break;
        }
    /* handle readonly flag! */
        if (roomTab[thisRoom].rtflags.READONLY && !groupseesroom(thisRoom)
            && ((letter == '\r') || (letter == '\n') || (letter == 'M')
        || (letter == 'E') || (letter == 'O') || (letter == 'G'))) {
            mPrintf("\b\n\n  --Room is readonly.\n ");
            break;
        }
    /* handle steeve */
        if (MessageRoom[thisRoom] == cfg.MessageRoom &&
			!(gl_user.sysop || gl_user.aide) &&
            ((letter == '\r') || (letter == '\n') || (letter == 'M')
        || (letter == 'E') || (letter == 'O') || (letter == 'G'))) {
            mPrintf("\b\n\n  --Only %d messages per room.\n ", cfg.MessageRoom);
            break;
        }
    /* handle nomail flag! */
        if (logBuf.lbflags.NOMAIL && (letter == 'E')) {
            mPrintf("\b\n\n  --You can't enter mail.\n ");
            break;
        }
    /* handle downonly flag! */
        if (roomTab[thisRoom].rtflags.DOWNONLY && !groupseesroom(thisRoom)
        && ((letter == 'T') || (letter == 'W'))) {
            mPrintf("\b\n\n  --Room is download only.\n ");
            break;
        }
        switch (letter) {
            case '\r':
            case '\n':
                moreYet = FALSE;
                makeMessage();
                break;
            case 'C':
                mPrintf("\bConfiguration ");
                configure(FALSE);
                break;
            case 'D':
                mPrintf("\bDefault-hallway ");
                doCR();
                defaulthall();
                break;
            case 'E':
                mPrintf("\bExclusive message ");
                doCR();
                if (whichIO != CONSOLE)
                    echo = CALLER;
                limitFlag = FALSE;
                mailFlag = TRUE;
                makeMessage();
                echo = BOTH;
                break;
            case 'F':
                mPrintf("\bForwarding-address ");
                doCR();
                forwardaddr();
                break;
            case 'H':
                mPrintf("\bHallway ");
                doCR();
                enterhall();
                break;
            case 'L':
                mPrintf("\bLimited-access ");
                done = FALSE;
                limitFlag = TRUE;
                break;
            case '*':
                if (!gl_user.sysop)
                    mPrintf("\b\n\n  --You can't enter a file-linked message.\n ");
                else {
                    mPrintf("\bFile-linked ");
                    done = FALSE;
                    linkMess = TRUE;
                }
                break;
            case 'M':
                mPrintf("\bMessage ");
                doCR();
                makeMessage();
                break;
            case 'G':
                mPrintf("\bGroup-only Message");
                doCR();
                limitFlag = TRUE;
                makeMessage();
                break;
            case 'O':
                mPrintf("\bOld-message ");
                done = FALSE;
                oldFlag = TRUE;
                break;
            case 'P':
                mPrintf("\bPassword ");
                doCR();
                newPW();
                break;
            case 'R':
                mPrintf("\bRoom ");
                if (!cfg.nonAideRoomOk && !gl_user.aide) {
                    mPrintf("\n --Must be aide to create room.\n ");
                } else if (!loggedIn) {
                    mPrintf("\n --Must log in to create new room.\n ");
                } else {
					doCR();
					makeRoom();
				}
                break;
            case 'T':
                mPrintf("\bTextfile ");
                if (roomBuf.rbflags.MSDOSDIR != 1) {
                    if (gl_user.expert)
                        mPrintf("? ");
                    else
                        mPrintf("\n Not a directory room.");
                    return;
                }
                if (!loggedIn) {
                    mPrintf("\n --Must be logged in.\n ");
                    break;
                }
                entertextfile();
                break;

        /* WC isn't needed on DOS machines which use DSZ or equiv. */
#ifndef __MSDOS__
            case 'W':
                mPrintf("\bWC-protocol upload ");
                if (!roomBuf.rbflags.MSDOSDIR) {
                    if (gl_user.expert)
                        mPrintf("? ");
                    else
                        mPrintf("\n Not a directory room.");
                } else if (!loggedIn) {
                    mPrintf("\n --Must be logged in.\n ");
                } else
                    enterwc();
                doCR();
                break;
#endif
            case 'A':
                mPrintf("\bApplication");
                if (!loggedIn) {
                    mPrintf("\n --Must be logged in.\n ");
                } else
                    ExeAplic();
                break;
            case 'X':
                mPrintf("\bExclude Room ");
                exclude();
                break;
            case 'S':

                if (!cfg.surnames) {
                    mPrintf(
                    "\b\n\nSurnames & Titles are not allowed on this system.\n");
                    doCR();
                } else if (!gl_user.sysop && !cfg.entersur) {
                    mPrintf("\b\n\n  --Users can't enter their surname.\n ");
                    break;
                } else if (!gl_user.sysop && logBuf.SURNAMLOK) {
                    mPrintf("\b\n\n  --Your surname has been locked!\n ");
                    break;
                } else {
                    label tempsur;

                    mPrintf("\bSurname / Title\n ");

                    getString("title", tempsur, NAMESIZE, 0, ECHO,
						logBuf.title);
                    if (*tempsur)
                        strcpy(logBuf.title, tempsur);
                    normalizeString(logBuf.title);

                    getString("surname", tempsur, NAMESIZE, 0, ECHO,
						logBuf.surname);
                    if (*tempsur)
                        strcpy(logBuf.surname, tempsur);
                    normalizeString(logBuf.surname);
                }
                break;
            case '?':
                nextmenu("entopt", &(cfg.cnt.entopttut), 1);
                break;
            default:
                mPrintf("? ");
                if (!gl_user.expert)
                    nextmenu("entopt", &(cfg.cnt.entopttut), 1);
                break;
        }
    }
    while (!done && moreYet);

    oldFlag = mailFlag = limitFlag = FALSE;

}
