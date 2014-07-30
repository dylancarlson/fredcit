/* $Header:   D:/VCS/FCIT/ROOMB.C_V   1.42   01 Nov 1991 11:20:50   FJM  $ */
/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/ROOMB.C_V  $
 * 
 *    Rev 1.42   01 Nov 1991 11:20:50   FJM
 * Added gl_ structures
 *
 *    Rev 1.41   06 Jun 1991  9:19:48   FJM
 *
 *    Rev 1.40   27 May 1991 11:43:20   FJM
 *
 *    Rev 1.29   28 Jan 1991 13:13:08   FJM
 *    Rev 1.24   18 Jan 1991 16:52:22   FJM
 *    Rev 1.16   06 Jan 1991 12:46:06   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.15   27 Dec 1990 20:16:38   FJM
 * Changes for porting.
 *
 *    Rev 1.12   22 Dec 1990 13:39:08   FJM
 *
 *    Rev 1.4   23 Nov 1990 13:25:24   FJM
 * Header change
 *
 *    Rev 1.2   17 Nov 1990 16:12:12   FJM
 * Added version control log header
 * -------------------------------------------------------------------- */

/************************************************************************/
/*                              roomb.c                                 */
/*              room code for Citadel bulletin board system             */
/************************************************************************/

#include <string.h>
#include "ctdl.h"

#ifndef ATARI_ST
#include <dir.h>
#endif

#include "proto.h"
#include "global.h"

/************************************************************************/
/*                              Contents                                */
/*                                                                      */
/*      findRoom()              find a free room                        */
/*      formatSummary()         summarizes current room                 */
/*      givePrompt()            gives usual "THISROOM>" prompt          */
/*      indexRooms()            build RAM index to room.cit             */
/*      killempties()           deletes empty rooms                     */
/*      killroom()              aide fn: to kill current room           */
/*      makeRoom()              make new room via user dialogue         */
/*      massdelete()            sysop fn: to kill all msgs from person  */
/*      noteRoom()              enter room into RAM index               */
/*      readbymsgno()           sysop fn: to read by message #          */
/*      renameRoom()            sysop special to rename rooms           */
/*      stepRoom()              goes to next or previous room           */
/************************************************************************/

/************************************************************************/
/*  History since 3.11.05d                                              */
/*                                                                      */
/*  02/06/90  {zm}  Add code for anonymous rooms.                       */
/*  03/15/90  {zm}  Allow buffers to not overflow if name = 30 chars.   */
/*                                                                      */
/*     08/07/90        FJM             Made newroom blurb rotate.                                              */
/************************************************************************/

static int directory_l(char *str);

/************************************************************************/
/*      findRoom() returns # of free room if possible, else ERROR       */
/************************************************************************/
int findRoom(void)
{
    int roomRover;

    for (roomRover = 0; roomRover < MAXROOMS; roomRover++) {
        if (roomTab[roomRover].rtflags.INUSE == 0)
            return roomRover;
    }
    return ERROR;
}

/************************************************************************/
/*      formatSummary() formats a summary of the current room           */
/************************************************************************/
void formatSummary(char *buffer)
{
    char line[150];

    sprintf(line, " Room %s", roomBuf.rbname);

    strcpy(buffer, line);

    if (roomBuf.rbflags.GROUPONLY) {
        sprintf(line, ", owned by group %s",
        grpBuf.group[roomBuf.rbgrpno].groupname);

        strcat(buffer, line);
    }
    if (!roomBuf.rbflags.PUBLIC) {
        strcat(buffer, ", hidden");
    }
    if (roomBuf.rbflags.MODERATED) {
        strcat(buffer, ", moderated");
    }
    if (roomBuf.rbflags.ANONYMOUS) {
        strcat(buffer, ", anonymous");
    }
    if (roomBuf.rbflags.READONLY) {
        strcat(buffer, ", read-only");
    }
    if (roomBuf.rbflags.DOWNONLY) {
        strcat(buffer, ", download only");
    }
    if (roomBuf.rbflags.SHARED) {
        strcat(buffer, ", networked/shared");
    }
    if (roomBuf.rbflags.APLIC) {
        strcat(buffer, ", application");
    }
    if (roomBuf.rbflags.PERMROOM) {
        strcat(buffer, ", permanent room");
    }
    if (gl_user.aide) {
        if (roomBuf.rbflags.MSDOSDIR) {
            sprintf(line, "\n Directory room:  path is %s", roomBuf.rbdirname);
            strcat(buffer, line);
        }
    }
    if (gl_user.sysop && roomBuf.rbflags.APLIC) {
        sprintf(line, "\n Application is %s", roomBuf.rbaplic);
        strcat(buffer, line);
    }
    if (roomBuf.rbroomtell[0] && cfg.roomtell && gl_user.sysop) {
        sprintf(line, "\n Room description file is %s", roomBuf.rbroomtell);
        strcat(buffer, line);
    }
    if (roomBuf.descript[0]) {
        sprintf(line, "\n Room Info-line is: %s", roomBuf.descript);
        strcat(buffer, line);
    }
}

/************************************************************************/
/*      makeRoom() constructs a new room via dialogue with user.        */
/************************************************************************/
void makeRoom(void)
{
    char roomname[NAMESIZE + 1];
    char oldName[NAMESIZE + 1];
    char groupname[NAMESIZE + 1];
    int groupslot, testslot;
    char line[80];
    int i;

    logBuf.lbroom[thisRoom].lbgen = roomBuf.rbgen;
    logBuf.lbroom[thisRoom].lvisit = 0;

    /* zero new count in talleybuffer */
    talleyBuf.room[thisRoom].new = 0;

    strcpy(oldName, roomBuf.rbname);

    thisRoom = findRoom();

    if ((thisRoom) == ERROR) {
        indexRooms();       /* try to reclaim empty room */

        thisRoom = findRoom();

        if (thisRoom == ERROR) {
            mPrintf(" Room table full.");

            thisRoom = roomExists(oldName);

        /* room is missing, go back to Lobby>    */
            if ((thisRoom) == ERROR)
                thisRoom = LOBBY;

            getRoom(thisRoom);  /* room is gone, go back to Lobby> */

            return;
        }
    }
    getNormStr("name for new room", roomname, NAMESIZE, ECHO);

    if (!strlen(roomname)) {
        thisRoom = roomExists(oldName);

    /* room is missing, go back to Lobby>    */
        if ((thisRoom) == ERROR)
            thisRoom = LOBBY;

        getRoom(thisRoom);  /* room is gone, go back to Lobby> */

        return;
    }
    testslot = roomExists(roomname);

    if (testslot != ERROR) {
        mPrintf(" A \"%s\" room already exists.\n", roomname);

        thisRoom = roomExists(oldName);

    /* room is missing, go back to Lobby>    */
        if ((thisRoom) == ERROR)
            thisRoom = LOBBY;

        getRoom(thisRoom);  /* room is gone, go back to Lobby> */

        return;
    }
    if (limitFlag) {
        getString("group for new room", groupname, NAMESIZE, FALSE, ECHO, "");

        groupslot = groupexists(groupname);
        if (groupslot == ERROR)
            groupslot = partialgroup(groupname);

        if (!strlen(groupname) || (groupslot == ERROR)
        || !ingroup(groupslot)) {
            mPrintf("No such group.");

            thisRoom = roomExists(oldName);

        /* room is missing, go back to Lobby>    */
            if ((thisRoom) == ERROR)
                thisRoom = LOBBY;

            getRoom(thisRoom);  /* room is gone, go back to Lobby> */

            return;
        }
        roomBuf.rbgrpno = (unsigned char) groupslot;
        roomBuf.rbgrpgen = grpBuf.group[groupslot].groupgen;

        roomBuf.rbflags.READONLY = getYesNo("Read-only", 0);
    }
    if (!gl_user.expert)
        nextblurb("newroom", &(cfg.cnt.newroomcount), 1);

    roomBuf.rbflags.APLIC = FALSE;
    roomBuf.rbaplic[0] = '\0';
    roomBuf.rbflags.INUSE = TRUE;
    roomBuf.rbflags.PERMROOM = FALSE;
    roomBuf.rbflags.MSDOSDIR = FALSE;
    roomBuf.rbflags.SHARED = FALSE;
    roomBuf.rbflags.MODERATED = FALSE;
    roomBuf.rbflags.ANONYMOUS = FALSE;
    roomBuf.rbflags.APLIC = FALSE;
    roomBuf.rbflags.GROUPONLY = limitFlag;
    roomBuf.rbroomtell[0] = '\0';

    getNormStr("Description for new room", roomBuf.descript, 80, ECHO);

    roomBuf.rbflags.PUBLIC = getYesNo("Make room public", 1);

    sprintf(line, "Install \"%s\" as a %s room",
    roomname, (roomBuf.rbflags.PUBLIC) ? "public" : "private", 0);

    if (!getYesNo(line, 0)) {
        thisRoom = roomExists(oldName);

    /* room is missing, go back to Lobby>    */
        if ((thisRoom) == ERROR)
            thisRoom = LOBBY;

        getRoom(thisRoom);  /* room is gone, go back to Lobby> */

        return;
    }
    strcpy(roomBuf.rbname, roomname);

    for (i = 0; i < sizetable(); i++) {
        if (msgTab[i].mtroomno == (uchar) thisRoom)
            changeheader(cfg.mtoldest + i,
            3 /* Dump      */ ,
            255 /* No change */ );
    }
    roomBuf.rbgen = (roomTab[thisRoom].rtgen + 1) % MAXGEN;

    noteRoom();         /* index new room       */
    if (strcmp(roomTab[thisRoom].rtname, roomBuf.rbname) != SAMESTRING) {
        cPrintf("Room names changed roomTab = \"%s\", roomBuf = \"%s\"\n",
        roomTab[thisRoom].rtname, roomBuf.rbname);
    }
    putRoom(thisRoom);

    /* remove room from all halls */
    for (i = 0; i < MAXHALLS; i++) {
    /* remove from halls */
        hallBuf->hall[i].hroomflags[thisRoom].inhall = FALSE;

    /* unwindow */
        hallBuf->hall[i].hroomflags[thisRoom].window = FALSE;
    }

    /* put room in current hall */
    hallBuf->hall[thisHall].hroomflags[thisRoom].inhall = TRUE;

    /* put room in maintenance hall */
    hallBuf->hall[1].hroomflags[thisRoom].inhall = TRUE;

    putHall();          /* save it */

    sprintf(msgBuf->mbtext, "%s> created by %s", roomname, logBuf.lbname);

    trap(msgBuf->mbtext, T_NEWROOM);

    aideMessage();

    logBuf.lbroom[thisRoom].lbgen = roomBuf.rbgen;
    logBuf.lbroom[thisRoom].lvisit = 0;
}

/************************************************************************
 *      renameRoom() is an aide or sysop special fn
 *      Returns:        TRUE on success else FALSE
 ************************************************************************/
int renameRoom(void)
{
    char pathname[64];
    char roomname[NAMESIZE + 1];
    char oldname[NAMESIZE + 1];
    char summary[256];
    char line[80];
    char waspublic;
    int groupslot;
    char groupname[NAMESIZE + 1];
    char description[13];
    int roomslot;
	int s_q;

    summary[0] = '\0';

    strcpy(oldname, roomBuf.rbname);

    if (!roomBuf.rbflags.MSDOSDIR)
        roomBuf.rbdirname[0] = '\0';

    while (onConsole || gotCarrier()) {
		s_q = 1;		/* Save - Don't save flag */
        doCR();
        formatSummary(summary);
        mPrintf("%s", summary);
        doCR();
        doCR();

        mtPrintf(TERM_REVERSE,"Room Edit function: ");

        switch (toupper(iChar())) {
            case 'A':
                mPrintf("\bApplication");
                doCR();
                if (gl_user.sysop && onConsole) {
                    if (getYesNo("Application", (uchar) (roomBuf.rbflags.APLIC))) {
                        getString("Application filename", description, 13, FALSE,
                        ECHO, (roomBuf.rbaplic[0]) ? roomBuf.rbaplic : "");

                        strcpy(roomBuf.rbaplic, description);

                        roomBuf.rbflags.APLIC = TRUE;
                    } else {
                        roomBuf.rbaplic[0] = '\0';
                        roomBuf.rbflags.APLIC = FALSE;
                    }
                } else {
                    mPrintf("Must be Sysop at console to enter application.");
                    doCR();
                }
                break;

            case 'C':
                mPrintf("\bChange Name \n ");
                getNormStr("New room name", roomname, NAMESIZE, ECHO);
                roomslot = roomExists(roomname);
                if (roomslot >= 0 && roomslot != thisRoom) {
                    mPrintf("A \"%s\" room already exists.\n", roomname);
                } else {
                    strcpy(roomBuf.rbname, roomname);   /* also in room itself */
                }
                break;

            case 'I':
                mPrintf("\bInfo-line \n ");
                getNormStr("New room Info-line", roomBuf.descript, 79, ECHO);
                break;

            case 'D':
                mPrintf("\bDirectory");
                doCR();

                if (gl_user.sysop) {
                    if (getYesNo("Directory room", (uchar) roomBuf.rbflags.MSDOSDIR)) {
                        roomBuf.rbflags.MSDOSDIR = TRUE;

                        if (!roomBuf.rbdirname[0])
                            mPrintf(" No drive and path");
                        else
                            mPrintf(" Now space %s", roomBuf.rbdirname);

                        doCR();
                        getString("Path", pathname, 63, FALSE, ECHO,
                        (roomBuf.rbdirname[0]) ? roomBuf.rbdirname : "A:");
                        pathname[0] = (char) toupper(pathname[0]);

                        doCR();
                        mPrintf("Checking pathname \"%s\"", pathname);
                        doCR();

                        if (directory_l(pathname) && !onConsole) {
                            gl_user.sysop = FALSE;
                            gl_user.aide = FALSE;
                            logBuf.VERIFIED = TRUE;

                            sprintf(msgBuf->mbtext,
                            "Security violation on directory %s by %s\n "
                            "User unverified.", pathname, logBuf.lbname);
                            aideMessage();

                            doCR();
                            mPrintf("Security violation, your account is being "
                            "held for sysop's review");
                            doCR();
                            Initport();

                            getRoom(thisRoom);
                            return FALSE;
                        }
                        if (changedir(pathname) != -1) {
                            mPrintf(" Now space %s", pathname);
                            doCR();
                            strcpy(roomBuf.rbdirname, pathname);
                        } else {
                            mPrintf("%s does not exist! ", pathname);
                            if (getYesNo("Create", 0)) {
                                if (mkdir(pathname) == -1) {
                                    mPrintf("mkdir() ERROR!");
                                    strcpy(roomBuf.rbdirname, cfg.temppath);
                                } else {
                                    strcpy(roomBuf.rbdirname, pathname);
                                    mPrintf(" Now space %s", roomBuf.rbdirname);
                                    doCR();
                                }
                            } else
                                strcpy(roomBuf.rbdirname, cfg.temppath);
                        }

                        if (roomBuf.rbflags.GROUPONLY && roomBuf.rbflags.MSDOSDIR) {
                            roomBuf.rbflags.DOWNONLY =
                            getYesNo("Download only",
                            (uchar) roomBuf.rbflags.DOWNONLY);

                            if (!roomBuf.rbflags.DOWNONLY) {
                                roomBuf.rbflags.UPONLY = getYesNo("Upload only",
                                (uchar) roomBuf.rbflags.UPONLY);
                            }
                        }
                    } else {
                        roomBuf.rbflags.MSDOSDIR = 0;
                        roomBuf.rbflags.DOWNONLY = 0;
                    }
                    changedir(cfg.homepath);
                } else {
                    doCR();
                    mPrintf("Must be Sysop to make directories.");
                    doCR();
                }
                break;

            case 'F':
                mPrintf("\bFile (room description)");
                doCR();

                if (cfg.roomtell && gl_user.sysop) {
                    if (getYesNo("Display room description File",
                    (uchar) (roomBuf.rbroomtell[0] != '\0'))) {
                        getString("Description Filename", description, 13, FALSE,
                        ECHO, (roomBuf.rbroomtell[0]) ? roomBuf.rbroomtell : "");
                        strcpy(roomBuf.rbroomtell, description);
                    } else
                        roomBuf.rbroomtell[0] = '\0';
                } else {
                    doCR();
                    mPrintf("Must be Sysop and have Room descriptions configured.");
                    doCR();
                }
                break;

            case 'G':
                mPrintf("\bGroup");
                doCR();
                if ((thisRoom > 2) || (thisRoom > 0 && gl_user.sysop)) {
                    if (getYesNo("Change Group", 0)) {
                        getString("Group for room <CR> for no group",
                        groupname, NAMESIZE, FALSE, ECHO, "");

                        roomBuf.rbflags.GROUPONLY = 1;

                        groupslot = groupexists(groupname);

                        if (!strlen(groupname) || (groupslot == ERROR)) {
                            roomBuf.rbflags.GROUPONLY = 0;
                            roomBuf.rbflags.READONLY = 0;
                            roomBuf.rbflags.DOWNONLY = 0;

                            if (groupslot == ERROR && strlen(groupname))
                                mPrintf("No such group.");
                        }
                        if (roomBuf.rbflags.GROUPONLY) {
                            roomBuf.rbgrpno = (unsigned char) groupslot;
                            roomBuf.rbgrpgen = grpBuf.group[groupslot].groupgen;
                        }
                    }
                } else {
                    if (thisRoom > 0) {
                        doCR();
                        mPrintf("Must be Sysop to change group for Mail> or Aide)");
                        doCR();
                    } else {
                        doCR();
                        mPrintf("Lobby> can never be group only");
                        doCR();
                    }
                }
                break;

            case 'H':
                mPrintf("\bHidden Room");
                doCR();
                if ((thisRoom > 2) || (thisRoom > 0 && gl_user.sysop)) {
                    waspublic = (uchar) roomBuf.rbflags.PUBLIC;

                    roomBuf.rbflags.PUBLIC =
                    !getYesNo("Hidden room", (uchar) (!roomBuf.rbflags.PUBLIC));

                    if (waspublic && (!roomBuf.rbflags.PUBLIC)) {
                        roomBuf.rbgen = (roomBuf.rbgen + 1) % MAXGEN;
                        logBuf.lbroom[thisRoom].lbgen = roomBuf.rbgen;
                    }
                } else {
                    doCR();
                    mPrintf("Must be Sysop to make Lobby>, Mail> or Aide) hidden.");
                    doCR();
                }
                break;

            case 'M':
                mPrintf("\bModerated");
                doCR();
                if (gl_user.sysop) {
                    if (getYesNo("Moderated", (uchar) (roomBuf.rbflags.MODERATED)))
                        roomBuf.rbflags.MODERATED = TRUE;
                    else
                        roomBuf.rbflags.MODERATED = FALSE;
                } else {
                    doCR();
                    mPrintf("Must be Sysop to make Moderated rooms.");
                    doCR();
                }
                break;

            case 'N':
                mPrintf("\bNetworked");
                doCR();
                if (gl_user.sysop) {
                    roomBuf.rbflags.SHARED = getYesNo("Networked/Shared room",
                    (uchar) roomBuf.rbflags.SHARED);
                } else {
                    doCR();
                    mPrintf("Must be Sysop to make Shared rooms.");
                    doCR();
                }
                break;

			/* all the >good< letters were taken, so call it "Omit headers" */

            case 'P':
                mPrintf("\bPermanent");
                doCR();
                if (thisRoom > DUMP) {
                    if (!roomBuf.rbflags.MSDOSDIR) {
                        roomBuf.rbflags.PERMROOM =
                        getYesNo("Permanent", (uchar) roomBuf.rbflags.PERMROOM);
                    } else {
                        roomBuf.rbflags.PERMROOM = 1;
                        doCR();
                        mPrintf("Directory rooms are automatically Permanent.");
                        doCR();
                    }
                } else {
                    doCR();
                    mPrintf("Lobby> Mail> Aide) or Dump> always Permanent.");
                    doCR();
                }
                break;

            case 'R':
                mPrintf("\bRead Only");
                doCR();
                if (roomBuf.rbflags.GROUPONLY) {
                    roomBuf.rbflags.READONLY =
                    getYesNo("Read only", (uchar) roomBuf.rbflags.READONLY);
                } else {
                    doCR();
                    mPrintf("Must be a group only room to be Read only.");
                    doCR();
                }
                break;

            case 'Y':
                mPrintf("\banonYmous");
                doCR();
                if (gl_user.sysop) {
                    roomBuf.rbflags.ANONYMOUS = getYesNo("Anonymous",
                    (uchar) roomBuf.rbflags.ANONYMOUS);
                } else {
                    doCR();
                    mPrintf("Must be a sysop to make Anonymous rooms.");
                    doCR();
                }
                break;

			/* save/exit section */
			case 'Q':
				s_q = 0;
            case 'S':
				if (s_q)
					mPrintf("\bSave");
				else
					mPrintf("\bQuit");
                doCR();
                if (getYesNo("Save changes", !s_q)) {
                    noteRoom();
                    putRoom(thisRoom);

        /* trap file line */
                    sprintf(line, "Room \'%s\' changed to \'%s\' by %s",
                    oldname, roomBuf.rbname, logBuf.lbname);
                    trap(line, T_AIDE);

                    sprintf(msgBuf->mbtext, "%s \n%s", line, summary);
                    aideMessage();

                    return TRUE;
                } else {
                    getRoom(thisRoom);
                    return FALSE;
                }
        /* break; Unreachable code */

            case '?':
                mPrintf(" for menu");
                nextmenu("roomedit", &(cfg.cnt.roomedittut), 1);
                break;

            default:
                if (!gl_user.expert)
                    mPrintf(" '?' for menu.\n ");
                else
                    mPrintf(" ?\n ");
                break;

        }           /* end of switch */

    }               /* end of while */

    getRoom(thisRoom);
    return FALSE;
}

/************************************************************************/
/*      killempties() aide fn: to kill empty rooms                      */
/************************************************************************/
void killempties(void)
{
    label oldName;
    int rm, roomExists();

    if (!getYesNo(confirm, 0))
        return;

    sprintf(msgBuf->mbtext, "The following empty rooms deleted by %s: ",
    logBuf.lbname);
    trap(msgBuf->mbtext, T_AIDE);

    strcpy(oldName, roomBuf.rbname);
    indexRooms();

    if ((rm = roomExists(oldName)) != ERROR)
        getRoom(rm);
    else
        getRoom(LOBBY);

    aideMessage();
}


/************************************************************************/
/*      killroom() aide fn: to kill current room                        */
/************************************************************************/
void killroom(void)
{
    int i;

    if (thisRoom == LOBBY || thisRoom == MAILROOM
    || thisRoom == AIDEROOM || thisRoom == DUMP) {
        doCR();
        mPrintf(" Cannot kill %s>, %s>, %s), or %s>",
        roomTab[LOBBY].rtname,
        roomTab[MAILROOM].rtname,
        roomTab[AIDEROOM].rtname,
        roomTab[DUMP].rtname);
        return;
    }
    if (!getYesNo(confirm, 0))
        return;

    for (i = 0; i < sizetable(); i++) {
        if (msgTab[i].mtroomno == (uchar) thisRoom) {
            changeheader((ulong) (cfg.mtoldest + i),
            3 /* Dump */ , 255 /* no change */ );
        }
    }

    /* kill current room from every hall */
    for (i = 0; i < MAXHALLS; i++) {
        hallBuf->hall[i].hroomflags[thisRoom].inhall = FALSE;
        hallBuf->hall[i].hroomflags[thisRoom].window = FALSE;
    }
    putHall();          /* update hall buffer */

    sprintf(msgBuf->mbtext, "%s> killed by %s",
    roomBuf.rbname, logBuf.lbname);

    trap(msgBuf->mbtext, T_AIDE);

    aideMessage();

    roomBuf.rbflags.INUSE = FALSE;
    putRoom(thisRoom);
    noteRoom();
    getRoom(LOBBY);
}


/************************************************************************/
/*     massdelete()  sysop fn: to kill all msgs from person             */
/************************************************************************/
void massdelete(void)
{
    label who;
    char string[80];
    int i, namehash, killed = 0;

    getNormStr("who", who, NAMESIZE, ECHO);

    if (!strlen(who))
        return;

    sprintf(string, "Delete all messages from %s", who);

    namehash = hash(who);

    if (getYesNo(string, 0)) {
        for (i = 0; i < sizetable(); ++i) {
            if (msgTab[i].mtauthhash == namehash) {
                if (msgTab[i].mtroomno != 3 /* DUMP */ ) {
                    ++killed;

                    changeheader((ulong) (cfg.mtoldest + i),
                    3 /* Dump */ , 255 /* no change */ );
                }
            }
        }

        mPrintf("%d messages killed", killed);

        doCR();

        if (killed) {
            sprintf(msgBuf->mbtext, "All messages from %s deleted", who);
            trap(msgBuf->mbtext, T_SYSOP);
        }
    }
}

/************************************************************************/
/*      readbymsgno()  sysop fn: to read by message #                   */
/************************************************************************/
void readbymsgno(void)
{
    ulong msgno;
    int slot;

    doCR();

    msgno = (ulong) getNumber("message number to read",
		(long) cfg.mtoldest, (long) cfg.newest, (long) cfg.newest);

    slot = indexslot(msgno);

    if (!mayseeindexmsg(slot))
        mPrintf("Message not viewable.");
    else
        printMessage(msgno, (char) 1);

    doCR();
}

/* ------------------------------------------------------------------------ */
/*  directory_l()   returns if a directory is locked                        */
/* ------------------------------------------------------------------------ */
static int directory_l(char *str)
{
    FILE *fBuf;
    char line[90];
    char *words[256];
    char path[80];

    sprintf(path, "%s\\external.cit", cfg.homepath);

    if ((fBuf = fopen(path, "r")) == NULL) {    /* ASCII mode */
        crashout("Can't find external.cit!");
    }
    while (fgets(line, 90, fBuf) != NULL) {
        if (line[0] != '#')
            continue;

        if (strnicmp(line, "#DIRECTORY", 5) != SAMESTRING)
            continue;

        parse_it(words, line);

		if (u_match(str, words[1])) {
			fclose(fBuf);
			return TRUE;
		}
    }
    fclose(fBuf);
    return FALSE;
}

/* EOF */
