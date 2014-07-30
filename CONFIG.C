/************************************************************************/
/*                              config.c                                */
/*      configuration program for Citadel bulletin board system.        */
/************************************************************************/

#include <string.h>
#include "ctdl.h"
#include "proto.h"
#include "keywords.h"
#include "global.h"
#include "config.atm"

/************************************************************************/
/*                              Contents                                */
/*      buildcopies()           copies appropriate msg index members    */
/*      buildhalls()            builds hall-table (all rooms in Maint.) */
/*      buildroom()             builds a new room according to msg-buf  */
/*      clearaccount()          sets all group accounting data to zero  */
/*      configcit()             the main configuration for citadel      */
/*      illegal()               abort config.exe program                */
/*      initfiles()             opens & initalizes any missing files    */
/*      logInit()               indexes log.dat                         */
/*      logSort()               Sorts 2 entries in logTab               */
/*      msgInit()               builds message table from msg.dat       */
/*      readaccount()           reads grpdata.cit values into grp struct*/
/*      readprotocols()         reads external.cit values into ext struc*/
/*      readconfig()            reads config.cit values                 */
/*      RoomTabBld()            builds room.tab, index's room.dat       */
/*      showtypemsg()           displays what kind of message was read  */
/*      slidemsgTab()           frees slots at beginning of msg table   */
/*      zapGrpFile()            initializes grp.dat                     */
/*      zapHallFile()           initializes hall.dat                    */
/*      zapLogFile()            initializes log.dat                     */
/*      zapMsgFile()            initializes msg.dat                     */
/*      zapRoomFile()           initializes room.dat                    */
/************************************************************************/

/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/CONFIG.C_V  $
 * 
 *    Rev 1.47   21 Sep 1991 10:18:28   FJM
 * FredCit release
 *
 *    Rev 1.46   19 Jun 1991 20:25:42   FJM
 * Made node, region names over 20 characters acceptable in CONFIG.CIT.
 *
 *    Rev 1.45   27 May 1991 11:41:16   FJM
 * Changes to support reading Config.Cit with .SEC
 *
 *    Rev 1.44   22 May 1991  2:14:56   FJM
 * Made memory check optional
 *
 *    Rev 1.40   04 Apr 1991 14:12:14   FJM
 * Made accountBuf dynamicly allocated.
 *
 *    Rev 1.36   10 Feb 1991 17:32:42   FJM
 *    Rev 1.34   05 Feb 1991 14:30:40   FJM
 * Added SCREEN_SAVE
 *
 *    Rev 1.33   28 Jan 1991 13:10:12   FJM
 *    Rev 1.28   18 Jan 1991 16:49:14   FJM
 * Added #SWAPFILE
 *
 *    Rev 1.26   13 Jan 1991  0:30:26   FJM
 * Name overflow fixes.
 *
 *    Rev 1.24   11 Jan 1991 12:42:46   FJM
 *    Rev 1.20   06 Jan 1991 12:45:04   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.19   27 Dec 1990 20:16:50   FJM
 * Changes for porting.
 *
 *    Rev 1.16   22 Dec 1990 13:39:28   FJM
 *    Rev 1.8   07 Dec 1990 23:10:02   FJM
 *    Rev 1.6   23 Nov 1990 23:23:08   FJM
 * Update to ATOMS method
 *
 *    Rev 1.5   23 Nov 1990 23:15:24   FJM
 * Modified for new "ATOMS" method of NLS support.
 *
 *    Rev 1.4   23 Nov 1990 13:22:28   FJM
 *    Rev 1.2   17 Nov 1990 16:11:44   FJM
 * Added version control log header
 * --------------------------------------------------------------------
 *  EARLY HISTORY:
 *
 *  03/08/90    {zm}    Fix some spelling errors.
 *  03/10/90    {zm}    Allow for possible COM3 or COM4.
 *  03/11/90    FJM     Allow COM ports 1-4
 *  03/12/90    {zm}    Add #DEFAULT_GROUP to external.cit
 *  03/19/90    FJM     Linted & partial cleanup
 *  06/06/90    FJM     Modified logSort for TC++
 *  07/29/90    FJM     Added ADCHANCE.  Percentage chance of an ad
 *                      appearing.  0 to disable.
 *                      Added ADTIME.  Idle time in seconds before an ad
 *                      appears.  0 to disable.
 *  09/21/90    FJM     Added #SYSOP, #CREDIT_NAME
 *  10/15/90    FJM     Added console only protocol flag (field #1)
 *  10/18/90    FJM     Changed initfiles() to use changedir(), not chdir
 *
 * -------------------------------------------------------------------- */

/************************************************************************
 *                local functions
 ************************************************************************/

static void checkDir(char *path);

/************************************************************************
 *                External variables
 ************************************************************************/

/************************************************************************/
/*      buildcopies()  copies appropriate msg index members             */
/************************************************************************/
void buildcopies(void)
{
    int i;

    for (i = 0; i < sizetable(); ++i) {
        if (msgTab[i].mtmsgflags.COPY) {
            if (msgTab[i].mtoffset <= i) {
                copyindex(i, (i - msgTab[i].mtoffset));
            }
        }
    }
}

/************************************************************************/
/*      buildhalls()  builds hall-table (all rooms in Maint.)           */
/************************************************************************/
void buildhalls(void)
{
    int i;

    doccr();
    cPrintf(ATOM1001);
    doccr();

    for (i = 4; i < MAXROOMS; ++i) {
        if (roomTab[i].rtflags.INUSE) {
            hallBuf->hall[1].hroomflags[i].inhall = 1;  /* In Maintenance */
            hallBuf->hall[1].hroomflags[i].window = 0;  /* Not a Window   */
        }
    }
    putHall();
}

/************************************************************************/
/*      buildroom()  builds a new room according to msg-buf             */
/************************************************************************/
void buildroom(void)
{
    int roomslot;

    if (*msgBuf->mbcopy)
        return;
    roomslot = msgBuf->mbroomno;

    if (msgBuf->mbroomno < MAXROOMS) {
        getRoom(roomslot);

        if ((strcmp(roomBuf.rbname, msgBuf->mbroom) != SAMESTRING)
        || (!roomBuf.rbflags.INUSE)) {
            if (msgBuf->mbroomno > 3) {
                roomBuf.rbflags.INUSE = TRUE;
                roomBuf.rbflags.PERMROOM = FALSE;
                roomBuf.rbflags.MSDOSDIR = FALSE;
                roomBuf.rbflags.GROUPONLY = FALSE;
                roomBuf.rbroomtell[0] = '\0';
                roomBuf.rbflags.PUBLIC = TRUE;
            }
            strcpy(roomBuf.rbname, msgBuf->mbroom);

            putRoom(thisRoom);
        }
    }
}

/************************************************************************/
/*      clearaccount()  initializes all group data                      */
/************************************************************************/
void clearaccount(void)
{
    int i;
    int groupslot;

    for (groupslot = 0; groupslot < MAXGROUPS; groupslot++) {
    /* init days */
        for (i = 0; i < 7; i++)
            accountBuf->group[groupslot].account.days[i] = 1;

    /* init hours & special hours */
        for (i = 0; i < 24; i++) {
            accountBuf->group[groupslot].account.hours[i] = 1;
            accountBuf->group[groupslot].account.special[i] = 0;
        }

        accountBuf->group[groupslot].account.have_acc = FALSE;
        accountBuf->group[groupslot].account.dayinc = 0.;
        accountBuf->group[groupslot].account.maxbal = 0.;
        accountBuf->group[groupslot].account.priority = 0.;
        accountBuf->group[groupslot].account.dlmult = -1;
        accountBuf->group[groupslot].account.ulmult = 1;

    }
}

/************************************************************************/
/*      configcit() the <main> for configuration                        */
/************************************************************************/
void configcit(void)
{
    fcloseall();

    /* read config.cit */
    readconfig(1);
	
	/* allocate memory for tables */
    allocateTables();

	/* read cron events */
    readcron();
	
    /* move to home-path */
    changedir(cfg.homepath);

    /* initialize & open any files */
    initfiles();

    if (msgZap)
        zapMsgFile();
    if (roomZap)
        zapRoomFile();
    if (logZap)
        zapLogFile();
    if (grpZap)
        zapGrpFile();
    if (hallZap)
        zapHallFile();

    if (roomZap && !msgZap)
        roomBuild = TRUE;
    if (hallZap && !msgZap)
        hallBuild = TRUE;

    logInit();
    msgInit();
    RoomTabBld();

    if (hallBuild)
        buildhalls();

    fclose(grpfl);
    fclose(hallfl);
    fclose(roomfl);
    fclose(msgfl);
    fclose(logfl);

    doccr();
    cPrintf(ATOM1002);
    doccr();
}

/***********************************************************************/
/*    illegal() Prints out configure error message and aborts          */
/***********************************************************************/
void illegal(char *errorstring)
{
    doccr();
    cPrintf("%s", errorstring);
    doccr();
    cPrintf(ATOM1003);
    doccr();
    exit(7);
}

/************************************************************************/
/*      initfiles() -- initializes files, opens them                    */
/************************************************************************/
void initfiles(void)
{
    char *grpFile, *hallFile, *logFile, *msgFile, *roomFile;
    char scratch[64];

    changedir(cfg.homepath);

    if (cfg.msgpath[(strlen(cfg.msgpath) - 1)] == '\\')
        cfg.msgpath[(strlen(cfg.msgpath) - 1)] = '\0';

    sprintf(scratch, "%s\\%s", cfg.msgpath, ATOM1004);

    grpFile = ATOM1005;
    hallFile = ATOM1006;
    logFile = ATOM1007;
    msgFile = scratch;
    roomFile = ATOM1008;

    /* open group file */
    if ((grpfl = fopen(grpFile, "r+b")) == NULL) {
        cPrintf(ATOM1009, grpFile);
        doccr();
        if ((grpfl = fopen(grpFile, "w+b")) == NULL)
            illegal(ATOM1010);
        else {
            cPrintf(ATOM1011);
            doccr();
            grpZap = TRUE;
        }
    }
    /* open hall file */
    if ((hallfl = fopen(hallFile, "r+b")) == NULL) {
        cPrintf(ATOM1012, hallFile);
        doccr();
        if ((hallfl = fopen(hallFile, "w+b")) == NULL)
            illegal(ATOM1013);
        else {
            cPrintf(ATOM1014);
            doccr();
            hallZap = TRUE;
        }
    }
    /* open log file */
    if ((logfl = fopen(logFile, "r+b")) == NULL) {
        cPrintf(ATOM1015, logFile);
        doccr();
        if ((logfl = fopen(logFile, "w+b")) == NULL)
            illegal(ATOM1016);
        else {
            cPrintf(ATOM1017);
            doccr();
            logZap = TRUE;
        }
    }
    /* open message file */
    if ((msgfl = fopen(msgFile, "r+b")) == NULL) {
        cPrintf(ATOM1018);
        doccr();
        if ((msgfl = fopen(msgFile, "w+b")) == NULL)
            illegal(ATOM1019);
        else {
            cPrintf(ATOM1020);
            doccr();
            msgZap = TRUE;
        }
    }
    /* open room file */
    if ((roomfl = fopen(roomFile, "r+b")) == NULL) {
        cPrintf(ATOM1021, roomFile);
        doccr();
        if ((roomfl = fopen(roomFile, "w+b")) == NULL)
            illegal(ATOM1022);
        else {
            cPrintf(ATOM1023);
            doccr();
            roomZap = TRUE;
        }
    }
}

/************************************************************************/
/*      logInit() indexes log.dat                                       */
/************************************************************************/
void logInit(void)
{
    int i;
    int logSort();
    int count = 0;

    doccr();
    doccr();
    cPrintf(ATOM1024);
    doccr();

    cfg.callno = 0l;

    rewind(logfl);
    /* clear logTab */
    for (i = 0; i < cfg.MAXLOGTAB; i++)
        logTab[i].ltcallno = 0l;

    /* load logTab: */
    for (thisLog = 0; thisLog < cfg.MAXLOGTAB; thisLog++) {

        cPrintf(ATOM1025, thisLog);

        getLog(&logBuf, thisLog);

        if (logBuf.callno > cfg.callno)
            cfg.callno = logBuf.callno;

    /* count valid entries:             */

        if (logBuf.lbflags.L_INUSE == 1)
            count++;


    /* copy relevant info into index:   */
        logTab[thisLog].ltcallno = logBuf.callno;
        logTab[thisLog].ltlogSlot = thisLog;
        logTab[thisLog].permanent = logBuf.lbflags.PERMANENT;

        if (logBuf.lbflags.L_INUSE == 1) {
            logTab[thisLog].ltnmhash = hash(logBuf.lbname);
            logTab[thisLog].ltinhash = hash(logBuf.lbin);
            logTab[thisLog].ltpwhash = hash(logBuf.lbpw);
        } else {
            logTab[thisLog].ltnmhash = 0;
            logTab[thisLog].ltinhash = 0;
            logTab[thisLog].ltpwhash = 0;
        }
    }
    doccr();
    cPrintf(ATOM1026, cfg.callno);
    doccr();
    cPrintf(ATOM1027, count);
    doccr();

    qsort(logTab, (unsigned) cfg.MAXLOGTAB,
    (unsigned) sizeof(*logTab), logSort);

}

/************************************************************************/
/*      logSort() Sorts 2 entries in logTab                             */
/************************************************************************/
int logSort(const void *p1, const void *p2)
/* struct lTable *s1, *s2;     */
{
    struct lTable *s1, *s2;

    s1 = (struct lTable *) p1;
    s2 = (struct lTable *) p2;
    if (s1->ltnmhash == 0 && (struct lTable *) (s2)->ltnmhash == 0)
        return 0;
    if (s1->ltnmhash == 0 && s2->ltnmhash != 0)
        return 1;
    if (s1->ltnmhash != 0 && s2->ltnmhash == 0)
        return -1;
    if (s1->ltcallno < s2->ltcallno)
        return 1;
    if (s1->ltcallno > s2->ltcallno)
        return -1;
    return 0;
}

/************************************************************************/
/*      msgInit() sets up lowId, highId, cfg.catSector and cfg.catChar, */
/*      by scanning over message.buf                                    */
/************************************************************************/
void msgInit(void)
{
    ulong first, here;
    int makeroom;
    int skipcounter = 0;    /* maybe need to skip a few . Dont put them in message index */
    int slot;

    doccr();
    doccr();
    cPrintf(ATOM1028);
    doccr();

    /* start at the beginning */
    fseek(msgfl, 0l, 0);

    getMessage();

    /* get the ID# */
    sscanf(msgBuf->mbId, "%ld", &first);

    showtypemsg(first);

    /* put the index in its place */
    /* mark first entry of index as a reference point */

    cfg.mtoldest = first;

    indexmessage(first);

    cfg.newest = cfg.oldest = first;

    cfg.catLoc = ftell(msgfl);

    while (TRUE) {
        getMessage();

        sscanf(msgBuf->mbId, "%ld", &here);

        if (here == first)
            break;

        showtypemsg(here);

    /* find highest and lowest message IDs: */
    /* >here< is the dip pholks             */
        if (here < cfg.oldest) {
            slot = (indexslot(cfg.newest) + 1);

            makeroom = (int) (cfg.mtoldest - here);

        /* check to see if our table can hold remaining msgs */
            if (cfg.nmessages < (makeroom + slot)) {
                skipcounter = (makeroom + slot) - cfg.nmessages;

                slidemsgTab(makeroom - skipcounter);

                cfg.mtoldest = (here + (ulong) skipcounter);

            } else {
        /* nmessages can handle it.. Just make room */
                slidemsgTab(makeroom);
                cfg.mtoldest = here;
            }
            cfg.oldest = here;
        }
        if (here > cfg.newest) {
            cfg.newest = here;

        /* read rest of message in and remember where it ends,      */
        /* in case it turns out to be the last message              */
        /* in which case, that's where to start writing next message */
            while (dGetWord(msgBuf->mbtext, MAXTEXT));

            cfg.catLoc = ftell(msgfl);
        }
    /* check to see if our table is big enough to handle it */
        if ((int) (here - cfg.mtoldest) >= cfg.nmessages) {
            crunchmsgTab(1);
        }
        if (skipcounter) {
            skipcounter--;
        } else {
            indexmessage(here);
        }
    }
    buildcopies();
}

/************************************************************************/
/*      readaccount()  reads grpdata.cit values into group structure    */
/************************************************************************/
void readaccount(void)
{
    FILE *fBuf;
    char line[90];
    char *words[256];
    int i, j, k, l, count;
    int groupslot = ERROR;
    int hour;

    clearaccount();     /* initialize all accounting data */

    /* move to home-path */
    changedir(cfg.homepath);

    if ((fBuf = fopen(ATOM1029, "r")) == NULL) {
    /* ASCII mode */
        cPrintf(ATOM1030);
        doccr();
        exit(1);
    }
    while (fgets(line, 90, fBuf) != NULL) {
        if (line[0] != '#')
            continue;

        count = parse_it(words, line);

        for (i = 0; grpkeywords[i] != NULL; i++) {
            if (strcmpi(words[0], grpkeywords[i]) == SAMESTRING) {
                break;
            }
        }

        switch (i) {
            case GRK_DAYS:
                if (groupslot == ERROR)
                    break;

        /* init days */
                for (j = 0; j < 7; j++)
                    accountBuf->group[groupslot].account.days[j] = 0;

                for (j = 1; j < count; j++) {
                    for (k = 0; daykeywords[k] != NULL; k++) {
                        if (strcmpi(words[j], daykeywords[k]) == SAMESTRING) {
                            break;
                        }
                    }
                    if (k < 7)
                        accountBuf->group[groupslot].account.days[k] = TRUE;
                    else if (k == 7) {  /* any */
                        for (l = 0; l < MAXGROUPS; ++l)
                            accountBuf->group[groupslot].account.days[l] = TRUE;
                    } else {
                        doccr();
                        cPrintf(
                        ATOM1031,
                        words[j]);
                        doccr();
                    }
                }
                break;

            case GRK_GROUP:
                groupslot = groupexists(words[1]);
                if (groupslot == ERROR) {
                    doccr();
                    cPrintf(ATOM1032, words[1]);
                    doccr();
                }
                accountBuf->group[groupslot].account.have_acc = TRUE;
                break;

            case GRK_HOURS:
                if (groupslot == ERROR)
                    break;

        /* init hours */
                for (j = 0; j < 24; j++)
                    accountBuf->group[groupslot].account.hours[j] = 0;

                for (j = 1; j < count; j++) {
                    if (strcmpi(words[j], ATOM1033) == SAMESTRING) {
                        for (l = 0; l < 24; l++)
                            accountBuf->group[groupslot].account.hours[l] = TRUE;
                    } else {
                        hour = atoi(words[j]);

                        if (hour > 23) {
                            doccr();
                            cPrintf(ATOM1034,
                            hour);
                            doccr();
                        } else
                            accountBuf->group[groupslot].account.hours[hour] = TRUE;
                    }
                }
                break;

            case GRK_DAYINC:
                if (groupslot == ERROR)
                    break;

                if (count > 1) {
                    sscanf(words[1], "%f ",
                    &accountBuf->group[groupslot].account.dayinc);    /* float */
                }
                break;

            case GRK_DLMULT:
                if (groupslot == ERROR)
                    break;

                if (count > 1) {
                    sscanf(words[1], "%f ",
                    &accountBuf->group[groupslot].account.dlmult);    /* float */
                }
                break;

            case GRK_ULMULT:
                if (groupslot == ERROR)
                    break;

                if (count > 1) {
                    sscanf(words[1], "%f ",
                    &accountBuf->group[groupslot].account.ulmult);    /* float */
                }
                break;

            case GRK_PRIORITY:
                if (groupslot == ERROR)
                    break;

                if (count > 1) {
                    sscanf(words[1], "%f ",
                    &accountBuf->group[groupslot].account.priority);  /* float */
                }
                break;

            case GRK_MAXBAL:
                if (groupslot == ERROR)
                    break;

                if (count > 1) {
                    sscanf(words[1], "%f ",
                    &accountBuf->group[groupslot].account.maxbal);    /* float */
                }
                break;



            case GRK_SPECIAL:
                if (groupslot == ERROR)
                    break;

        /* init hours */
                for (j = 0; j < 24; j++)
                    accountBuf->group[groupslot].account.special[j] = 0;

                for (j = 1; j < count; j++) {
                    if (strcmpi(words[j], ATOM1035) == SAMESTRING) {
                        for (l = 0; l < 24; l++)
                            accountBuf->group[groupslot].account.special[l] = TRUE;
                    } else {
                        hour = atoi(words[j]);

                        if (hour > 23) {
                            doccr();
                            cPrintf(ATOM1036, hour);
                            doccr();
                        } else
                            accountBuf->group[groupslot].account.special[hour] = TRUE;
                    }

                }
                break;
        }

    }
    fclose(fBuf);
}

/************************************************************************/
/*      readprotocols() reads external.cit values into ext   structure  */
/************************************************************************/
void readprotocols(void)
{
    FILE *fBuf;
    char line[90];
    char *words[256];
    int j, count;
    int cmd = 0;

    extrncmd[0] = NULL;
    editcmd[0] = NULL;
    default_group[0] = NULL;

    /* move to home-path */
    changedir(cfg.homepath);

    if ((fBuf = fopen(ATOM1037, "r")) == NULL) {    /* ASCII mode */
        cPrintf(ATOM1038);
        doccr();
        exit(1);
    }
    while (fgets(line, 90, fBuf) != NULL) {
        if (line[0] != '#')
            continue;

        count = parse_it(words, line);

        if (strcmpi(ATOM1039, words[0]) == SAMESTRING) {
            j = strlen(extrncmd);

        /* Field 1, name */
            if (strlen(words[1]) > 19)
                illegal(ATOM1040);

        /* Field 2, Net/console only flag */
            if (atoi(words[2]) < 0 || atoi(words[2]) > 1)
                illegal(ATOM1041);

        /* Field 3, Batch protcol flag */
            if (atoi(words[3]) < 0 || atoi(words[3]) > 1)
                illegal(ATOM1042);

        /* Field 4, Block size, 0 means unknown or variable */
            if (atoi(words[4]) < 0 || atoi(words[4]) > 10 * 1024)
                illegal(ATOM1043);

        /* Field 5, Recieve command */
            if (strlen(words[5]) > 39)
                illegal(ATOM1044);

        /* Field 3, Send command */
            if (strlen(words[6]) > 39)
                illegal(ATOM1045);

            if (j >= MAXEXTERN)
                illegal(ATOM1046);

            strcpy(extrn[j].ex_name, words[1]);
            extrn[j].ex_console = atoi(words[2]);
            extrn[j].ex_batch = atoi(words[3]);
            extrn[j].ex_block = atoi(words[4]);
            strcpy(extrn[j].ex_rcv, words[5]);
            strcpy(extrn[j].ex_snd, words[6]);
            extrncmd[j] = tolower(*words[1]);
            extrncmd[j + 1] = '\0';
        } else if (strcmpi(ATOM1047, words[0]) == SAMESTRING) {
            j = strlen(editcmd);

            if (strlen(words[1]) > 19)
                illegal(ATOM1048);
            if (strlen(words[3]) > 29)
                illegal(ATOM1049);
            if (atoi(words[2]) < 0 || atoi(words[2]) > 1)
                illegal(ATOM1050);
            if (j > 19)
                illegal(ATOM1051);

            strcpy(edit[j].ed_name, words[1]);
            edit[j].ed_local = atoi(words[2]);
            strcpy(edit[j].ed_cmd, words[3]);
            editcmd[j] = tolower(*words[1]);
            editcmd[j + 1] = '\0';
        } else if (strcmpi(ATOM1052, words[0]) == SAMESTRING) {
            if (strlen(words[1]) > 19)
                illegal(ATOM1053);
            strcpy(default_group, words[1]);
        } else if (strcmpi(ATOM1054, words[0]) == SAMESTRING) {
            if (cmd >= MAXEXTERN) {
                illegal(ATOM1055);
            }
            if (count < 3)
                illegal(ATOM1056);
            if (*words[2] < '0' || *words[2] > '1')
                illegal(ATOM1057);
            if (strlen(words[1]) > 30) {
                illegal(ATOM1058);
            }
            if (strlen(words[3]) > 40) {
                illegal(ATOM1059);
            }
            strcpy(extCmd[cmd].name, words[1]);
            extCmd[cmd].local = *words[2] - '0';
            strcpy(extCmd[cmd].command, words[3]);
            ++cmd;
        } else if (strcmpi(ATOM1060, words[0]) == SAMESTRING) {
            if (count != 2)
                illegal(ATOM1061);
            if (!isdigit(*words[1]))
                illegal(ATOM1062);
            cfg.ad_chance = atoi(words[1]);
            ++cmd;
        } else if (strcmpi(ATOM1063, words[0]) == SAMESTRING) {
            if (count != 2)
                illegal(ATOM1064);
            if (!isdigit(*words[1]))
                illegal(ATOM1065);
            cfg.ad_time = atoi(words[1]);
            ++cmd;
        }
    }
    fclose(fBuf);
}

/*
 * count the lines that start with keyword...
 *
int keyword_count(key, filename)
char *key;
char *filename;
{
    FILE *fBuf;
    char line[90];
    char *words[256];
    int  count = 0;

    changedir(cfg.homepath);

    if ((fBuf = fopen(filename, "r")) == NULL) {
        cPrintf(ATOM1066, filename); doccr();
        exit(1);
    }

    while (fgets(line, 90, fBuf) != NULL) {
        if (line[0] != '#')  continue;

        parse_it( words, line);

        if (strcmpi(key, words[0]) == SAMESTRING)
          count++;
   }

   fclose(fBuf);

   return (count == 0 ? 1 : count);
} */

/************************************************************************
 *      checkDir() check if required directory exists, crashout if not.
 ************************************************************************/
static void checkDir(char *path)
{
	char buf[128];
	static int did_crash=0;		/* to avoid mutual recursion via crashout() */
	
	if (changedir(path) == -1) {
		sprintf(buf, "No path %s", path);
		if (!did_crash) {
			did_crash = 1;
			crashout(buf);
		}
	}
}
/************************************************************************/
/*      readconfig() reads config.cit values                            */
/************************************************************************/
void readconfig(int firstime)
{
    FILE *fBuf;
    char line[90];
    char *words[256];
    int i, j, k, l, count, att;
    label notkeyword;
    char valid = FALSE;
    char found[K_NWORDS + 2];
    int lineNo = 0;

    for (i = 0; i <= K_NWORDS; i++)
        found[i] = FALSE;

    if ((fBuf = fopen(ATOM1067, "r")) == NULL) {    /* ASCII mode */
        cPrintf(ATOM1068);
        doccr();
        exit(3);
    }
    while (fgets(line, 90, fBuf) != NULL) {
        lineNo++;

        if (line[0] != '#')
            continue;

        count = parse_it(words, line);

        for (i = 0; keywords[i] != NULL; i++) {
            if (strcmpi(words[0], keywords[i]) == SAMESTRING) {
                break;
            }
        }

        if (keywords[i] == NULL) {
            cPrintf(ATOM1069, lineNo,
            words[0]);
            doccr();
            continue;
        } else {
            if (found[i] == TRUE) {
                cPrintf(ATOM1070, lineNo, words[0]);
                doccr();
            } else {
                found[i] = TRUE;
            }
        }

        switch (i) {
            case K_ACCOUNTING:
                cfg.accounting = atoi(words[1]);
                break;

            case K_IDLE_WAIT:
                cfg.idle = atoi(words[1]);
                break;

            case K_ALLDONE:
                break;

            case K_ATTR:
                sscanf(words[1], "%x ", &att);  /* hex! */
                cfg.attr = (uchar) att;
                break;

            case K_WATTR:
                sscanf(words[1], "%x ", &att);  /* hex! */
                cfg.wattr = (uchar) att;
                break;

            case K_CATTR:
                sscanf(words[1], "%x ", &att);  /* hex! */
                cfg.cattr = (uchar) att;
                break;

            case K_BATTR:
                sscanf(words[1], "%x ", &att);  /* hex! */
                cfg.battr = att;
                break;

            case K_UTTR:
                sscanf(words[1], "%x ", &att);  /* hex! */
                cfg.uttr = att;
                break;

            case K_INIT_BAUD:
                cfg.initbaud = atoi(words[1]);
                break;

            case K_BIOS:
                cfg.bios = atoi(words[1]);
                break;

            case K_COST1200:
                sscanf(words[1], "%f ", &cfg.cost1200); /* float */
                break;

            case K_COST2400:
                sscanf(words[1], "%f ", &cfg.cost2400); /* float */
                break;

            case K_DUMB_MODEM:
                cfg.dumbmodem = atoi(words[1]);
                break;

            case K_READLLOG:
                cfg.readluser = atoi(words[1]);
                break;

            case K_READLOG:
                cfg.readuser = atoi(words[1]);
                break;

            case K_AIDEREADLOG:
                cfg.aidereaduser = atoi(words[1]);
                break;

            case K_ENTERSUR:
                cfg.entersur = atoi(words[1]);
                break;

            case K_DATESTAMP:
                if (strlen(words[1]) > 63)
                    illegal(ATOM1071);

                strcpy(cfg.datestamp, words[1]);
                break;

            case K_VDATESTAMP:
                if (strlen(words[1]) > 63)
                    illegal(ATOM1072);

                strcpy(cfg.vdatestamp, words[1]);
                break;

            case K_ENTEROK:
                cfg.unlogEnterOk = atoi(words[1]);
                break;

            case K_FORCELOGIN:
                cfg.forcelogin = atoi(words[1]);
                break;

            case K_MODERATE:
                cfg.moderate = atoi(words[1]);
                break;

            case K_HELPPATH:
                if (strlen(words[1]) > 63)
                    illegal(ATOM1073);

                strcpy(cfg.helppath, words[1]);
                break;

            case K_TEMPPATH:
                if (strlen(words[1]) > 63)
                    illegal(ATOM1074);

                strcpy(cfg.temppath, words[1]);
                break;

            case K_HOMEPATH:
                if (strlen(words[1]) > 63)
                    illegal(ATOM1075);

                strcpy(cfg.homepath, words[1]);
                break;

            case K_KILL:
                cfg.kill = atoi(words[1]);
                break;

            case K_LINEFEEDS:
                cfg.linefeeds = atoi(words[1]);
                break;

            case K_LOGINSTATS:
                cfg.loginstats = atoi(words[1]);
                break;

            case K_MAXBALANCE:
                sscanf(words[1], "%f ", &cfg.maxbalance);   /* float */
                break;

            case K_MAXLOGTAB:
                cfg.MAXLOGTAB = atoi(words[1]);
                break;

            case K_MESSAGE_ROOM:
                cfg.MessageRoom = atoi(words[1]);
                break;

            case K_NEWUSERAPP:
                if (strlen(words[1]) > 12)
                    illegal(ATOM1076);

                strcpy(cfg.newuserapp, words[1]);
                break;

            case K_PRIVATE:
                cfg.private = atoi(words[1]);
                break;

            case K_MAXTEXT:
				if (firstime)
					cfg.maxtext = atoi(words[1]);
                break;

            case K_MAX_WARN:
                cfg.maxwarn = atoi(words[1]);
                break;

            case K_MDATA:
                cfg.mdata = atoi(words[1]);
                if ((cfg.mdata < 1) || (cfg.mdata > 4))
                    illegal(ATOM1077);
                break;

            case K_MAXFILES:
				if (firstime)
					cfg.maxfiles = atoi(words[1]);
                break;

            case K_MSGPATH:
                if (strlen(words[1]) > 63)
                    illegal(ATOM1078);

                strcpy(cfg.msgpath, words[1]);
                break;

            case K_F6PASSWORD:
                if (strlen(words[1]) > 19)
                    illegal(ATOM1079);

                strcpy(cfg.f6pass, words[1]);
                break;

            case K_APPLICATIONS:
                if (strlen(words[1]) > 63)
                    illegal(ATOM1080);

                strcpy(cfg.aplpath, words[1]);
                break;

            case K_MESSAGEK:
				if (firstime)
					cfg.messagek = atoi(words[1]);
                break;

            case K_MODSETUP:
                if (strlen(words[1]) > 63)
                    illegal(ATOM1081);

                strcpy(cfg.modsetup, words[1]);
                break;

            case K_DIAL_INIT:
                if (strlen(words[1]) > 63)
                    illegal(ATOM1082);

                strcpy(cfg.dialsetup, words[1]);
                break;

            case K_DIAL_PREF:
                if (strlen(words[1]) > 20)
                    illegal(ATOM1083);

                strcpy(cfg.dialpref, words[1]);
                break;

            case K_NEWBAL:
                sscanf(words[1], "%f ", &cfg.newbal);   /* float */
                break;

            case K_SURNAMES:
                cfg.surnames = (atoi(words[1]) != 0);
                cfg.netsurname = (atoi(words[1]) == 2);
                break;

            case K_AIDEHALL:
                cfg.aidehall = atoi(words[1]);
                break;

            case K_NMESSAGES:
				if (firstime)
					cfg.nmessages = atoi(words[1]);
                break;

            case K_NODENAME:
                if (strlen(words[1]) > NAMESIZE)
                    illegal(ATOM1084);

                strcpy(cfg.nodeTitle, words[1]);
                break;

            case K_NODEREGION:
                if (strlen(words[1]) > NAMESIZE)
                    illegal(ATOM1085);

                strcpy(cfg.nodeRegion, words[1]);
                break;

            case K_NOPWECHO:
                cfg.nopwecho = (unsigned char) atoi(words[1]);
                break;

            case K_NULLS:
                cfg.nulls = atoi(words[1]);
                break;

            case K_OFFHOOK:
                cfg.offhook = atoi(words[1]);
                break;

            case K_OLDCOUNT:
                cfg.oldcount = atoi(words[1]);
                break;

            case K_PRINTER:
                if (strlen(words[1]) > 63)
                    illegal(ATOM1086);

                strcpy(cfg.printer, words[1]);
                break;

            case K_READOK:
                cfg.unlogReadOk = atoi(words[1]);
                break;

            case K_ROOMOK:
                cfg.nonAideRoomOk = atoi(words[1]);
                break;

            case K_ROOMTELL:
                cfg.roomtell = atoi(words[1]);
                break;

            case K_ROOMPATH:
                if (strlen(words[1]) > 63)
                    illegal(ATOM1087);

                strcpy(cfg.roompath, words[1]);
                break;

            case K_SUBHUBS:
                cfg.subhubs = atoi(words[1]);
                break;

            case K_TABS:
                cfg.tabs = atoi(words[1]);
                break;

            case K_TIMEOUT:
                cfg.timeout = atoi(words[1]);
                break;

            case K_TRAP:
                for (j = 1; j < count; j++) {
                    valid = FALSE;

                    for (k = 0; trapkeywords[k] != NULL; k++) {
                        sprintf(notkeyword, "!%s", trapkeywords[k]);

                        if (strcmpi(words[j], trapkeywords[k]) == SAMESTRING) {
                            valid = TRUE;

                            if (k == 0) {   /* ALL */
                                for (l = 0; l < 16; ++l)
                                    cfg.trapit[l] = TRUE;
                            } else
                                cfg.trapit[k] = TRUE;
                        } else if (strcmpi(words[j], notkeyword) == SAMESTRING) {
                            valid = TRUE;

                            if (k == 0) {   /* ALL */
                                for (l = 0; l < 16; ++l)
                                    cfg.trapit[l] = FALSE;
                            } else
                                cfg.trapit[k] = FALSE;
                        }
                    }

                    if (!valid) {
                        doccr();
                        cPrintf(ATOM1088
                        ATOM1089, words[j]);
                        doccr();
                    }
                }
                break;

            case K_TRAP_FILE:
                if (strlen(words[1]) > 63)
                    illegal(ATOM1090);

                strcpy(cfg.trapfile, words[1]);

                break;

            case K_UNLOGGEDBALANCE:
                sscanf(words[1], "%f ", &cfg.unlogbal); /* float */
                break;

            case K_UNLOGTIMEOUT:
                cfg.unlogtimeout = atoi(words[1]);
                break;

            case K_UPPERCASE:
                cfg.uppercase = atoi(words[1]);
                break;

            case K_USER:
                for (j = 0; j < 5; ++j)
                    cfg.user[j] = 0;

                for (j = 1; j < count; j++) {
                    valid = FALSE;

                    for (k = 0; userkeywords[k] != NULL; k++) {
                        if (strcmpi(words[j], userkeywords[k]) == SAMESTRING) {
                            valid = TRUE;
                            cfg.user[k] = TRUE;
                        }
                    }

                    if (!valid) {
                        doccr();
                        cPrintf(ATOM1091,
                        words[j]);
                        doccr();
                    }
                }
                break;

            case K_WIDTH:
                cfg.width = atoi(words[1]);
                break;

            case K_SYSOP:
                if (strlen(words[1]) > 63)
                    illegal(ATOM1092);

                strcpy(cfg.sysop, words[1]);
                break;

            case K_CREDIT_NAME:
                if (strlen(words[1]) > 63)
                    illegal(ATOM1093);

                strcpy(cfg.credit_name, words[1]);
                break;

            case K_SWAP_FILE:
                if (strlen(words[1]) > 63)
                    illegal(ATOM1125);

                strcpy(cfg.swapfile, words[1]);
                break;

			case K_SCREEN_SAVE:
				cfg.screen_save = atol(words[1]);
				break;

            default:
                cPrintf(ATOM1094, words[0]);
                doccr();
                break;
        }
    }

    fclose(fBuf);

    if (!*cfg.credit_name) {
        strcpy(cfg.credit_name, ATOM1095);
    }
    if (!*cfg.sysop) {
        strcpy(cfg.sysop, ATOM1096);
    }
    for (i = 0, valid = TRUE; i <= K_NWORDS; i++) {
        if (!found[i]) {
            cPrintf(ATOM1097,keywords[i]);
            valid = FALSE;
        }
    }

    if (!valid)
        illegal("");

	/* check directories */
	checkDir(cfg.homepath);
	checkDir(cfg.msgpath);
	checkDir(cfg.helppath);
	checkDir(cfg.temppath);
	checkDir(cfg.roompath);
	checkDir(cfg.aplpath);
	changedir(cfg.homepath);
	/* checkDir(cfg.transpath); */	/* what's this? */
}

/************************************************************************/
/*      RoomTabBld() -- build RAM index to ROOM.DAT, displays stats.    */
/************************************************************************/
void RoomTabBld(void)
{
    int slot;
    int roomCount = 0;

    doccr();
    doccr();
    cPrintf(ATOM1098);
    doccr();

    for (slot = 0; slot < MAXROOMS; slot++) {
        getRoom(slot);

        cPrintf(ATOM1099, slot);

        if (roomBuf.rbflags.INUSE)
            ++roomCount;

        noteRoom();
        putRoom(slot);
    }
    doccr();
    cPrintf(ATOM1100, roomCount, MAXROOMS);
    doccr();

}

/************************************************************************/
/*      showtypemsg() prints out what kind of message is being read     */
/************************************************************************/
void showtypemsg(ulong here)
{
#   ifdef DEBUG
    cPrintf("(%7lu)", msgBuf->mbheadLoc);
#   endif

    if (*msgBuf->mbcopy)
        cPrintf(ATOM1101, here);
    else {
        if (*msgBuf->mbto)
            cPrintf(ATOM1102, here);
        else if (*msgBuf->mbx == 'Y')
            cPrintf(ATOM1103, here);
        else if (*msgBuf->mbx == 'M')
            cPrintf(ATOM1104, here);
        else if (*msgBuf->mbgroup)
            cPrintf(ATOM1105, here);
        else
            cPrintf(ATOM1106, here);
    }
}

/************************************************************************/
/*      slidemsgTab() frees >howmany< slots at the beginning of the     */
/*      message table.                                                  */
/************************************************************************/
void slidemsgTab(int howmany)
{
    hmemcpy(&msgTab[howmany], &msgTab[0], (long)
    ((long) ((long) cfg.nmessages - (long) howmany) * (long) (sizeof(*msgTab)))
    );
}

/************************************************************************/
/*      zapGrpFile(), erase & reinitialize group file                   */
/************************************************************************/
void zapGrpFile(void)
{
    doccr();
    cPrintf(ATOM1107);
    doccr();

    setmem(&grpBuf, sizeof grpBuf, 0);

    strcpy(grpBuf.group[0].groupname, ATOM1108);
    grpBuf.group[0].g_inuse = 1;
    grpBuf.group[0].groupgen = 1;   /* Group Null's gen# is one      */
    /* everyone's a member at log-in */

    strcpy(grpBuf.group[1].groupname, ATOM1109);
    grpBuf.group[1].g_inuse = 1;
    grpBuf.group[1].groupgen = 1;

    putGroup();
}

/************************************************************************/
/*      zapHallFile(), erase & reinitialize hall file                   */
/************************************************************************/
void zapHallFile(void)
{
    doccr();
    cPrintf(ATOM1110);
    doccr();

    strcpy(hallBuf->hall[0].hallname, ATOM1111);
    hallBuf->hall[0].owned = 0; /* Hall is not owned     */

    hallBuf->hall[0].h_inuse = 1;
    hallBuf->hall[0].hroomflags[0].inhall = 1;  /* Lobby> in Root        */
    hallBuf->hall[0].hroomflags[1].inhall = 1;  /* Mail>  in Root        */
    hallBuf->hall[0].hroomflags[2].inhall = 1;  /* Aide)  in Root        */

    strcpy(hallBuf->hall[1].hallname, ATOM1112);
    hallBuf->hall[1].owned = 0; /* Hall is not owned     */

    hallBuf->hall[1].h_inuse = 1;
    hallBuf->hall[1].hroomflags[0].inhall = 1;  /* Lobby> in Maintenance */
    hallBuf->hall[1].hroomflags[1].inhall = 1;  /* Mail>  in Maintenance */
    hallBuf->hall[1].hroomflags[2].inhall = 1;  /* Aide)  in Maintenance */


    hallBuf->hall[0].hroomflags[2].window = 1;  /* Aide) is the window   */
    hallBuf->hall[1].hroomflags[2].window = 1;  /* Aide) is the window   */

    putHall();
}

/************************************************************************/
/*      zapLogFile() erases & re-initializes userlog.buf                */
/************************************************************************/
int zapLogFile(void)
{
    int i;

    /* clear RAM buffer out:                    */
    logBuf.lbflags.L_INUSE = FALSE;
    logBuf.lbflags.NOACCOUNT = FALSE;
    logBuf.lbflags.AIDE = FALSE;
    logBuf.lbflags.NETUSER = FALSE;
    logBuf.lbflags.NODE = FALSE;
    logBuf.lbflags.PERMANENT = FALSE;
    logBuf.lbflags.PROBLEM = FALSE;
    logBuf.lbflags.SYSOP = FALSE;
    logBuf.lbflags.ROOMTELL = FALSE;
    logBuf.lbflags.NOMAIL = FALSE;
    logBuf.lbflags.UNLISTED = FALSE;

    logBuf.callno = 0l;

    for (i = 0; i < NAMESIZE; i++) {
        logBuf.lbname[i] = 0;
        logBuf.lbin[i] = 0;
        logBuf.lbpw[i] = 0;
    }
    doccr();
    doccr();
    cPrintf(ATOM1113, cfg.MAXLOGTAB);
    doccr();

    /* write empty buffer all over file;        */
    for (i = 0; i < cfg.MAXLOGTAB; i++) {
        cPrintf(ATOM1114, i);
        putLog(&logBuf, i);
        logTab[i].ltcallno = logBuf.callno;
        logTab[i].ltlogSlot = i;
        logTab[i].ltnmhash = hash(logBuf.lbname);
        logTab[i].ltinhash = hash(logBuf.lbin);
        logTab[i].ltpwhash = hash(logBuf.lbpw);
    }
    doccr();
    return TRUE;
}

/************************************************************************/
/*      zapMsgFl() initializes message.buf                              */
/************************************************************************/
int zapMsgFile(void)
{
    int i;
    unsigned sect;
    unsigned char sectBuf[128];

    /* put null message in first sector... */
    sectBuf[0] = 0xFF;      /* */
    sectBuf[1] = DUMP;      /* \  To the dump                   */
    sectBuf[2] = '\0';      /* /  Attribute                     */
    sectBuf[3] = '1';       /* >                                */
    sectBuf[4] = '\0';      /* \  Message ID "1" MS-DOS style   */
    sectBuf[5] = 'M';       /* /         Null messsage          */
    sectBuf[6] = '\0';      /* */

    cfg.newest = cfg.oldest = 1l;

    cfg.catLoc = 7l;

    if (fwrite(sectBuf, 128, 1, msgfl) != 1) {
        cPrintf(ATOM1115);
        doccr();
    }
    for (i = 0; i < 128; i++)
        sectBuf[i] = 0;

    doccr();
    doccr();
    cPrintf(ATOM1116, cfg.messagek);
    doccr();
    for (sect = 1; sect < (cfg.messagek * 8); sect++) {
        cPrintf(ATOM1117, sect);
        if (fwrite(sectBuf, 128, 1, msgfl) != 1) {
            cPrintf(ATOM1118);
            doccr();
        }
    }
    return TRUE;
}

/************************************************************************/
/*      zapRoomFile() erases and re-initailizes ROOM.DAT                */
/************************************************************************/
int zapRoomFile(void)
{
    int i;

    roomBuf.rbflags.INUSE = FALSE;
    roomBuf.rbflags.PUBLIC = FALSE;
    roomBuf.rbflags.MSDOSDIR = FALSE;
    roomBuf.rbflags.PERMROOM = FALSE;
    roomBuf.rbflags.GROUPONLY = FALSE;
    roomBuf.rbflags.READONLY = FALSE;
    roomBuf.rbflags.SHARED = FALSE;
    roomBuf.rbflags.MODERATED = FALSE;
    roomBuf.rbflags.DOWNONLY = FALSE;

    roomBuf.rbgen = 0;
    roomBuf.rbdirname[0] = 0;
    roomBuf.rbname[0] = 0;
    roomBuf.rbroomtell[0] = 0;
    roomBuf.rbgrpgen = 0;
    roomBuf.rbgrpno = 0;

    doccr();
    doccr();
    cPrintf(ATOM1119, MAXROOMS);
    doccr();

    for (i = 0; i < MAXROOMS; i++) {
        cPrintf(ATOM1120, i);
        putRoom(i);
        noteRoom();
    }

    /* Lobby> always exists -- guarantees us a place to stand! */
    thisRoom = 0;
    strcpy(roomBuf.rbname, ATOM1121);
    roomBuf.rbflags.PERMROOM = TRUE;
    roomBuf.rbflags.PUBLIC = TRUE;
    roomBuf.rbflags.INUSE = TRUE;

    putRoom(LOBBY);
    noteRoom();

    /* Mail> is also permanent...       */
    thisRoom = MAILROOM;
    strcpy(roomBuf.rbname, ATOM1122);
    roomBuf.rbflags.PERMROOM = TRUE;
    roomBuf.rbflags.PUBLIC = TRUE;
    roomBuf.rbflags.INUSE = TRUE;

    putRoom(MAILROOM);
    noteRoom();

    /* Aide) also...                    */
    thisRoom = AIDEROOM;
    strcpy(roomBuf.rbname, ATOM1123);
    roomBuf.rbflags.PERMROOM = TRUE;
    roomBuf.rbflags.PUBLIC = FALSE;
    roomBuf.rbflags.INUSE = TRUE;

    putRoom(AIDEROOM);
    noteRoom();

    /* Dump> also...                    */
    thisRoom = DUMP;
    strcpy(roomBuf.rbname, ATOM1124);
    roomBuf.rbflags.PERMROOM = TRUE;
    roomBuf.rbflags.PUBLIC = TRUE;
    roomBuf.rbflags.INUSE = TRUE;

    putRoom(DUMP);
    noteRoom();

    return TRUE;
}

/* EOF */
