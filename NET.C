/* -------------------------------------------------------------------- */
/*  NET.C                     Citadel                                   */
/* -------------------------------------------------------------------- */
/*      Networking libs for the Citadel bulletin board system           */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Includes                                                            */
/* -------------------------------------------------------------------- */
#include <string.h>
#include <time.h>
#include "response.h"			/* Hayes command responses */
#include "ctdl.h"

#ifndef ATARI_ST
#include <conio.h>
#include <dos.h>
#endif

#include "keywords.h"
#include "proto.h"
#include "global.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  GetStr()        gets a null-terminated string from a file           */
/*  PutStr()        puts a null-terminated string to a file             */
/*  GetMessage()    Gets a message from a file, returns sucess          */
/*  fPutMessage()   Puts a message to a file                            */
/*  NewRoom()       Puts all new messages in a room to a file           */
/*  saveMessage()   saves a message to file if it is netable            */
/*  ReadMsgFl()     Reads a message file into thisRoom                  */
/*  NfindRoom()     find the room for mail (unimplmented, ret: MAILROOM)*/
/*  readnode()      read the node.cit to get the nodes info for logbuf  */
/*  getnode()       read the node.cit to get the nodes info             */
/*  net_slave()     network entry point from LOGIN                      */
/*  net_master()    entry point to call a node                          */
/*  slave()         Actual networking slave                             */
/*  master()        During network master code                          */
/*  n_dial()        call the bbs in the node buffer                     */
/*  n_login()       Login to the bbs with the macro in the node file    */
/*  wait_for()      wait for a string to come from the modem            */
/*  net_callout()   Entry point from Cron.C                             */
/*  cleanup()       Done with other system, save mail and messages      */
/*  netcanseeroom() Can the node see this room?                         */
/*  alias()         return the name of the BBS from the #ALIAS          */
/*  route()         return the routing of a BBS from the #ROUTE         */
/*  alias_route()   returns the route or alias specified                */
/*  get_first_room()    get the first room in the room list             */
/*  get_next_room() gets the next room in the list                      */
/*  save_mail()     save a message bound for another system             */
/* -------------------------------------------------------------------- */


/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/NET.C_V  $
 * 
 *    Rev 1.53   01 Nov 1991 11:20:40   FJM
 * Added gl_ structures
 *
 *    Rev 1.52   21 Sep 1991 10:19:18   FJM
 * FredCit release
 *
 *    Rev 1.51   08 Jul 1991 16:19:32   FJM
 *
 *    Rev 1.50   15 Jun 1991  8:44:28   FJM
 * Prevent ANSI from netting out in routing fields.
 *
 *    Rev 1.49   14 Jun 1991  7:42:28   FJM
 * Started conversions to strip ANSI from network routing fields.
 *
 *    Rev 1.48   06 Jun 1991  9:19:36   FJM
 *
 *    Rev 1.47   29 May 1991 11:18:46   FJM
 * Timer for "waiting for transfer"
 * Timeouts in waiting for transfer section
 * Timeouts in wait_for when modem input is comming in
 *
 *    Rev 1.46   22 May 1991  2:17:44   FJM
 *
 *    Rev 1.45   18 May 1991  9:32:16   FJM
 * Made readNode() a macro for size/speed.
 *
 *    Rev 1.44   06 May 1991 17:29:28   FJM
 *    Rev 1.41   17 Apr 1991 12:55:44   FJM
 *    Rev 1.40   11 Apr 1991 10:31:46   FJM
 * Added delay before buffer flush after dial out.
 *
 *    Rev 1.35   28 Feb 1991 12:18:00   FJM
 * Flush modem before response detect in verbose_response().
 *
 *    Rev 1.34   10 Feb 1991 18:02:34   FJM
 * Made response code check explicit.
 *
 *    Rev 1.31   05 Feb 1991 14:32:08   FJM
 * Added modem response code detection for dial out.
 *
 *    Rev 1.22   11 Jan 1991 12:43:36   FJM
 *    Rev 1.18   06 Jan 1991 12:44:40   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.17   27 Dec 1990 20:16:22   FJM
 * Changes for porting.
 *
 *    Rev 1.14   22 Dec 1990 13:38:46   FJM
 *    Rev 1.12   19 Dec 1990  2:01:34   FJM
 * Modifed ReadMessage to drop messages without a digit starting the source
 * ID field.
 *
 *    Rev 1.7   07 Dec 1990 23:10:34   FJM
 *    Rev 1.6   02 Dec 1990  0:55:48   FJM
 * Enhanced local netting.
 *
 *    Rev 1.5   01 Dec 1990  2:55:50   FJM
 * Added local netting support.
 *
 *    Rev 1.4   23 Nov 1990 13:25:18   FJM
 * Header change
 *
 *    Rev 1.2   17 Nov 1990 16:12:16   FJM
 * Added version control log header
 * --------------------------------------------------------------------
 *  EARLY HISTORY:
 *
 *  06/05/89  (PAT)  Made history, cleaned up comments, reformatted
 *                   icky code.
 *  02/06/90  {zm}   Fix #GROUP bug so they're in right order.
 *  03/09/90  {zm}   Release all incoming [viewable twit] msgs.
 *                   Add netmail count to "messages entered" in trap.
 *  03/10/90  {zm}   If not sysop, send [viewable] msgs as normal.
 *  03/11/90  {zm}   Change the above to sysop >or< aide.
 *                   Add a default group for group-only msgs.
 *  03/11/90  FJM    Usage of memset in netting
 *  03/15/90  {zm}   Add [title] name [surname] everywhere.
 *  03/16/90  {zm}   Add route path and other new DragCit msg fields.
 *  04/08/90  FJM    Force creation of empty mail file if none exists
 *  04/18/90  FJM    Cleaned up networking code some in NewRoom().
 *  04/19/90  FJM    Cleaned up string copy routines
 *  08/10/90  FJM    Renamed mail file to mesg.tmp before send
 *                   in slave()
 *  08/16/90  FJM    Modified group mapping for no group found.
 *  09/02/90  FJM    Fixed length of "str" in ReadMsgFl().
 *  10/13/90  FJM    Changed length test in GetStr().
 *  10/18/90  FJM    Added #DEFAULT_ROUTE keyword.
 *
 * -------------------------------------------------------------------- */


/* --------------------------------------------------------------------
 *  Macro functions
 * -------------------------------------------------------------------- */

#define readnode()	(getnode(logBuf.lbname))
#define GetStrNA(f,s,l) (GetStr((f),(s),(l)),stripANSI(s))

/* -------------------------------------------------------------------- */
/*  Global Data                                                         */
/* -------------------------------------------------------------------- */
int local_mode = FALSE;

/*
 * Shared with MSG3.C
 */

label default_route = "";

/* --------------------------------------------------------------------
 *  Local functions
 * -------------------------------------------------------------------- */

static void PutStrNA(FILE * fl, char *str);
static int verbose_response(char *buf);

/* -------------------------------------------------------------------- */
/*  GetMessage()    Gets a message from a file, returns success     (i) */
/* -------------------------------------------------------------------- */

// !!!!!!!!!!!!!!!!!!!!! NOTE !!!!!!!!!!!!!!!!!!!!!!!!!!!
// need a GetStrNA here

BOOL GetMessage(FILE * fl)
{
    char c;

    /* clear message buffer out */
    clearmsgbuf();

    /* find start of message */
    do {
        c = (uchar) fgetc(fl);
    } while (c != -1 && !feof(fl));

    if (feof(fl))
        return FALSE;

    /* get message's attribute byte */
    msgBuf->mbattr = (uchar) fgetc(fl);

    GetStr(fl, msgBuf->mbId, NAMESIZE);

    do {
        c = (uchar) fgetc(fl);
        switch (c) {
            case 'A':
                GetStrNA(fl, msgBuf->mbauth, NAMESIZE);
                break;
            case 'N':
                GetStr(fl, msgBuf->mbtitle, NAMESIZE);
                break;
            case 'n':
                GetStr(fl, msgBuf->mbsur, NAMESIZE);
                break;
            case 'D':
                GetStr(fl, msgBuf->mbtime, NAMESIZE);
                break;
            case 'G':
                GetStrNA(fl, msgBuf->mbgroup, NAMESIZE);
                break;
            case 'I':
                GetStrNA(fl, msgBuf->mbreply, NAMESIZE);
                break;
            case 'M':       /* will be read off disk later */
                break;
            case 'O':
                GetStrNA(fl, msgBuf->mboname, NAMESIZE);
                break;
            case 'o':
                GetStrNA(fl, msgBuf->mboreg, NAMESIZE);
                break;
            case 'Q':
                GetStrNA(fl, msgBuf->mbocont, NAMESIZE);
                break;
            case 'P':
                GetStrNA(fl, msgBuf->mbfpath, 128);
                break;
            case 'p':
                GetStrNA(fl, msgBuf->mbtpath, 128);
                break;
            case 'R':
                GetStrNA(fl, msgBuf->mbroom, NAMESIZE);
                break;
            case 'S':
                GetStrNA(fl, msgBuf->mbsrcId, NAMESIZE);
                break;
            case 'T':
                GetStrNA(fl, msgBuf->mbto, NAMESIZE);
                break;
            case 'X':
                GetStrNA(fl, msgBuf->mbx, NAMESIZE);
                break;
            case 'Z':
                GetStrNA(fl, msgBuf->mbzip, NAMESIZE);
                break;
            case 'z':
                GetStrNA(fl, msgBuf->mbrzip, NAMESIZE);
                break;
            case 'q':
                GetStrNA(fl, msgBuf->mbczip, NAMESIZE);
                break;

            default:
                GetStr(fl, msgBuf->mbtext, cfg.maxtext);    /* discard unknown field */
                msgBuf->mbtext[0] = '\0';
                break;
        }
    } while (c != 'M' && !feof(fl));

    if (!*msgBuf->mboname) {    /* 'O' */
        strncpy(msgBuf->mboname, node.ndname, sizeof(label) - 1);
    }
    if (!*msgBuf->mboreg) { /* 'o' */
        strncpy(msgBuf->mboreg, node.ndregion, sizeof(label) - 1);
    }
    if (!*msgBuf->mbsrcId) {    /* 'S' */
        strncpy(msgBuf->mbsrcId, msgBuf->mbId, sizeof(label) - 1);
    }
    if (!*msgBuf->mbfpath) {    /* 'P' - no route path, so start one. */
        strncpy(msgBuf->mbfpath, node.ndname, sizeof(label) - 1);
    }
    if (feof(fl))
        return FALSE;

    GetStr(fl, msgBuf->mbtext, cfg.maxtext);    /* get the message field  */

    return TRUE;
}

/* -------------------------------------------------------------------- */
/*  fPutMessage()    Puts a message to a file                        (o) */
/* -------------------------------------------------------------------- */

void fPutMessage(FILE * fl)
{
    /* write start of message */
    fputc(0xFF, fl);

    /* put message's attribute byte */
    fputc(msgBuf->mbattr, fl);

    /* put local ID # out */
    PutStr(fl, msgBuf->mbId);

    if (msgBuf->mbauth[0]) {
        fputc('A', fl);
        PutStrNA(fl, msgBuf->mbauth);
    }
    if (!msgBuf->mbtime[0]) {
        sprintf(msgBuf->mbtime, "%ld", cit_time());
    }
    fputc('D', fl);
    PutStr(fl, msgBuf->mbtime);

    if (msgBuf->mbgroup[0]) {
        fputc('G', fl);
        PutStrNA(fl, msgBuf->mbgroup);
    }
    if (msgBuf->mbreply[0]) {
        fputc('I', fl);
        PutStrNA(fl, msgBuf->mbreply);
    }
    if (!msgBuf->mboname[0]) {
        strncpy(msgBuf->mboname, cfg.nodeTitle, sizeof(label) - 1);
        strncpy(msgBuf->mboreg, cfg.nodeRegion, sizeof(label) - 1);
    }
    if (msgBuf->mboname[0]) {
        fputc('O', fl);
        PutStrNA(fl, msgBuf->mboname);
    }
    if (msgBuf->mboreg[0]) {
        fputc('o', fl);
        PutStrNA(fl, msgBuf->mboreg);
    }
    if (msgBuf->mbocont[0]) {
        fputc('Q', fl);
        PutStrNA(fl, msgBuf->mbocont);
    }
    fputc('P', fl);     /* always have a route path */
    if (msgBuf->mbfpath[0]) {
        if (strlen(msgBuf->mbfpath) + strlen(cfg.nodeTitle) + 2 < 128) {
            strcat(msgBuf->mbfpath, "!");
            strcat(msgBuf->mbfpath, cfg.nodeTitle); /* concatenate our name */
        } else {
            amPrintf(" \n Path overflow in message #%s\n", msgBuf->mbId);
            netError = TRUE;
        }

    } else {
        strcpy(msgBuf->mbfpath, cfg.nodeTitle); /* our node name only */
    }
    PutStrNA(fl, msgBuf->mbfpath);

    if (msgBuf->mbtpath[0]) {
        fputc('p', fl);
        PutStrNA(fl, msgBuf->mbtpath);
    }
    if (msgBuf->mbroom[0]) {
        fputc('R', fl);
        PutStrNA(fl, msgBuf->mbroom);
    }
    if (!msgBuf->mbsrcId[0]) {
        strncpy(msgBuf->mbsrcId, msgBuf->mbId, sizeof(label) - 1);
    }
    if (msgBuf->mbsrcId[0]) {
        fputc('S', fl);
        PutStrNA(fl, msgBuf->mbsrcId);
    }
    if (msgBuf->mbtitle[0]) {
        fputc('N', fl);
        PutStr(fl, msgBuf->mbtitle);
    }
    if (msgBuf->mbsur[0]) {
        fputc('n', fl);
        PutStr(fl, msgBuf->mbsur);
    }
    if (msgBuf->mbto[0]) {
        fputc('T', fl);
        PutStrNA(fl, msgBuf->mbto);
    }
    if (msgBuf->mbx[0]) {   /* problem user message */
        fputc('X', fl);
        PutStrNA(fl, msgBuf->mbx);
    }
    if (msgBuf->mbzip[0]) {
        fputc('Z', fl);
        PutStrNA(fl, msgBuf->mbzip);
    }
    if (msgBuf->mbrzip[0]) {
        fputc('z', fl);
        PutStrNA(fl, msgBuf->mbrzip);
    }
    if (msgBuf->mbczip[0]) {
        fputc('q', fl);
        PutStrNA(fl, msgBuf->mbczip);
    }
    /* put the message field  */
    fputc('M', fl);
    PutStr(fl, msgBuf->mbtext);
}

/* -------------------------------------------------------------------- */
/*  NewRoom()       Puts all new messages in a room to a file           */
/* -------------------------------------------------------------------- */
void NewRoom(int room, char *filename)
{
    int i, h;
    char str[100];
    ulong lowLim, highLim, msgNo;
    FILE *file;
    unsigned tsize;

    lowLim = logBuf.lbvisit[logBuf.lbroom[room].lvisit] + 1;
    highLim = cfg.newest;

    logBuf.lbroom[room].lvisit = 0;

    /* stuff may have scrolled off system unseen, so: */
    if (cfg.oldest > lowLim)
        lowLim = cfg.oldest;

    sprintf(str, "%s\\%s", cfg.temppath, filename);

    file = fopen(str, "ab");
    if (!file) {
        return;
    }
    h = hash(cfg.nodeTitle);
    /* need to clean up here !!!! Start at first new message! */
    /* don't sizetable() every pass */
    tsize = sizetable();
    for (i = 0; i != tsize; i++) {
    /* skip messages not in this room */
        if (msgTab[i].mtroomno != (uchar) room)
            continue;

    /* no open messages from the system */
        if (msgTab[i].mtauthhash == h)
            continue;

    /* skip mail */
        if (msgTab[i].mtmsgflags.MAIL)
            continue;

    /* skip >unreleased< twit/moderated messages only.         */
    /* remember that 'aide_can_see_moderated' is in CONFIG.CIT */
    /* but we are ignoring that fact since it's too much fuss. */
/*      (omit for testing)
        if ((msgTab[i].mtmsgflags.PROBLEM
          || msgTab[i].mtmsgflags.MODERATED)
         && (!msgTab[i].mtmsgflags.MADEVIS)
         && (!msgTab[i].mtmsgflags.RELEASED)
         && !(gl_user.sysop || gl_user.aide))  continue;
*/
        msgNo = (ulong) (cfg.mtoldest + i);

        if (msgNo >= lowLim && highLim >= msgNo) {
            saveMessage(msgNo, file);
            mread++;
        }
    }
    fclose(file);
}

/* -------------------------------------------------------------------- */
/*  saveMessage()   saves a message to file if it is nettable       (o) */
/* -------------------------------------------------------------------- */
#define msgstuff  (msgTab[slot].mtmsgflags)
void saveMessage(ulong id, FILE * fl)
{
    ulong here;
    ulong loc;
    int slot;

    slot = indexslot(id);

    if (slot == ERROR)
        return;

    if (msgTab[slot].mtmsgflags.COPY) {
        copyflag = TRUE;
        originalId = id;
        originalattr = 0;

        originalattr = (uchar)
        (originalattr | (msgstuff.RECEIVED) ? ATTR_RECEIVED : 0);
        originalattr = (uchar)
        (originalattr | (msgstuff.REPLY) ? ATTR_REPLY : 0);
        if (gl_user.sysop || gl_user.aide) {
			/* non-sysop (or aide) gets "normal" messages. */
            originalattr = (uchar)
            (originalattr | (msgstuff.MADEVIS) ? ATTR_MADEVIS : 0);
        }
        if (msgTab[slot].mtoffset <= slot)
            saveMessage((ulong) (id - (ulong) msgTab[slot].mtoffset), fl);

        return;
    }
    /* in case it returns without clearing buffer */
    msgBuf->mbfwd[0] = '\0';
    msgBuf->mbto[0] = '\0';

    loc = msgTab[slot].mtmsgLoc;
    if (loc == ERROR)
        return;

    if (copyflag) {
        slot = indexslot(originalId);
    } else {            /* the following doesn't notice copyflag. */
        if (!mayseeindexmsg(slot) && !msgTab[slot].mtmsgflags.NET)
            return;
    }

    fseek(msgfl, loc, 0);

    getMessage();
    getMsgStr(msgBuf->mbtext, cfg.maxtext);

    sscanf(msgBuf->mbId, "%lu", &here);

    /* cludge to return on dummy msg #1 */
    if ((int) here == 1)
        return;

    if (!mayseemsg() && !msgTab[slot].mtmsgflags.NET)
        return;

    if (here != id) {
        cPrintf(" Can't find message. Looking for %lu at byte %ld! (net)\n ",
        id, loc);
        return;
    }
    if (!(gl_user.sysop || gl_user.aide)) {
        msgBuf->mbattr &= ~(ATTR_MADEVIS);  /* strip the [viewable] bit */
        msgBuf->mbx[0] = '\0';  /* since it's a normal msg. */
    }
    if (msgBuf->mblink[0])
        return;

    fPutMessage(fl);
}

/* -------------------------------------------------------------------- */
/*  ReadMsgFile()   Reads a message file into thisRoom              (i) */
/* -------------------------------------------------------------------- */
int ReadMsgFl(int room, char *filename, char *here, char *there)
{
    FILE *file;
    ulong oid, id, loc;
    int i, bad, oname, temproom, lp;
    int goodmsg = 0;
    int rejects = 0;
    char str[120];

    changedir(cfg.temppath);
    expired = duplicate = 0;

    file = fopen(filename, "rb");

    if (!file)
        return -1;

    while (GetMessage(file) == TRUE) {
        msgBuf->mbroomno = (uchar) room;

        sscanf(msgBuf->mbsrcId, "%ld", &oid);
        oname = hash(msgBuf->mboname);

        memcpy(msgBuf2, msgBuf, sizeof(struct msgB));

        bad = FALSE;

        if (!isdigit(*msgBuf->mbId)) {
            bad = TRUE;
            ++rejects;
        }
        if (strcmpi(cfg.nodeTitle, msgBuf->mboname) == SAMESTRING) {
            bad = TRUE;
            duplicate++;
        }
        if (*msgBuf->mbzip) {   /* is mail */
        /* not for this system */
            if (strcmpi(msgBuf->mbzip, cfg.nodeTitle) != SAMESTRING) {
                if (!save_mail()) {
                    clearmsgbuf();
                    strncpy(msgBuf->mbauth, cfg.nodeTitle, sizeof(label) - 1);
                    strncpy(msgBuf->mbto, msgBuf2->mbauth, sizeof(label) - 1);
                    strncpy(msgBuf->mbzip, msgBuf2->mboname, sizeof(label) - 1);
                    strncpy(msgBuf->mbrzip, msgBuf2->mboreg, sizeof(label) - 1);
                    strncpy(msgBuf->mbroom, msgBuf2->mbroom, sizeof(label) - 1);
                    sprintf(msgBuf->mbtext,
						" \n Can not route mail to '%s' via path '%s'.\n",
						msgBuf2->mbzip, msgBuf2->mbfpath);
                    amPrintf(
						" \n Can not route mail from '%s' to '%s' via path '%s'.\n",
						msgBuf2->mboname, msgBuf2->mbzip, msgBuf->mbfpath);

                    save_mail();
                }
                bad = TRUE;
            } else {
        /* for this system */
                if (*msgBuf->mbto && personexists(msgBuf->mbto) == ERROR
                && strcmpi(msgBuf->mbto, "Sysop") != SAMESTRING) {
                    clearmsgbuf();
                    strncpy(msgBuf->mbauth, cfg.nodeTitle, sizeof(label) - 1);
                    strncpy(msgBuf->mbto, msgBuf2->mbauth, sizeof(label) - 1);
                    strncpy(msgBuf->mbzip, msgBuf2->mboname, sizeof(label) - 1);
                    strncpy(msgBuf->mbrzip, msgBuf2->mboreg, sizeof(label) - 1);
                    strncpy(msgBuf->mbroom, msgBuf2->mbroom, sizeof(label) - 1);
                    sprintf(msgBuf->mbtext,
						" \n No '%s' user found on %s.", msgBuf2->mbto,
						cfg.nodeTitle);
                    save_mail();
                    bad = TRUE;
                }
            }
        } else {
        /* is public */
            if (!bad) {
                for (i = sizetable(); i != -1 && !bad; i--) {
                    if (msgTab[i].mtorigin == oname
                    && oid == msgTab[i].mtomesg) {
                        loc = msgTab[i].mtmsgLoc;
                        fseek(msgfl, loc, 0);
                        getMessage();
                        if (strcmpi(msgBuf->mbauth, msgBuf2->mbauth)
                            == SAMESTRING
                            && strcmpi(msgBuf->mboname, msgBuf2->mboname)
                            == SAMESTRING
                            && strcmpi(msgBuf->mbtime, msgBuf2->mbtime)
                            == SAMESTRING
            /*
             * && strcmpi(msgBuf->mboreg, msgBuf2->mboreg) == SAMESTRING
             */
            /* Changed beacuse of region name changes */
                        ) {
                            bad = TRUE;
                            duplicate++;
                        }
                    }
                }
            }
            memcpy(msgBuf, msgBuf2, sizeof(struct msgB));

        /* fix group only messages, or discard them! */
            if (*msgBuf->mbgroup && !bad) {
                bad = TRUE;
                for (i = 0; node.ndgroups[i].here[0]; i++) {
                    if (strcmpi(node.ndgroups[i].there, msgBuf->mbgroup)
                    == SAMESTRING) {    /* fixed */
                        strncpy(msgBuf->mbgroup, node.ndgroups[i].here, sizeof(label) - 1);
                        bad = FALSE;
                    }
                }
                if (bad) {  /* still didn't find the group! */
                    if (default_group[0]) {
                        sprintf(str, "Message #%s mapped from group '%s'.",
							msgBuf->mbId, msgBuf->mbgroup);
                        trap(str, T_NETWORK);
                        amPrintf(" \n %s", str);
                        netError = TRUE;
                        strncpy(msgBuf->mbgroup, default_group, sizeof(label) - 1);
                        bad = FALSE;
                    }       /* #DEFAULT_GROUP is in external.cit */
                }
            }
        /* Expired? */
            if (atol(msgBuf2->mbtime)
            < (cit_time() - ((long) node.ndexpire * 60 * 60 * 24))) {
                bad = TRUE;
                expired++;
            }
        }

        if (!bad) {     /* it's good, save it */
            temproom = room;

            if (strcmpi(msgBuf->mbroom, there) == SAMESTRING)
                strncpy(msgBuf->mbroom, here, sizeof(label) - 1);

            if (*msgBuf->mbto)
                temproom = NfindRoom(msgBuf->mbroom);

            msgBuf->mbroomno = (uchar) temproom;

            putMessage();
            noteMessage();
            goodmsg++;

            if (*msgBuf->mbto) {
                lp = thisRoom;
                thisRoom = temproom;
                notelogmessage(msgBuf->mbto);
                thisRoom = lp;
            }
	/* a viewable twit/moderated msg.  Simulate a local aide releasing it. */

            if (msgBuf->mbx[0] && (msgBuf->mbattr & ATTR_MADEVIS)) {
                sscanf(msgBuf->mbId, "%lu", &id);
                copymessage(id, (uchar) temproom);
            }
        }
    }
    fclose(file);
    if (rejects)
        amPrintf(
			" \n Rejected %d messages with non-numeric source ID numbers.\n",
			rejects);


    return goodmsg;
}

/* -------------------------------------------------------------------- */
/*  NfindRoom()     find the room for mail (unimplmented, ret: MAILROOM)*/
/* -------------------------------------------------------------------- */
int NfindRoom(char *str)
{

/************************************************************************/
/* Does this make sense to you?                                         */
/************************************************************************/
/* yes, it's patched out since it wasn't implemented this way;          */
/* netmail is treated as a separate room file (mesg.tmp), probably      */
/* because Henge-style mail in the rooms just doesn't work right.       */
/* by now, everyone on the DragCit net has gotten used to finding all   */
/* their netmail in the mail room (including me) so I wouldn't worry    */
/* about changing it; let's see how Net ][ works.                       */
/************************************************************************/

    int i = MAILROOM;

    str[0] = str[0];        /* -W3 */

/*  i = roomExists(str);

    if (i == ERROR)
        i = MAILROOM;  */

    return (i);
}

/* -------------------------------------------------------------------- */
/*  readnode()      read the nodes.cit to get the nodes info for logbuf */
/* -------------------------------------------------------------------- */
// BOOL readnode(void)
// {
//     return getnode(logBuf.lbname);
// }

/* -------------------------------------------------------------------- */
/*  getnode()       read the nodes.cit to get the node info             */
/* -------------------------------------------------------------------- */
BOOL getnode(char *nodename)
{
    FILE *fBuf;
    char line[90], ltmp[90];
    char *words[256];
    int i, j, k, found = FALSE;
    long pos, ftell();
    char path[80];

    sprintf(path, "%s\\nodes.cit", cfg.homepath);

    if ((fBuf = fopen(path, "r")) == NULL) {    /* ASCII mode */
        cPrintf("Can't find nodes.cit!");
        doccr();
        return FALSE;
    }
    pos = ftell(fBuf);
    while (fgets(line, 90, fBuf) != NULL) {
        if (line[0] != '#') {
            pos = ftell(fBuf);
            continue;
        }
        if (!found && strnicmp(line, "#NODE", 5) != SAMESTRING) {
            pos = ftell(fBuf);
            continue;
        }
        strcpy(ltmp, line);
        parse_it(words, line);

        for (i = 0; nodekeywords[i] != NULL; i++) {
            if (strcmpi(words[0], nodekeywords[i]) == SAMESTRING) {
                break;
            }
        }

        if (i == NOK_NODE) {
            if (found) {
                fclose(fBuf);
                return TRUE;
            }
            if (strcmpi(nodename, words[1]) == SAMESTRING)
                found = TRUE;
        }
        if (found)
        switch (i) {
            case NOK_BAUD:
                j = atoi(words[1]);
            switch (j) {    /* ycky hack */
                case 300:
                    node.ndbaud = 0;
                    break;
                case 1200:
                    node.ndbaud = 1;
                    break;
                case 2400:
                    node.ndbaud = 2;
                    break;
                case 4800:
                    node.ndbaud = 3;
                    break;
                case 9600:
                    node.ndbaud = 4;
                    break;
                default:
                    node.ndbaud = 1;
                    break;
            }
                break;

            case NOK_PHONE:
                if (strlen(words[1]) < NAMESIZE)
                    strncpy(node.ndphone, words[1], sizeof(label) - 1);
                break;

            case NOK_PROTOCOL:
                if (strlen(words[1]) < NAMESIZE)
                    strncpy(node.ndprotocol, words[1], sizeof(label) - 1);
                break;

            case NOK_MAIL_TMP:
                if (strlen(words[1]) < NAMESIZE)
                    strncpy(node.ndmailtmp, words[1], sizeof(label) - 1);
                break;

            case NOK_LOGIN:
                strncpy(node.ndlogin, ltmp, 90);
                break;

            case NOK_NODE:
                if (strlen(words[1]) < NAMESIZE)
                    strncpy(node.ndname, words[1], sizeof(label) - 1);
                if (strlen(words[2]) < NAMESIZE)
                    strncpy(node.ndregion, words[2], sizeof(label) - 1);
                for (j = 0; j < MAXGROUPS; j++)
                    node.ndgroups[j].here[0] = '\0';
                node.roomoff = 0L;
                break;

            case NOK_REDIAL:
                node.ndredial = atoi(words[1]);
                break;

            case NOK_DIAL_TIMEOUT:
                node.nddialto = atoi(words[1]);
                break;

            case NOK_WAIT_TIMEOUT:
                node.ndwaitto = atoi(words[1]);
                break;

            case NOK_EXPIRE:
                node.ndexpire = atoi(words[1]);
                break;

            case NOK_ROOM:
                if (!node.roomoff)
                    node.roomoff = pos;
                break;


            case NOK_GROUP:
                for (j = 0, k = ERROR; j < MAXGROUPS; j++) {
                    if (node.ndgroups[j].here[0] == '\0') {
                        k = j;
                        j = MAXGROUPS;
                    }
                }

                if (k == ERROR) {
                    cPrintf("Too many groups!!\n ");
                    break;
                }
                if (strlen(words[1]) < NAMESIZE)
                    strncpy(node.ndgroups[k].here, words[1], sizeof(label) - 1);
                if (strlen(words[2]) < NAMESIZE)
                    strncpy(node.ndgroups[k].there, words[2], sizeof(label) - 1);
                if (!strlen(words[2]))
                    strncpy(node.ndgroups[k].there, words[1], sizeof(label) - 1);
                break;

            default:
                cPrintf("Nodes.cit - Warning: Unknown variable %s", words[0]);
                doccr();
                break;
        }
        pos = ftell(fBuf);
    }
    fclose(fBuf);
    return (BOOL) (found);
}

/* -------------------------------------------------------------------- */
/*  net_slave()     network entry point from LOGIN                      */
/* -------------------------------------------------------------------- */
BOOL net_slave(void)
{
    if  (readnode() == FALSE)
        return FALSE;

    if (debug) {
        cPrintf("Node:  \"%s\" \"%s\"", node.ndname, node.ndregion);
        doccr();
        cPrintf("Phone: \"%s\" %d", node.ndphone, node.nddialto);
        doccr();
        cPrintf("Login: \"%s\" %d", node.ndlogin, node.ndwaitto);
        doccr();
        cPrintf("Baud:  %d    Protocol: \"%s\"\n ", node.ndbaud, node.ndprotocol);
        cPrintf("Expire:%d    Waitout:  %d", node.ndexpire, node.ndwaitto);
        doccr();
    }
    netError = FALSE;

    /* cleanup */
    changedir(cfg.temppath);
    ambigUnlink("room.[0-9]*", FALSE);
    ambigUnlink("roomin.*", FALSE);

    if (slave() && master()) {
        cleanup();
        did_net(node.ndname);
        return TRUE;
    } else {
        changedir(cfg.homepath);
        rename("mesg.tmp", node.ndmailtmp);
        return FALSE;
    }
}

/* -------------------------------------------------------------------- */
/*  net_master()    entry point to call a node (cron event)             */
/* -------------------------------------------------------------------- */
BOOL net_master(void)
{
    if  (readnode() == FALSE) {
        cPrintf("\n No nodes.cit entry!\n ");
        return FALSE;
    }
    if (!local_mode) {
        if (n_dial() == FALSE)
            return FALSE;
        if (n_login() == FALSE)
            return FALSE;
        netError = FALSE;
    }
    /* cleanup */
    changedir(cfg.temppath);
    ambigUnlink("room.*", FALSE);
    ambigUnlink("roomin.*", FALSE);

    if (master() && slave()) {
        cleanup();
        did_net(node.ndname);
        return TRUE;

    } else {
        changedir(cfg.homepath);
        rename("mesg.tmp", node.ndmailtmp);
        return FALSE;
    }
}

/* -------------------------------------------------------------------- */
/*  slave()         Actual networking slave                             */
/* -------------------------------------------------------------------- */
BOOL slave(void)
{
    label troo, fn;
    FILE *file, *fopen();
    int i = 0, rm;

    cPrintf(" Sending mail file.");
    doccr();

    changedir(cfg.homepath);
    /* create empty mail file if it doesn't exist */
    file = fopen(node.ndmailtmp, "rb");
    if (!file) {        /* create empty mail file       */
        file = fopen(node.ndmailtmp, "wb");
    }
    if (file) {
        fclose(file);
    }
    unlink("mesg.tmp");
    rename(node.ndmailtmp, "mesg.tmp");
    wxsnd(cfg.homepath, "mesg.tmp",
    (char) strpos((char) tolower(node.ndprotocol[0]), extrncmd));

    /* duplicate code here, make subfunction */
    if (!local_mode && !gotCarrier()) {
        changedir(cfg.homepath);
        rename("mesg.tmp", node.ndmailtmp);
        return FALSE;
    }
    cPrintf(" Receiving room request file.");
    doccr();

    wxrcv(cfg.temppath, "roomreq.tmp",
    (char) strpos((char) tolower(node.ndprotocol[0]), extrncmd));
    if (!local_mode && !gotCarrier()) {
        changedir(cfg.homepath);
        rename("mesg.tmp", node.ndmailtmp);
        return FALSE;
    }
    file = fopen("roomreq.tmp", "rb");
    if (!file) {
        perror("Error opening roomreq.tmp");
        changedir(cfg.homepath);
        rename("mesg.tmp", node.ndmailtmp);
        return FALSE;
    }
    GetStr(file, troo, NAMESIZE);

    doccr();
    cPrintf(" Fetching:");
    doccr();

    while (strlen(troo)) {
        rm = roomExists(troo);
        if (rm != ERROR && netcanseeroom(rm)) {
            if (debug) {
        /* trap room requests if debug on (FJM) */
                sprintf(msgBuf->mbtext, "Room requested by remote: %s", troo);
                trap(msgBuf->mbtext, T_NETWORK);
            }
            sprintf(fn, "room.%d", i);
            cPrintf(" %-*s  ", NAMESIZE, troo);
            if (!((i + 1) % 2))
                doccr();
            NewRoom(rm, fn);
        } else {
            doccr();
            cPrintf(" Can't see or find %s.", troo);
            doccr();
            amPrintf(" \n '%s' room not found for remote.", troo);
            netError = TRUE;
        }

        i++;
        GetStr(file, troo, NAMESIZE);
    }
    doccr();
    fclose(file);
    unlink("roomreq.tmp");

    cPrintf(" Sending message files.");
    doccr();

    if (!local_mode && !gotCarrier()) {
        changedir(cfg.homepath);
        rename("mesg.tmp", node.ndmailtmp);
        return FALSE;
    }
    wxsnd(cfg.temppath, "room.*",
    (char) strpos((char) tolower(node.ndprotocol[0]), extrncmd));

    ambigUnlink("room.*", FALSE);

    return TRUE;
}

/* -------------------------------------------------------------------- */
/*  master()        During network master code                          */
/* -------------------------------------------------------------------- */
BOOL master(void)
{
    char line[75], line2[FILESIZE];
    label here, there;
    FILE *file, *fopen();
    int i, rms;
	time_t t, t2, elapsed;

    if (!local_mode && !gotCarrier())
        return FALSE;

    sprintf(line, "%s\\mesg.tmp", cfg.temppath);
    unlink(line);

    cPrintf(" Receiving mail file.");
    doccr();

    wxrcv(cfg.temppath, "mesg.tmp",
    (char) strpos((char) tolower(node.ndprotocol[0]), extrncmd));

    if (!local_mode && !gotCarrier())
        return FALSE;

    sprintf(line, "%s\\roomreq.tmp", cfg.temppath);
    unlink(line);

    if ((file = fopen(line, "ab")) == NULL) {
        perror("Error opening roomreq.tmp");
        return FALSE;
    }
    for (i = get_first_room(here, there), rms = 0;
        i;
    i = get_next_room(here, there), rms++) {
        PutStr(file, there);
    }

    PutStr(file, "");
    fclose(file);

    cPrintf(" Sending room request file.");
    doccr();

    wxsnd(cfg.temppath, "roomreq.tmp",
    (char) strpos((char) tolower(node.ndprotocol[0]), extrncmd));
    unlink("roomreq.tmp");

    if (!local_mode && !gotCarrier())
        return FALSE;

    /* clear the input buffer */
	portDump();

    /* show time delay while waiting here */
	t2 = 0L;
    cPrintf(" Waiting for transfer:     ");
	t = cit_timer ();
    /* wait for them to get their shit together */
    while (gotCarrier() && !MIReady()) {
		elapsed = cit_timer () - t;
		if (t2 != elapsed) {
			cPrintf ("\b\b\b\b%4ld", elapsed);
			if ((elapsed > 3600L) || KBReady()) {		/* abort? */
				cPrintf (" network attempt aborted.");
				Hangup();
			}
			t2 = elapsed;
		}
	}
	
    doccr();

    if (!local_mode && !gotCarrier())
        return FALSE;

    cPrintf(" Receiving message files.");
    doccr();

    wxrcv(cfg.temppath, "",
		(char) strpos((char) tolower(node.ndprotocol[0]), extrncmd));

    for (i = 0; i < rms; i++) {
        sprintf(line, "room.%d", i);
        sprintf(line2, "roomin.%d", i);
        rename(line, line2);
    }

    return TRUE;
}

/* -------------------------------------------------------------------- */
/*  n_dial()        call the bbs in the node buffer                     */
/* -------------------------------------------------------------------- */
BOOL n_dial(void)
{
    long ts;
    char str[40];
    char ch;
	label resp_buf;
	int resp = HAYES_UNKNOWN;
	int resp_index = 0;

	/* clear the buffer */
	portDump();

    cPrintf("\n \n Dialing...");

    if (debug)
        cPrintf("(%s%s)", cfg.dialpref, node.ndphone);

    baud(node.ndbaud);
    /* update25();	*/
	do_idle(0);

    outstring(cfg.dialsetup);
    outstring("\r");

	delay(2000);
	portDump();
	
    strcpy(str, cfg.dialpref);
    strcat(str, node.ndphone);
    outstring(str);
    outstring("\r");

    ts = cit_timer();

    while (TRUE) {
        if ((int) (cit_timer() - ts) > node.nddialto) {  /* Timeout */
			resp = -2;
            break;
		}

        if (KBReady()) {    /* Aborted by user */
            getch();
            getkey = 0;
			resp = -1;
            break;
        }
        if (local_mode || gotCarrier()) {   /* got carrier!  */
            cPrintf("success");
            return TRUE;
        } else if (MIReady()) {
			ch = (char) getMod();
			if (debug)
				outCon(ch);
			if (ch != '\r') {				/* dump CR's */
				if (ch == '\n') {
					resp_buf[resp_index] = '\0';	/* terminate string */
					resp_index = 0;
					
					resp = verbose_response(resp_buf);
					
					if (debug)
						cPrintf("\n(response was %d:%s)\n",resp,hayes_string[resp]);
					
					/* otherwise unknown */
					if ((resp == HAYES_NOCARRIER) ||
						(resp == HAYES_ERROR) ||
						(resp == HAYES_NODIAL) ||
						(resp == HAYES_BUSY) ||
						(resp == HAYES_NOANSWER))
						break;
					
				} else {		/* add character to buffer */
					if (resp_index < NAMESIZE) {
						resp_buf[resp_index] = ch;
						++resp_index;
					}
				}
			}
		}
    }

	if (resp == -1)
		cPrintf("aborted ");
	else if (resp == -2)
		cPrintf("timeout ");
	else
		cPrintf("failed: %s ",hayes_string[resp]);

    return FALSE;
}

/* -------------------------------------------------------------------- */
/*  n_login()       Login to the bbs with the macro in the node file    */
/* -------------------------------------------------------------------- */
BOOL n_login(void)
{
    time_t ptime, now;
    char ch;
    int i, count;
    char *words[256];

    cPrintf("\n Logging in...");

    count = parse_it(words, node.ndlogin);

    i = 1;

    while (i <= count) {
        switch (tolower(*words[i++])) {
            case 'p':
                if (debug)
                    cPrintf("Pause For (%s)", words[i]);
                ptime=cit_timer();
                ptime += atoi(words[i++]);
                now = ptime;
                while (now < ptime) {
                    now = cit_timer();
                    if (MIReady()) {
                        ch = (char) getMod();
                        if (debug)
                            outCon(ch);
                    }
                }
                break;
            case 's':
                outstring(words[i++]);
                break;
            case 'w':
                if (debug)
                    cPrintf("Wait For (%s)", words[i]);
                if (!wait_for(words[i++])) {
                    cPrintf("failed");
                    return FALSE;
                }
                break;
            case '!':
                apsystem(words[i++]);
                break;
            default:
                break;
        }
    }
    cPrintf("success");
    doccr();
    doccr();
    return TRUE;
}

/* -------------------------------------------------------------------- */
/*  wait_for()      wait for a string to come from the modem            */
/* -------------------------------------------------------------------- */
BOOL wait_for(char *str)
{
#define MAXWLINE	80
    char line[MAXWLINE+1];
    time_t st;
    int stl;
	BOOL rc = FALSE;

    stl = strlen(str);

	if (stl < MAXWLINE) {
		/* clear input buffer */
		memset(line, 0, stl);
		/* set timer */
		st = cit_timer();
		/* get & test the string */
		while (TRUE) {
			if (MIReady()) {
				/* shift input one place right, up to input string length */
				memcpy(line, line + 1, stl);
				/* insert character into end of recieve buffer */
				line[stl - 1] = (char) getMod();
				/* set terminating null */
				line[stl] = '\0';
				/* send to screen if debug is on */
				if (debug)
					outCon(line[stl - 1]);
				/* compare the strings (case independent) */
				if (strcmpi(line, str) == SAMESTRING)
					return TRUE;
			}
			/* check for timeout */
			if ((cit_timer() - st) > (time_t) node.ndwaitto)
				return FALSE;
			/* check for keyboard abort */
			else if (KBReady()) {    /* Aborted by user */
				getch();
				getkey = 0;
				return FALSE;
			}
		}
	} else
		cPrintf("'W' string too long!\n");
	return rc;
}

/* -------------------------------------------------------------------- */
/*  net_callout()   Entry point from Cron.C                             */
/* -------------------------------------------------------------------- */
BOOL net_callout(char *node)
{
    int slot;
    int tmp;

    /* login user */

    mread = entered = 0;

    slot = personexists(node);

    if (slot == ERROR) {
        cPrintf("\n No such node in userlog!");
        local_mode = FALSE;
        return FALSE;
    }
    getLog(&logBuf, logTab[slot].ltlogSlot);

    thisSlot = slot;
    thisLog = logTab[slot].ltlogSlot;

    loggedIn = TRUE;
    setsysconfig();
    setgroupgen();
    setroomgen();
    setlbvisit();

    /* update25();	*/
	do_idle(0);

    sprintf(msgBuf->mbtext, "NetCallout %s", logBuf.lbname);
    trap(msgBuf->mbtext, T_NETWORK);

    /* node logged in */

    tmp = net_master();

    /* terminate user */

    if (tmp == TRUE) {
        logBuf.callno = cfg.callno;
        logtimestamp=cit_time();
        logBuf.calltime = logtimestamp;
        logBuf.lbvisit[0] = cfg.newest;
        logTab[0].ltcallno = cfg.callno;

        slideLTab(thisSlot);
        cfg.callno++;

        storeLog();
        loggedIn = FALSE;

    /* trap it */
        sprintf(msgBuf->mbtext, "NetLogout %s (succeeded)", logBuf.lbname);
        trap(msgBuf->mbtext, T_NETWORK);

        outFlag = IMPERVIOUS;
        cPrintf("Networked with \"%s\"\n ", logBuf.lbname);

        if (cfg.accounting)
            unlogthisAccount();
        heldMessage = FALSE;
        cleargroupgen();
        initroomgen();

        logBuf.lbname[0] = 0;

        setalloldrooms();
    } else {
        loggedIn = FALSE;

        sprintf(msgBuf->mbtext, "NetLogout %s (failed)", logBuf.lbname);
        trap(msgBuf->mbtext, T_NETWORK);
    }

    /* user terminated */
    onConsole = FALSE;
    callout = FALSE;

    delay(1000);

    Initport();

    local_mode = FALSE;
    return (BOOL) (tmp);
}

/* -------------------------------------------------------------------- */
/*  cleanup()       Done with other system, save mail and messages      */
/* -------------------------------------------------------------------- */
void cleanup(void)
{
    int t, i, rm, err;
    unsigned new = 0, exp = 0, dup = 0, rms = 0;
    label fn, here, there;

	if (parm.door)
		ExitToMsdos = TRUE;
	else
		drop_dtr();
    changedir(cfg.temppath);

    outFlag = IMPERVIOUS;

    doccr();
    cPrintf(" Incorporating:");
    doccr();
    cPrintf("                           Room:  New: Expired: Duplicate:");
    doccr();            /* XXXXXXXXXXXXXXXXXXXX    XX     XX     XX */
    cPrintf("อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ");
    doccr();
    for (t = get_first_room(here, there), i = 0;
        t;
    t = get_next_room(here, there), i++) {
        sprintf(fn, "roomin.%d", i);

        rm = roomExists(here);
        if (rm != ERROR) {
            cPrintf(" %*.*s  ", NAMESIZE, NAMESIZE, here);
            err = ReadMsgFl(rm, fn, here, there);
            if (err != ERROR) {
                cPrintf(" %3u    %3u    %3u", err, expired, duplicate);
                new += err;
                exp += expired;
                dup += duplicate;
                rms++;
            } else {
                amPrintf(" \n Can not see room '%s' on remote.", there);
                netError = TRUE;
                cPrintf(" Room not found on other system.");
            }
            doccr();
        } else {
            cPrintf(" %*.*s   Room not found on local system.",
            NAMESIZE, NAMESIZE, here);
            amPrintf(" \n No '%s' room accessable local.", here);
            netError = TRUE;
            doccr();
        }

        unlink(fn);
    }
    cPrintf("อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ");
    doccr();
    cPrintf("Totals:                      %2u    %2u     %2u     %2u",
		rms, new, exp, dup);
    /* XXXXXXXXXXXXXXXXXXXX    XX     XX     XX */
    doccr();
    doccr();

    cPrintf("Incorporating MAIL");
    i = ReadMsgFl(MAILROOM, "mesg.tmp", "", "");
    cPrintf(" %d new message(s)", i == ERROR ? 0 : i);
    doccr();

    ambigUnlink("room.[0-9]*", FALSE);
    ambigUnlink("roomin.*", FALSE);
    unlink("mesg.tmp");
    if (!strcmp(cfg.temppath, cfg.homepath))
        ambigUnlink("room.*", FALSE);

    xpd = exp;
    duplic = dup;
    entered = new + (i == ERROR ? 0 : i);   /* add mail too. */

    if (netError) {
        amPrintf(" \n \n While netting with '%s'\n", logBuf.lbname);
        SaveAideMess();
    }
}

/* -------------------------------------------------------------------- */
/*  netcanseeroom() Can the node see this room?                     (o) */
/* -------------------------------------------------------------------- */
BOOL netcanseeroom(int roomslot)
{
    /* is room in use              */
    if  (roomTab[roomslot].rtflags.INUSE

    /* and it is shared            */
        &&  roomTab[roomslot].rtflags.SHARED

    /* and group can see this room */
        &&  (groupseesroom(roomslot)
        ||  roomTab[roomslot].rtflags.READONLY
        ||  roomTab[roomslot].rtflags.DOWNONLY)
    /* only aides go to aide room  */
    &&  (roomslot != AIDEROOM || gl_user.aide)) {
        return TRUE;
    }
    return FALSE;
}

/* -------------------------------------------------------------------- */
/*  alias()         return the name of the BBS from the #ALIAS          */
/* -------------------------------------------------------------------- */
BOOL alias(char *str)
{
    return alias_route(str, "#ALIAS");
}

/* -------------------------------------------------------------------- */
/*  route()         return the routing of a BBS from the #ROUTE         */
/* -------------------------------------------------------------------- */
BOOL route(char *str)
{
    if  (alias_route(str, "#ROUTE"))
        return (TRUE);
    else
        return (alias_route(default_route, "#ROUTE"));
}

/* -------------------------------------------------------------------- */
/*  alias_route()   returns the route or alias specified                */
/* -------------------------------------------------------------------- */
BOOL alias_route(char *str, char *srch)
{
    FILE *fBuf;
    char line[90];
    char *words[256];
    char path[80];

    sprintf(path, "%s\\route.cit", cfg.homepath);

    if ((fBuf = fopen(path, "r")) == NULL) {
        crashout("Can't find route.cit!");
    }
    while (fgets(line, 90, fBuf) != NULL) {
        if (line[0] != '#')
            continue;
    /* silly way to do this, time to rewrite another module - FJM */
        else if (!strnicmp(line, "#DEFAULT_ROUTE", 14)) {
            parse_it(words, line);
            strcpy(default_route, words[1]);
            continue;
        } else if (strnicmp(line, srch, 5) != SAMESTRING)
            continue;

        parse_it(words, line);

        if (strcmpi(srch, words[0]) == SAMESTRING) {
            if (strcmpi(str, words[1]) == SAMESTRING) {
                fclose(fBuf);
                strcpy(str, words[2]);
                return TRUE;
            }
        }
    }
    fclose(fBuf);
    return FALSE;
}

/* ------------------------------------------------------------------------ */
/*  the following two routines are used for scanning through the rooms      */
/*  requested from a node                                                   */
/* ------------------------------------------------------------------------ */
FILE *nodefile;

/* -------------------------------------------------------------------- */
/*  get_first_room()    get the first room in the room list             */
/* -------------------------------------------------------------------- */
BOOL get_first_room(char *here, char *there)
{
    if  (!node.roomoff)
        return FALSE;

    /* move to home-path */
    changedir(cfg.homepath);

    if ((nodefile = fopen("nodes.cit", "r")) == NULL) {
        crashout("Can't find nodes.cit!");
    }
    changedir(cfg.temppath);

    fseek(nodefile, node.roomoff, SEEK_SET);

    return get_next_room(here, there);
}

/* -------------------------------------------------------------------- */
/*  get_next_room() gets the next room in the list                      */
/* -------------------------------------------------------------------- */
BOOL get_next_room(char *here, char *there)
{
    char line[90];
    char *words[256];

    while (fgets(line, 90, nodefile) != NULL) {
        if (line[0] != '#')
            continue;

        parse_it(words, line);

        if (strcmpi(words[0], "#NODE") == SAMESTRING) {
            fclose(nodefile);
            return FALSE;
        }
        if (strcmpi(words[0], "#ROOM") == SAMESTRING) {
            strcpy(here, words[1]);
            strcpy(there, words[2]);
            return TRUE;
        }
    }
    fclose(nodefile);
    return FALSE;
}

/* -------------------------------------------------------------------- */
/*  save_mail()     save a message bound for another system             */
/* -------------------------------------------------------------------- */
BOOL save_mail()
{
    label tosystem;
    char filename[100];
    char our_path[NAMESIZE + 2];
    FILE *fl;

    /* where are we sending it? */
    strncpy(tosystem, msgBuf->mbzip, sizeof(label) - 1);

    /* send it via... */
    route(tosystem);

    /* get the node entery */
    if (!getnode(tosystem)) {
        if (!*default_route || !getnode(default_route)) {
            amPrintf(" \n Default mail route not found.");
            netError = TRUE;
            return FALSE;
        } else {
            amPrintf(" \n Can't find mail route to %s,", tosystem);
            amPrintf(" defaulting mail route to %s.", default_route);
            netError = TRUE;
            strcpy(tosystem, default_route);
        }
    }
    /* check to see if this mail has been routed through here before */
    sprintf(our_path, "!%s!", cfg.nodeTitle);

    /* test for our node name (e.g. "!Fred's Toy!") */
    if (strstr(msgBuf->mbfpath, our_path)) {
        amPrintf(" \n Circular route found in mail message #%s.",msgBuf->mbId);
		amPrintf(" Path '%s'.", msgBuf->mbfpath);
        netError = TRUE;
        return FALSE;
    }
    sprintf(filename, "%s\\%s", cfg.homepath, node.ndmailtmp);

    fl = fopen(filename, "ab");
    if (!fl)
        return FALSE;

    fPutMessage(fl);

    fclose(fl);

    return TRUE;
}

/* --------------------------------------------------------------------
 *  verbose_response()	translate Hayes modem responses
 * -------------------------------------------------------------------- */

static int verbose_response(char *buf)
{
	int i;
	
	i = 0;
	if (debug)
		cPrintf("\n(decoding response '%s')\n",buf);
	while (stricmp(buf,hayes_string[i])) {
		++i;
		if (i >= HAYES_NUMCMDS) {
			if (debug)
				cPrintf("(response not found)\n",i);
			i = HAYES_UNKNOWN;
			break;
		}
	}
	return i;
}

/* --------------------------------------------------------------------
 *  PutStrNA()   puts a null-terminated string to a file, ANSI stripped
 * -------------------------------------------------------------------- */
static void PutStrNA(FILE * fp, char *str)
{
	stripANSI(str);
	PutStr(fp,str);
}
/* EOF */
