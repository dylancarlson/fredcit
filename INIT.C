/* --------------------------------------------------------------------
 *  INIT.C                    Citadel
 * --------------------------------------------------------------------
 *  Routines to initialize or exit Citadel
 * --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/INIT.C_V  $
 * 
 *    Rev 1.29   01 Nov 1991 11:20:14   FJM
 * Added gl_ structures
 *
 *    Rev 1.28   11 Jun 1991  8:17:54   FJM
 *
 *    Rev 1.27   06 Jun 1991  9:19:10   FJM
 *
 *    Rev 1.26   27 May 1991 11:42:46   FJM
 *
 *    Rev 1.25   22 May 1991  2:16:54   FJM
 * Made memory check optional.
 *
 *    Rev 1.24   06 May 1991 17:28:44   FJM
 *
 *    Rev 1.23   19 Apr 1991 23:38:56   FJM
 * No change.
 *
 *    Rev 1.22   19 Apr 1991 23:26:52   FJM
 *
 *    Rev 1.21   17 Apr 1991 12:55:22   FJM
 *
 *    Rev 1.19   04 Apr 1991 14:13:06   FJM
 *    Rev 1.4   11 Jan 1991 12:44:32   FJM
 *    Rev 1.0   06 Jan 1991 12:45:50   FJM
 * Porting, formating, fixes and misc changes.
 *
 * -------------------------------------------------------------------- */

/* --------------------------------------------------------------------
 *                              Contents
 * --------------------------------------------------------------------
 *  crashout()      Fatal system error
 *  exitcitadel()   Done with cit, time to leave
 *  asciitable()    initializes the ascii translation table
 *  initCitadel()   Load up data files and open everything.
 *  readTables()    loads all tables into ram
 *  freeTables()	deallocate tables from memory
 *  writeTables()   writes all system tables to disk
 *  allocateTables()allocate table space
 * -------------------------------------------------------------------- */

/* --------------------------------------------------------------------
 *  Includes
 * -------------------------------------------------------------------- */

#define INIT

#include <signal.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>

/* Cit */
#include "ctdl.h"

#ifndef ATARI_ST
#include <bios.h>
//#include <conio.h>
#include <dos.h>
#include <io.h>
#include <alloc.h>
#endif

#include "proto.h"
#include "global.h"

/* -------------------------------------------------------------------- */
/*  External data                                                       */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Static Data definitions                                             */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  asciitable()    initializes the ascii translation table             */
/* -------------------------------------------------------------------- */
void asciitable(void)
{
#ifdef OLD_ASCII_TBL
    unsigned char c;

    /* initialize input character-translation table:  */

    /* control chars -> nulls     */
    for (c = 0; c < '\40'; c++)
        filter[c] = '\0';

    /* pass printing chars        */
    for (c = '\40'; c < FILT_LEN; c++)
        filter[c] = c;

    filter[1] = 1;      /* ctrl-a    0x01 = ctrl-a    0x01   */
    filter[27] = 27;        /* special   0x1b = special   0x1b   */
    filter[8] = 8;      /* backspace 0x08 = backspace 0x08   */
    filter[0x7f] = 8;       /* del       0x7f = backspace 0x08   */
    filter[19] = 'P';       /* xoff      0x13 = 'P'       0x50   */
    filter['\r'] = '\n';    /* '\r'      0x0d = NEWLINE   0x0a   */
    filter['\t'] = '\t';    /* '\t'      0x09 = '\t'      0x09   */
    filter[10] = NULL;      /* newline   0x0a = null      0x00   */
    filter[15] = 'N';       /* ctrlo     0x0f = 'N'       0x4e   */
    filter[26] = 26;        /* ctrlz     0x1a = ctrlz     0x1a   */
#endif
}

/* -------------------------------------------------------------------- */
/*  initCitadel()   Load up data files and open everything.             */
/* -------------------------------------------------------------------- */
void initCitadel(void)
{
    /* FILE *fd, *fp; */

    char *grpFile, *hallFile, *logFile, *msgFile, *roomFile;
    char scratch[80];

    /* lets ignore ^C's  */
    signal(SIGINT, ctrl_c);
    randomize();        /* we will use random numbers */
    asciitable();

    /* moved all allocation to allocateTables (called from readTables()) */

    if (!readTables() || strcmp(compdate, cfg.compdate) ||
    strcmp(comptime, cfg.comptime)) {
        if (msgTab) {
            freeTables();
        }
		if (!parm.door)
			delay(3000);
        cls();
        cCPrintf("Configuring Citadel - Please wait");
        doccr();
        configcit();
    }
    portInit();

    setscreen();

    /* update25();	*/
	do_idle(0);

    if (cfg.msgpath[(strlen(cfg.msgpath) - 1)] == '\\')
        cfg.msgpath[(strlen(cfg.msgpath) - 1)] = '\0';

    sprintf(scratch, "%s\\%s", cfg.msgpath, "msg.dat");

    /* open message files: */
    grpFile = "grp.dat";
    hallFile = "hall.dat";
    logFile = "log.dat";
    msgFile = scratch;
    roomFile = "room.dat";

    openFile(grpFile, &grpfl);
    openFile(hallFile, &hallfl);
    openFile(logFile, &logfl);
    openFile(msgFile, &msgfl);
    openFile(roomFile, &roomfl);

    /* open Trap file */
    trapfl = fopen(cfg.trapfile, "a+");

    sprintf(msgBuf->mbtext, "%s Started", softname);
    trap(msgBuf->mbtext, T_SYSOP);

    getGroup();
    getHall();

    if (cfg.accounting)
        readaccount();      /* read in accounting data */
    readprotocols();

    getRoom(LOBBY);     /* load Lobby>  */
    Initport();
    Initport();
    whichIO = MODEM;

    /* record when we put system up */
    uptimestamp=cit_time();

    cls();
    setdefaultconfig();
    /* update25();	*/
	do_idle(0);
    setalloldrooms();
    roomtalley();
	
		

}

#define MAXRW   3000

/* -------------------------------------------------------------------- */
/*  readMsgTable()     Avoid the 64K limit. (stupid segments)           */
/* -------------------------------------------------------------------- */
int readMsgTab(void)
{
    FILE *fd;
    int i;
    char HUGE *tmp;
    char temp[80];

    /* read message table */
    sprintf(temp, "%s\\%s", cfg.homepath, "msg.tab");

    tmp = (char HUGE *) msgTab;

    if ((fd = fopen(temp, "rb")) == NULL)
        return (FALSE);

    for (i = 0; i < cfg.nmessages / MAXRW; i++) {
        if (fread((void FAR *) (tmp), sizeof(struct messagetable), MAXRW, fd)
        != MAXRW) {
            fclose(fd);
            return FALSE;
        }
        tmp += (sizeof(struct messagetable) * MAXRW);
    }

    i = cfg.nmessages % MAXRW;
    if (i) {
        if (fread((void FAR *) (tmp), sizeof(struct messagetable), i, fd)
        != i) {
            fclose(fd);
            return FALSE;
        }
    }
    fclose(fd);
    unlink(temp);

    return TRUE;
}

/* -------------------------------------------------------------------- */
/*  writeMsgTable()     Avoid the 64K limit. (stupid segments)          */
/* -------------------------------------------------------------------- */
void writeMsgTab(void)
{
    FILE *fd;
    int i;
    char HUGE *tmp;
    char temp[80];

    sprintf(temp, "%s\\%s", cfg.homepath, "msg.tab");

    tmp = (char HUGE *) msgTab;

    if ((fd = fopen(temp, "wb")) == NULL)
        return;

    for (i = 0; i < cfg.nmessages / MAXRW; i++) {
        if (fwrite((void FAR *) (tmp), sizeof(struct messagetable), MAXRW, fd)
        != MAXRW) {
            fclose(fd);
        }
        tmp += (sizeof(struct messagetable) * MAXRW);
    }

    i = cfg.nmessages % MAXRW;
    if (i) {
        if (fwrite((void FAR *) (tmp), sizeof(struct messagetable), i, fd)
        != i) {
            fclose(fd);
        }
    }
    fclose(fd);
}

/************************************************************************/
/*      readTables()  loads all tables into ram                         */
/************************************************************************/
readTables()
{
    FILE *fd;

    getcwd(etcpath, 64);

    /*
     * ETC.tab
     */
    if ((fd = fopen("etc.tab", "rb")) == NULL)
        return (FALSE);
    if (!fread(&cfg, sizeof cfg, 1, fd)) {
        fclose(fd);
        return FALSE;
    }
    fclose(fd);
    unlink("etc.tab");
    allocateTables();

    changedir(cfg.homepath);

    if (logTab == NULL)
        crashout("Error allocating logTab \n");
    if (msgTab == NULL)
        crashout("Error allocating msgTab \n");

    /*
     * LOG.TAB
     */
    if ((fd = fopen("log.tab", "rb")) == NULL)
        return (FALSE);
    if (!fread(logTab, sizeof(struct lTable), cfg.MAXLOGTAB, fd)) {
        fclose(fd);
        return FALSE;
    }
    fclose(fd);
    unlink("log.tab");

    /*
     * MSG.TAB
     */
    if (readMsgTab() == FALSE)
        return FALSE;

    /*
     * ROOM.TAB
     */
    if ((fd = fopen("room.tab", "rb")) == NULL)
        return (FALSE);
    if (!fread(roomTab, sizeof(struct rTable), MAXROOMS, fd)) {
        fclose(fd);
        return FALSE;
    }
    fclose(fd);
    unlink("room.tab");

    readcron();         /* was being read before homepath set! */
    readCrontab();
    unlink("cron.tab");

    return (TRUE);
}


/* -------------------------------------------------------------------- */
/*  crashout()      Fatal system error                                  */
/* -------------------------------------------------------------------- */
void crashout(char *message)
{
    FILE *fd;           /* Record some crash data */

    Hangup();

    fcloseall();

    fd = fopen("crash.cit", "w");
    fprintf(fd, message);
    fclose(fd);

    /* writeTables(); */		/* this could write tables all over the disk! */

    cfg.attr = 7;       /* exit with white letters */

    position(0, 0);
    cPrintf("F\na\nt\na\nl\n \nS\ny\ns\nt\ne\nm\n \nC\nr\na\ns\nh\n");
    cPrintf(" %s\n", message);

    drop_dtr();

    portExit();

    freeTables();

    exit(1);
}

/* -------------------------------------------------------------------- */
/*  exitcitadel()   Done with cit, time to leave                        */
/* -------------------------------------------------------------------- */

extern void interrupt ( *oldhandler)(void);

void exitcitadel(void)
{
    if  (loggedIn)
        terminate( /* hangUp == */ TRUE, FALSE);

    drop_dtr();         /* turn DTR off */

    putGroup();         /* save group table */
    putHall();          /* save hall table  */

    writeTables();

    sprintf(msgBuf->mbtext, "%s Terminated", softname);
    trap(msgBuf->mbtext, T_SYSOP);

    /* close all files */
    fcloseall();

    cfg.attr = 7;       /* exit with white letters */
    cls();

    drop_dtr();

    portExit();

    freeTables();

    if (gmode() != 7) {			/* what the heck does this do? */
        outp(0x3d9, 0);
    }
    setvect(24, oldhandler);
	
    exit(ExitToMsdos - 1);
}

/************************************************************************
 *              freeTables()            Free all allocated tables
 ************************************************************************/

void freeTables(void)
{
    if  (msgTab)
        farfree((void FAR *) msgTab);
    msgTab = NULL;
	if (accountBuf)
		farfree(accountBuf);
	accountBuf = NULL;
    if (logTab)
        farfree(logTab);
    logTab = NULL;
    if (roomTab)
        farfree(roomTab);
    roomTab = NULL;
    if (extCmd)
        farfree(extCmd);
    extCmd = NULL;
    if (extrn)
        farfree(extrn);
    extrn = NULL;
    if (edit)
        farfree(edit);
    edit = NULL;
    if (hallBuf)
        farfree(hallBuf);
    hallBuf = NULL;
    if (msgBuf)
        farfree(msgBuf);
    msgBuf = NULL;
    if (msgBuf2)
        farfree(msgBuf2);
    msgBuf2 = NULL;
    if (parm.memcheck && (farheapcheck() < 0))
        mPrintf("memory corrupted 1\n");
}

/************************************************************************/
/*      writeTables()  stores all tables to disk                        */
/************************************************************************/
void writeTables(void)
{
    FILE *fd;

    changedir(etcpath);

    if ((fd = fopen("etc.tab", "wb")) == NULL) {
        crashout("Can't make Etc.tab");
    }
    strcpy(cfg.compdate, compdate);
    strcpy(cfg.comptime, comptime);
    /* write out Etc.tab */
    fwrite(&cfg, sizeof cfg, 1, fd);
    fclose(fd);

    changedir(cfg.homepath);

    if ((fd = fopen("log.tab", "wb")) == NULL) {
        crashout("Can't make Log.tab");
    }
    /* write out Log.tab */
    fwrite(logTab, sizeof(struct lTable), cfg.MAXLOGTAB, fd);
    fclose(fd);

    writeMsgTab();

    if ((fd = fopen("room.tab", "wb")) == NULL) {
        crashout("Can't make Room.tab");
    }
    /* write out Room.tab */
    fwrite(roomTab, sizeof(struct rTable), MAXROOMS, fd);
    fclose(fd);

    writeCrontab();

    changedir(etcpath);
}

/************************************************************************
 *    allocateTables()   allocate msgTab, logTab and allocated tables
 *                                              see tableOut, tableIn in APPLIC.C
 ************************************************************************/

void allocateTables(void)
{
    if  ((edit = farcalloc(MAXEXTERN, sizeof(struct ext_editor))) == NULL) {
        crashout("Can not allocate external editors");
    }
    if ((hallBuf = farcalloc(1L, sizeof(struct hallBuffer))) == NULL) {
        crashout("Can not allocate memory for halls");
    }
    if ((msgBuf = farcalloc(1L, sizeof(struct msgB))) == NULL) {
        crashout("Can not allocate memory for message buffer 1");
    }
    if ((msgBuf2 = farcalloc(1L, sizeof(struct msgB))) == NULL) {
        crashout("Can not allocate memory for message buffer 2");
    }
    if ((extrn = farcalloc(MAXEXTERN, sizeof(struct ext_prot))) == NULL) {
        crashout("Can not allocate memory for external protocol");
    }
    if ((extCmd = farcalloc(MAXEXTERN, sizeof(struct ext_other))) == NULL) {
        crashout("Can not allocate memory for external commands");
    }
    if ((roomTab = farcalloc(MAXROOMS, sizeof(struct rTable))) == NULL) {
        crashout("Can not allocate memory for other extern commands");
    }
    logTab = farcalloc((long) cfg.MAXLOGTAB + 1, (long) sizeof(struct lTable));
    if (!logTab) {
        crashout("Can not allocate memory for user log");
    }
    accountBuf = farcalloc(1L, (long) sizeof(struct accountBuffer));
    if (!accountBuf) {
        crashout("Can not allocate memory for user accounting");
    }
    msgTab = farcalloc((long) cfg.nmessages + 1, (long) sizeof(struct messagetable));
    if (!msgTab) {
        crashout("Can not allocate memory for message base");
    }
}


