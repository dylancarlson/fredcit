/* -------------------------------------------------------------------- */
/*  MSG3.C                    Citadel                                   */
/* -------------------------------------------------------------------- */
/*                       Overlayed message code                         */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Includes                                                            */
/* -------------------------------------------------------------------- */
#include <string.h>
#include <stdarg.h>
#include "ctdl.h"
#include "proto.h"
#include "global.h"
#ifndef ATARI_ST
#include <alloc.h>
#endif

/* --------------------------------------------------------------------
                                Contents
   --------------------------------------------------------------------
    strblank()		check for blank string
    copymessage()   copies specified message # into specified room
    deleteMessage() deletes message for pullIt()
    insert()        aide fn: to insert a message
    makeMessage()   is menu-level routine to enter a message
    markIt()        is a sysop special to mark current message
    markmsg()       marks a message for insertion and/or visibility
    printMessage()  prints message on modem and console
    pullIt()        is a sysop special to remove a message from a room
    stepMessage()   find the next message in DIR
    showMessages()  is routine to print roomful of msgs
    printheader()   prints current message header
   -------------------------------------------------------------------- */


/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/MSG3.C_V  $
 * 
 *    Rev 1.47   01 Nov 1991 11:20:38   FJM
 * Added gl_ structures
 *
 *    Rev 1.46   06 Jun 1991  9:19:32   FJM
 *
 *    Rev 1.45   29 May 1991 11:23:42   FJM
 * Cleanup to strblank()
 *
 *    Rev 1.44   27 May 1991 11:43:12   FJM
 *
 *    Rev 1.43   22 May 1991  2:17:38   FJM
 *
 *    Rev 1.42   18 May 1991  9:32:06   FJM
 * Added strblank()
 *
 *    Rev 1.41   06 May 1991 17:29:22   FJM
 *    Rev 1.38   10 Apr 1991  9:05:52   FJM
 * Fixed ANSI after N on messages.
 *
 *    Rev 1.37   04 Apr 1991 14:13:48   FJM
 *    Rev 1.33   10 Feb 1991 17:33:50   FJM
 *    Rev 1.31   05 Feb 1991 14:32:04   FJM
 *    Rev 1.30   28 Jan 1991 13:12:44   FJM
 *    Rev 1.25   18 Jan 1991 16:51:42   FJM
 *    Rev 1.23   13 Jan 1991  0:32:08   FJM
 * Name overflow fixes.
 *
 *    Rev 1.17   06 Jan 1991 12:45:54   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.16   27 Dec 1990 20:16:14   FJM
 * Changes for porting.
 *
 *    Rev 1.13   22 Dec 1990 13:38:34   FJM
 *    Rev 1.5   24 Nov 1990  3:07:42   FJM
 * Changes for shell/door mode.
 *
 *    Rev 1.4   23 Nov 1990 13:25:14   FJM
 * Header change
 *
 *    Rev 1.2   17 Nov 1990 16:12:08   FJM
 * Added version control log header
 * --------------------------------------------------------------------
 *  EARLY HISTORY:
 *
 *  82Nov03      CrT    Individual history begun.  General cleanup.
 *  06/14/89    (PAT)   Created from MSG.C to move some of the system
 *                      out of memory.
 *  03/03/90    {zm}    Display '****' in header in anonymous rooms.
 *  03/07/90    {zm}    Do fake full-case based on a % of uppercase.
 *  03/15/90    {zm}    Add [title] name [surname], 30 characters long.
 *  03/16/90    {zm}    Add message route path to verbose header.
 *  04-04-90    {zm}    Add + and - to <Reversed> to show direction.
 *  05/17/90    FJM     Removed "Can't find message..." from modem outpt
 *  06/06/90    FJM     Changed strftime to cit_strftime
 *
 * -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  External data                                                       */
/* -------------------------------------------------------------------- */
extern label default_route;

/* --------------------------------------------------------------------
	strblank(str)   Check for blank string
   -------------------------------------------------------------------- */
int strblank(char *p)
{
	while (*p)
		if ((*p++ > '!') && ((unsigned)*p < 255))
			return 0;
	return 1;
}

/* -------------------------------------------------------------------- */
/*  copymessage()   copies specified message # into specified room      */
/* -------------------------------------------------------------------- */
void copymessage(ulong id, uchar roomno)
{
    unsigned char attr;
    label copy;
    int slot;

    slot = indexslot(id);

    /* load in message to be inserted */
    fseek(msgfl, msgTab[slot].mtmsgLoc, 0);
    getMessage();

    /* retain vital information */
    attr = msgBuf->mbattr;
    strcpy(copy, msgBuf->mbId);

    clearmsgbuf();
    msgBuf->mbtext[0] = '\0';

    strcpy(msgBuf->mbcopy, copy);
    msgBuf->mbattr = attr;
    msgBuf->mbroomno = roomno;

    putMessage();
    noteMessage();
}

/* -------------------------------------------------------------------- */
/*  deleteMessage() deletes message for pullIt()                        */
/* -------------------------------------------------------------------- */
void deleteMessage(void)
{
    ulong id;

    if (!copyflag)
        sscanf(msgBuf->mbId, "%lu", &id);
    else
        id = originalId;

    if (!(*msgBuf->mbx))
        markmsg();      /* Mark it for possible insertion elsewhere */

    changeheader(id, DUMP, 255);

    if (thisRoom != AIDEROOM && thisRoom != DUMP) {
    /* note in Aide): */
        sprintf(msgBuf->mbtext, "Message #%lu deleted by %s",
        id, logBuf.lbname);

        trap(msgBuf->mbtext, T_AIDE);

        aideMessage();

        copymessage(id, AIDEROOM);
        if (!logBuf.lbroom[AIDEROOM].lvisit)
            talleyBuf.room[AIDEROOM].new--;
    }
}

/* -------------------------------------------------------------------- */
/*  insert()        aide fn: to insert a message                        */
/* -------------------------------------------------------------------- */
void insert(void)
{
    if  (thisRoom == AIDEROOM || markedMId == 0l) {
        mPrintf("Not here.");
        return;
    }
    copymessage(markedMId, (uchar) thisRoom);

    sprintf(msgBuf->mbtext, "Message #%s inserted in %s> by %s",
    msgBuf->mbcopy, roomBuf.rbname, logBuf.lbname);

    trap(msgBuf->mbtext, T_AIDE);

    aideMessage();

    copymessage(markedMId, AIDEROOM);
    if (!logBuf.lbroom[AIDEROOM].lvisit)
        talleyBuf.room[AIDEROOM].new--;
}

/* -------------------------------------------------------------------- */
/*  makeMessage()   is menu-level routine to enter a message            */
/* -------------------------------------------------------------------- */
BOOL makeMessage(void)
{
    int i;

    /* take out char *pc, allUpper; */
    int logNo, logNo2;
    char recipient[NAMESIZE + NAMESIZE + 1];
    char rnode[NAMESIZE + NAMESIZE + 1];
    label forward;
    label groupname;
    int groupslot;
    label replyId;
    char filelink[64];
    struct logBuffer *lBuf2;

    if (oldFlag && heldMessage)
        memcpy(msgBuf, msgBuf2, sizeof(struct msgB));

    *recipient = '\0';
    *forward = '\0';
    *filelink = '\0';
    *rnode = '\0';

    /* limited-access message, ask for group name */
    if (limitFlag) {
        getString("group", groupname, NAMESIZE, FALSE, ECHO, "");

        groupslot = groupexists(groupname);
        if (groupslot == ERROR)
            groupslot = partialgroup(groupname);

        if (groupslot == ERROR || !ingroup(groupslot)) {
            mPrintf("\n No such group.");
            return FALSE;
        }
    /* makes it look prettier */
        strcpy(groupname, grpBuf.group[groupslot].groupname);
    }
    /* if replying, author becomes recipient */
    /* also record message which is being replied to */
    if (replyFlag) {
        strcpy(recipient, msgBuf->mbauth);
        strcpy(replyId, *msgBuf->mbsrcId ? msgBuf->mbsrcId : msgBuf->mbId);
        strcpy(rnode, msgBuf->mboname);
    }
    /* clear message buffer 'cept when entering old message */
    if (!oldFlag)
        setmem(msgBuf, sizeof(struct msgB), 0);

    /* user not logged in, sending exclusive to sysop */
    if (mailFlag && !replyFlag && !loggedIn) {
        doCR();
        mPrintf(" Private mail to 'Sysop'");
        strcpy(recipient, "Sysop");
    }
    /* sending exclusive mail which is not a reply */
    if (mailFlag && !replyFlag && loggedIn) {
        getNormStr("recipient", recipient, NAMESIZE + NAMESIZE + 1, ECHO);
        if (!strlen(recipient))
            return FALSE;
        if (strchr(recipient, '@'))
            strcpy(rnode, strchr(recipient, '@'));
        if (*rnode) {
            rnode[0] = ' ';

            normalizeString(rnode);

            if (strlen(rnode) > NAMESIZE) {
                mPrintf("\n No node %s.\n", rnode);
                return FALSE;
            }
            for (i = 0; i < NAMESIZE + NAMESIZE; i++)
                if (recipient[i] == '@')
                    recipient[i] = '\0';

            normalizeString(recipient);

            if (strlen(recipient) > NAMESIZE) {
                mPrintf("\n User's name is too long!\n");
                return FALSE;
            }
        }
    }
    if (mailFlag) {
        if (*rnode)
            alias(rnode);

        logNo = findPerson(*rnode ? rnode : recipient, &lBuf);

        if ((logNo != ERROR) && *rnode) {
            if (!lBuf.lbflags.NODE)
                logNo = ERROR;
        }
        if ((logNo != ERROR) && lBuf.lbflags.NODE && !rnode[0]) {
            mPrintf(" Message forwarded to Sysop on %s\n", recipient);
            strcpy(rnode, recipient);
            strcpy(recipient, "SysOp");
        }
        if ((logNo != ERROR) && lBuf.forward[0]) {
			if ((lBuf2 = farcalloc(1, sizeof(struct logBuffer))) == NULL) {
				crashout("Can not allocate memory in makeMessage()");
			}
            mPrintf(" Message forwarded to ");

            logNo2 = findPerson(lBuf.forward, lBuf2);

            if (logNo2 != ERROR) {
                mPrintf("%s", lBuf2->lbname);
                strcpy(forward, lBuf2->lbname);
            }
            doCR();
			if (lBuf2)
				farfree(lBuf2);
			lBuf2 = NULL;
        }
        if ((logNo == ERROR) && (hash(recipient) != hash("Sysop"))
        && (hash(recipient) != hash("Aide"))) {
            if (*rnode) {
                label temp;

                strcpy(temp, rnode);
                route(temp);
                if (!getnode(temp)) {
                    if (!getnode(default_route)) {
                        mtPrintf(TERM_BOLD,"Don't know how to reach '%s'.\n", rnode);
                        return FALSE;
                    } else {
                        mtPrintf(TERM_BOLD,
							"Don't know how to reach '%s', routing via '%s'.\n",
							rnode,default_route);
						if (!getYesNo(confirm,1))
							return FALSE;
					}
                }
            } else {
                mPrintf("No '%s' known", recipient);
                return FALSE;
            }
        }
    }
    if (linkMess) {
        getNormStr("file", filelink, 64, ECHO);
        if (!strlen(filelink))
            return FALSE;
    }
    /* copy groupname into the message buffer */
    if (limitFlag) {
        strcpy(msgBuf->mbgroup, groupname);
    }
    if (*rnode) {
        strcpy(msgBuf->mbzip, rnode);
    }
    /* moderated messages */
    if ((roomBuf.rbflags.MODERATED
        || (roomTab[thisRoom].rtflags.SHARED && !logBuf.lbflags.NETUSER))
    && !mailFlag) {
        strcpy(msgBuf->mbx, "M");
    }
    /* problem user message */
    if (gl_user.twit && !mailFlag) {
        strcpy(msgBuf->mbx, "Y");
    }
    /* copy message Id of message being replied to */
    if (replyFlag) {
        strcpy(msgBuf->mbreply, replyId);
    }
    /* finally it's time to copy recipient to message buffer */
    if (*recipient) {
        strcpy(msgBuf->mbto, recipient);
    } else {
        msgBuf->mbto[0] = '\0';
    }

    /* finally it's time to copy forward addressee to message buffer */
    if (*forward) {
        strcpy(msgBuf->mbfwd, forward);
    } else {
        msgBuf->mbfwd[0] = '\0';
    }

    if (*filelink) {
        strcpy(msgBuf->mblink, filelink);
    } else {
        msgBuf->mblink[0] = '\0';
    }

    /* let's handle .Enter old-message */
    if (oldFlag) {
        if (!heldMessage) {
            mPrintf("\n No aborted message\n ");
            return FALSE;
        } else {
            if (!getYesNo("Use aborted message", 1))
        /* clear only the text portion of message buffer */
                setmem(msgBuf->mbtext, sizeof msgBuf->mbtext, 0);
        }
    }
    /* clear our flags */
    heldMessage = FALSE;

    /* copy author name into message buffer */
    if (loggedIn) {
        strcpy(msgBuf->mbauth, logBuf.lbname);
        strcpy(msgBuf->mbtitle, logBuf.title);
        strcpy(msgBuf->mbsur, logBuf.surname);
    }
    /* set room# and attribute byte for message */
    msgBuf->mbroomno = (uchar) thisRoom;
    msgBuf->mbattr = 0;

    if (!linkMess) {
        if (getText() == TRUE) {
/* -------------------------------------------------------------------- */
/*  we use the Bunny Filter whenever more than 90 percent               */
/* (or whatever percent you wish) of the message is uppercase.          */
/* -------------------------------------------------------------------- */
            if (ismostlyupper(msgBuf->mbtext, 90))
                fakeFullCase(msgBuf->mbtext);

            sprintf(msgBuf->mbId, "%lu", (unsigned long) (cfg.newest + 1));

            if (*msgBuf->mbzip) /* save it for netting... */
                save_mail();

            strcpy(msgBuf->mboname, "");
            strcpy(msgBuf->mboreg, "");

            if (gl_user.sysop && msgBuf->mbx[0] == 'M') {
                if (getYesNo("Release message", 1)) {
                    msgBuf->mbx[0] = EOS;
                }
            }
            putMessage();

            if (!replyFlag)
                MessageRoom[msgBuf->mbroomno]++;

            noteMessage();
            limitFlag = 0;  /* keeps Aide) messages from being group-only */
        /* if it's mail, note it in recipient's log entry */
            if (mailFlag)
                notelogmessage(msgBuf->mbto);
        /* note in forwarding addressee's log entry */
            if (*forward)
                notelogmessage(msgBuf->mbfwd);

            msgBuf->mbto[0] = '\0';
            msgBuf->mbgroup[0] = '\0';
            msgBuf->mbfwd[0] = '\0';

            oldFlag = FALSE;

            return TRUE;
        }
    } else {
        msgBuf->mbtext[0] = '\0';

        sprintf(msgBuf->mbId, "%lu", (unsigned long) (cfg.newest + 1));

        putMessage();

        if (!replyFlag)
            MessageRoom[msgBuf->mbroomno]++;

        noteMessage();
        limitFlag = 0;      /* keeps Aide) messages from being group-only */
    /* if it's mail, note it in recipient's log entry */
        if (mailFlag)
            notelogmessage(msgBuf->mbto);
    /* note in forwarding addressee's log entry */
        if (*forward)
            notelogmessage(msgBuf->mbfwd);

        msgBuf->mbto[0] = '\0';
        msgBuf->mbgroup[0] = '\0';
        msgBuf->mbfwd[0] = '\0';
        msgBuf->mblink[0] = '\0';

        return TRUE;
    }
    oldFlag = FALSE;
    return FALSE;
}

/* -------------------------------------------------------------------- */
/*  markIt()        is a sysop special to mark current message          */
/* -------------------------------------------------------------------- */
BOOL markIt(void)
{
    ulong id;

    sscanf(msgBuf->mbId, "%lu", &id);

    /* confirm that we're marking the right one: */
    outFlag = OUTOK;
    printMessage(id, (char) 0);

    outFlag = OUTOK;

    doCR();

    if (getYesNo("Mark", 1)) {
        markmsg();
        return TRUE;
    } else
        return FALSE;
}

/* -------------------------------------------------------------------- */
/*  markmsg()       marks a message for insertion and/or visibility     */
/* -------------------------------------------------------------------- */
void markmsg(void)
{
    ulong id;
    uchar attr;

    sscanf(msgBuf->mbId, "%lu", &markedMId);

    if (!copyflag)
        id = markedMId;
    else
        id = originalId;

    if (msgBuf->mbx[0]) {
        if (!copyflag)
            attr = msgBuf->mbattr;
        else
            attr = originalattr;

        attr = (attr ^ ATTR_MADEVIS);

        if (!copyflag)
            msgBuf->mbattr = attr;
        else
            originalattr = attr;

        changeheader(id, 255, attr);

        if (attr & ATTR_MADEVIS)
            copymessage(id, (uchar) thisRoom);
    }
}

/* -------------------------------------------------------------------- */
/*  printMessage()  prints message on modem and console                 */
/* -------------------------------------------------------------------- */
#define msgstuff  msgTab[slot].mtmsgflags

void printMessage(ulong id, char verbose)
{
    char moreFollows;
    ulong here;
    long loc;
    int strip;
    int slot;
    static level = 0;

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

        originalattr = (uchar)
        (originalattr | (msgstuff.MADEVIS) ? ATTR_MADEVIS : 0);

        level++;

        if (level > 20) {
            level = 0;
            return;
        }
        if (msgTab[slot].mtoffset <= slot)
            printMessage((ulong) (id - (ulong) msgTab[slot].mtoffset), verbose);

        level--;

        return;
    }
    /* in case it returns without clearing buffer */
    msgBuf->mbfwd[0] = '\0';
    msgBuf->mbto[0] = '\0';

    loc = msgTab[slot].mtmsgLoc;
    if (loc == ERROR)
        return;

    if (copyflag)
        slot = indexslot(originalId);

    if (!mayseeindexmsg(slot))
        return;

    fseek(msgfl, loc, 0);

    getMessage();

    dotoMessage = NO_SPECIAL;

    sscanf(msgBuf->mbId, "%lu", &here);

    /* cludge to return on dummy msg #1 */
    if ((int) here == 1)
        return;

    if (!mayseemsg())
        return;

    mread++;            /* Increment # messages read */

    if (here != id) {
		if (debug)
			cPrintf("Can't find message. Looking for %lu at byte %ld!\n ",
        id, loc);
        return;
    }
    printheader(id, verbose, slot);

    seen = TRUE;

    if (msgBuf->mblink[0]) {
        dumpf(msgBuf->mblink);
    } else {
        while (TRUE) {
            moreFollows = dGetWord(msgBuf->mbtext, 150);

			/* strip control Ls out of the output                   */
            for (strip = 0; msgBuf->mbtext[strip] != 0; strip++) {
                if (msgBuf->mbtext[strip] == 0x0c ||
                    msgBuf->mbtext[strip] == 27 /* SPECIAL */ )
                    msgBuf->mbtext[strip] = 0x00;   /* Null NOT space.. */
            }

            putWord(msgBuf->mbtext);

            if (!(moreFollows && !mAbort())) {
                if (outFlag == OUTNEXT)
                    doCR(); /* If <N>ext, extra line */
                break;
            }
        }
    }
    termCap(TERM_NORMAL);
    doCR();
    echo = BOTH;
}

/* -------------------------------------------------------------------- */
/*  pullIt()        is a sysop special to remove a message from a room  */
/* -------------------------------------------------------------------- */
BOOL pullIt(void)
{
    ulong id;

    sscanf(msgBuf->mbId, "%lu", &id);

    /* confirm that we're removing the right one: */
    outFlag = OUTOK;

    printMessage(id, (char) 0);

    outFlag = OUTOK;

    doCR();

    if (getYesNo("Pull", 0)) {
        deleteMessage();
        return TRUE;
    } else
        return FALSE;
}

/* -------------------------------------------------------------------- */
/*  stepMessage()   find the next message in DIR                        */
/* -------------------------------------------------------------------- */
BOOL stepMessage(ulong * at, int dir)
{
    int i;

    for (i = indexslot(*at), i += dir; i > -1 && i < sizetable(); i += dir) {
    /* skip messages not in this room */
        if (msgTab[i].mtroomno != (uchar) thisRoom)
            continue;

    /* skip by special flag */
        if (mf.mfMai && !msgTab[i].mtmsgflags.MAIL)
            continue;
        if (mf.mfLim && !msgTab[i].mtmsgflags.LIMITED)
            continue;
        if (mf.mfPub &&
            (msgTab[i].mtmsgflags.LIMITED || msgTab[i].mtmsgflags.MAIL))
            continue;

        if (mayseeindexmsg(i)) {
            *at = (ulong) (cfg.mtoldest + i);
            return TRUE;
        }
    }
    return FALSE;
}

/* -------------------------------------------------------------------- */
/*  showMessages()  is routine to print roomful of msgs                 */
/* -------------------------------------------------------------------- */
void showMessages(char whichMess, char revOrder, char verbose)
{
    int increment;
    ulong lowLim, highLim, msgNo, start;
    unsigned char attr;
    BOOL done;
    char savepath[64];

	termCap(TERM_NORMAL);
    if (mf.mfLim) {
        getgroup();
        if (!mf.mfLim)
            return;
    } else {
        doCR();
    }

    if (mf.mfUser[0])
        getNormStr("user", mf.mfUser, NAMESIZE, ECHO);

    outFlag = OUTOK;

    if (!gl_user.expert)
        mPrintf("\n <J>ump <N>ext <P>ause <S>top");

    switch (whichMess) {
        case NEWoNLY:
            lowLim = logBuf.lbvisit[logBuf.lbroom[thisRoom].lvisit] + 1;
            highLim = cfg.newest;

    /* print out last new message */
            if (!revOrder && gl_user.oldToo && (highLim >= lowLim))
                stepMessage(&lowLim, -1);
            break;

        case OLDaNDnEW:
            lowLim = cfg.oldest;
            highLim = cfg.newest;
            break;

        case OLDoNLY:
            lowLim = cfg.oldest;
            highLim = logBuf.lbvisit[logBuf.lbroom[thisRoom].lvisit];
            break;
    }

    /* stuff may have scrolled off system unseen, so: */
    if (cfg.oldest > lowLim)
        lowLim = cfg.oldest;

    /* Allow for reverse retrieval: */
    if (!revOrder) {
        start = lowLim;
        increment = 1;
    } else {
        start = highLim;
        increment = -1;
    }

    start -= (long) increment;
    done = (BOOL) (!stepMessage(&start, increment));

    for (msgNo = start;
        !done
        && msgNo >= lowLim
        && msgNo <= highLim
        && (!ExitToMsdos && (haveCarrier || onConsole));
		done = (BOOL) (!stepMessage(&msgNo, increment))) {

        if (BBSCharReady())
            mAbort();

        if (outFlag != OUTOK) {
            if (outFlag == OUTNEXT || outFlag == OUTPARAGRAPH)
                outFlag = OUTOK;
            else if (outFlag == OUTSKIP) {
				termCap(TERM_NORMAL);
                echo = BOTH;
                mf.mfPub = FALSE;
                mf.mfMai = FALSE;
                mf.mfLim = FALSE;
                mf.mfUser[0] = FALSE;
                mf.mfGroup[0] = FALSE;
                outFlag = OUTOK;
                return;
            }
        }
        seen = FALSE;

        printMessage(msgNo, verbose);

        switch (dotoMessage) {
            case COPY_IT:
                if (!gl_user.sysop || !onConsole)
                    break;
                getNormStr("save path", savepath, 64, ECHO);
                if (*savepath)
					saveMessageText(savepath);
                outFlag = OUTOK;
                break;
            case PULL_IT:
				/* Pull current message from room if flag set */
                pullIt();
                outFlag = OUTOK;
                break;

            case MARK_IT:
				/* Mark current message from room if flag set */
                markIt();
                outFlag = OUTOK;
                break;

            case REVERSE_READ:
                increment = (increment == 1) ? -1 : 1;
                doCR();
                mPrintf("<Reversed %c>", (increment == 1) ? '+' : '-');
                doCR();
                break;

            case NO_SPECIAL:

        /* Release (Y/N)[N] */
                if (*msgBuf->mbx && gl_user.aide && seen
                    && (msgBuf->mbattr & ATTR_MADEVIS) != ATTR_MADEVIS
                    && outFlag == OUTOK)
                    if (getYesNo("Release", 0)) {
                        markmsg();
                        outFlag = OUTOK;
                    }
        /* reply to mail */
                if (whichMess == NEWoNLY
                    && (strcmpi(msgBuf->mbto, logBuf.lbname) == SAMESTRING
                    || strcmpi(msgBuf->mbfwd, logBuf.lbname) == SAMESTRING)
                && loggedIn) {
                    outFlag = OUTOK;
                    doCR();
                    if (getYesNo("Respond", 1)) {
                        replyFlag = 1;
                        mailFlag = 1;
                        linkMess = FALSE;

                        if (!copyflag)
                            attr = msgBuf->mbattr;
                        else
                            attr = originalattr;

                        if (whichIO != CONSOLE)
                            echo = CALLER;

                        if (makeMessage()) {
                            attr = (attr | ATTR_REPLY);

                            if (!copyflag)
                                msgBuf->mbattr = attr;
                            else
                                originalattr = attr;

                            if (!copyflag)
                                changeheader(msgNo, 255, attr);
                            else
                                changeheader(originalId, 255, attr);
                        }
                        replyFlag = 0;
                        mailFlag = 0;

            /* Restore privacy zapped by make... */
                        if (whichIO != CONSOLE)
                            echo = BOTH;

                        outFlag = OUTOK;

                        if (cfg.oldest > lowLim) {
                            lowLim = cfg.oldest;
                            if (msgNo < lowLim)
                                msgNo = lowLim;
                        }
                    }
                }
                break;

            default:
                mPrintf("Showmess(), dotoMessage == BAD_VALUE\n");
                break;
        }

        copyflag = FALSE;
        originalId = 0;
        originalattr = 0;
    }
    echo = BOTH;
    mf.mfPub = FALSE;
    mf.mfMai = FALSE;
    mf.mfLim = FALSE;
    mf.mfUser[0] = FALSE;
    mf.mfGroup[0] = FALSE;
}

/* --------------------------------------------------------------------
 * saveMessageText(filename) - saves current message buffer to named file
 * need enhancement - no header saved.
 * -------------------------------------------------------------------- */

void saveMessageText(char *filename)
{
	FILE *fp;
	fp = fopen(filename,"ab");
	if (!strblank(msgBuf->mbtext)) {
		if (fp) {
			if (!fwrite(msgBuf->mbtext,strlen(msgBuf->mbtext),1,fp))
				mPrintf("Can not write to file %s\n",filename);
			fclose(fp);
		} else {
			mPrintf("Can not save to file %s\n",filename);
		}
	} else {
		mPrintf("Message is blank\n");
	}
	/* save msgBuf->mbtext to file */
}

/* -------------------------------------------------------------------- */
/*  printheader()   prints current message header                       */
/* -------------------------------------------------------------------- */
void printheader(ulong id, char verbose, int slot)
{
    char dtstr[80];
    uchar attr;
    long timestamp;

    if (outFlag == OUTNEXT)
        outFlag = OUTOK;

    if (*msgBuf->mbtime) {
        sscanf(msgBuf->mbtime, "%ld", &timestamp);
        cit_strftime(dtstr, 79,
        verbose ? cfg.vdatestamp : cfg.datestamp, timestamp);
    }
    if (*msgBuf->mbto && whichIO != CONSOLE)
        echo = CALLER;

    doCR();
    termCap(TERM_BOLD);

    if (roomBuf.rbflags.ANONYMOUS &&
		!(gl_user.sysop && verbose) && !msgBuf->mbto[0]) {
        mPrintf("    ****");

        if (msgBuf->mbgroup[0]) {
            mPrintf(" (%s only)", msgBuf->mbgroup);
        }
        if ((gl_user.aide || gl_user.sysop) && msgBuf->mbx[0]) {
            if (!msgstuff.MADEVIS) {
                if (msgBuf->mbx[0] == 'Y')
                    mPrintf(" [problem user]");
                else
                    mPrintf(" [moderated]");
            } else
                mPrintf(" [viewable %s]", msgBuf->mbx[0] == 'Y' ?
                "problem user" : "moderated");
        }
        doCR();
    } else {            /* not anonymous, so display it all... */
        if (verbose) {
            mPrintf("    # %s of %lu", msgBuf->mbId, cfg.newest);
            if (copyflag && gl_user.aide)
                mPrintf(" (Duplicate id # %lu)", originalId);
            if (*msgBuf->mbsrcId) {
                doCR();
                mPrintf("    Source id #%s", msgBuf->mbsrcId);
            }
            if (*msgBuf->mbfpath) {
                doCR();
                mPrintf("    Path followed: %s", msgBuf->mbfpath);
            }
            if (*msgBuf->mblink && gl_user.sysop) {
                doCR();
                mPrintf("    Linked file is %s", msgBuf->mblink);
            }
            doCR();
        }
        mPrintf("    %s ", dtstr);
        if (msgBuf->mbauth[0]) {
			mPrintf("From ");
            if (msgBuf->mbtitle[0]
                && ((cfg.surnames && !(msgBuf->mboname[0]))
				|| cfg.netsurname)) {
                mPrintf("[%s] ", msgBuf->mbtitle);
            }
            mPrintf("%s", msgBuf->mbauth);
            if (msgBuf->mbsur[0]
                && ((cfg.surnames && !(msgBuf->mboname[0]))
				|| cfg.netsurname)) {
                mPrintf(" [%s]", msgBuf->mbsur);
            }
			/* reset colors */
			termCap(TERM_NORMAL);
			termCap(TERM_BOLD);
        }
        if (msgBuf->mboname[0]
            && (strcmpi(msgBuf->mboname, cfg.nodeTitle) != SAMESTRING
            || strcmpi(msgBuf->mboreg, cfg.nodeRegion) != SAMESTRING)
            && strcmpi(msgBuf->mbauth, msgBuf->mboname) != SAMESTRING)
            mPrintf(" @ %s", msgBuf->mboname);

        if (msgBuf->mboreg[0] &&
            strcmpi(msgBuf->mboreg, cfg.nodeRegion) != SAMESTRING)
            mPrintf(", %s", msgBuf->mboreg);

        if (msgBuf->mbto[0]) {
            mPrintf(" To %s", msgBuf->mbto);

            if (msgBuf->mbzip[0]
                && strcmpi(msgBuf->mbzip, cfg.nodeTitle) != SAMESTRING)
                mPrintf(" @ %s", msgBuf->mbzip);

            if (msgBuf->mbrzip[0] &&
                strcmpi(msgBuf->mbrzip, cfg.nodeRegion))
                mPrintf(", %s", msgBuf->mbrzip);

            if (msgBuf->mbfwd[0])
                mPrintf(" Forwarded to %s", msgBuf->mbfwd);

            if (msgBuf->mbreply[0]) {
                if (verbose)
                    mPrintf(" [reply to %s]", msgBuf->mbreply);
                else
                    mPrintf(" [reply]");
            }
            if (msgstuff.RECEIVED)
                mPrintf(" [received]");
            if (msgstuff.REPLY)
                mPrintf(" [reply sent]");

            if ((msgBuf->mbto[0])
            && !(strcmpi(msgBuf->mbauth, logBuf.lbname) == SAMESTRING)) {

                if (!copyflag)
                    attr = msgBuf->mbattr;
                else
                    attr = originalattr;

                if (!(attr & ATTR_RECEIVED)) {
                    attr = (attr | ATTR_RECEIVED);

                    if (!copyflag)
                        msgBuf->mbattr = attr;
                    else
                        originalattr = attr;

                    if (!copyflag)
                        changeheader(id, 255, attr);
                    else
                        changeheader(originalId, 255, attr);

                }
            }
        }
/* -------------------------------------------------------------------- */
/*  bug-fix (kludge) for the "In >" in all netmail headers...           */
/*  i.e. don't do it for roomname = null and room = mailroom.           */
/* -------------------------------------------------------------------- */

        if (strcmpi(msgBuf->mbroom, roomBuf.rbname) != SAMESTRING &&
        (msgBuf->mbroom[0] || thisRoom != MAILROOM)) {
            mPrintf(" In %s>", msgBuf->mbroom);
        }
        if (msgBuf->mbgroup[0]) {
            mPrintf(" (%s only)", msgBuf->mbgroup);
        }
        if ((gl_user.aide || gl_user.sysop) && msgBuf->mbx[0]) {
            if (!msgstuff.MADEVIS) {
                if (msgBuf->mbx[0] == 'Y')
                    mPrintf(" [problem user]");
                else
                    mPrintf(" [moderated]");
            } else
                mPrintf(" [viewable %s]", msgBuf->mbx[0] == 'Y' ?
                "problem user" : "moderated");
        }
        if ((gl_user.aide || gl_user.sysop) && msgBuf->mblink[0])
            mPrintf(" [file-linked]");

        doCR();
    }
    termCap(TERM_NORMAL);
}

/* EOF */
