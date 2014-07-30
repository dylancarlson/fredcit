/************************************************************************/
/*                              aplic.c                                 */
/*                    Application code for Citadel                      */
/************************************************************************/

#include <string.h>

#include "ctdl.h"

#ifndef ATARI_ST
#include <dos.h>
#include <alloc.h>
#include <process.h>
#endif

#include "proto.h"
#include "global.h"
#include "swap.h"
#include "apstruct.h"

#pragma warn -ucp

/************************************************************************
 *                              Contents
 *
 *      aplreadmess()           read message in from application
 *
 *      apsystem()              turns off interupts and makes
 *                              a system call
 *
 *      ExeAplic()              gets name of aplication and executes
 *
 *      readuserin()            reads userdati.apl from disk
 *
 *      shellescape()           handles the sysop '!' shell command
 *
 *      tableIn()               allocates RAM and reads log and msg
 *                              and tab files into RAM
 *
 *      tableOut()              writes msg and log tab files to disk
 *                              and frees RAM
 *
 *      writeDoorFiles()        write door interface files
 ************************************************************************/


/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/APLIC.C_V  $
 * 
 *    Rev 1.46   01 Nov 1991 11:19:40   FJM
 * Added gl_ structures
 *
 *    Rev 1.45   08 Jul 1991 16:17:48   FJM
 *
 *    Rev 1.44   18 May 1991  9:31:02   FJM
 * Better checking for blank APL messages.
 *
 *    Rev 1.43   06 May 1991 17:28:02   FJM
 * Removed old prompt code.
 *
 *    Rev 1.40   17 Apr 1991 12:54:34   FJM
 * Changes to farheapcheck() message.
 *
 *    Rev 1.39   10 Apr 1991  8:43:24   FJM
 * Prevent posting 0 length MESSAGE.APL files.
 *
 *    Rev 1.38   04 Apr 1991 14:12:02   FJM
 *    Rev 1.34   10 Feb 1991 17:32:12   FJM
 * Reveresed sense of chat & bells in output.apl again.
 *
 *    Rev 1.32   05 Feb 1991 14:30:18   FJM
 * Reversed bells and chat in the output.apl
 *
 *    Rev 1.30   19 Jan 1991 14:14:24   FJM
 *    Rev 1.27   18 Jan 1991 16:48:48   FJM
 * Added #SWAPFILE
 *
 *    Rev 1.25   13 Jan 1991  0:30:00   FJM
 * Name overflow fixes.
 *
 *    Rev 1.23   11 Jan 1991 12:42:34   FJM
 *    Rev 1.22   06 Jan 1991 16:49:02   FJM
 * Seperated out operations into readDoorFiles()
 *
 *    Rev 1.19   06 Jan 1991 12:42:34   FJM
 *    Rev 1.18   27 Dec 1990 20:16:28   FJM
 * Changes for porting.
 *
 *    Rev 1.14   22 Dec 1990 13:38:52   FJM
 *    Rev 1.8   16 Dec 1990 18:12:36   FJM
 *    Rev 1.6   07 Dec 1990 23:09:54   FJM
 *    Rev 1.4   23 Nov 1990 13:22:02   FJM
 *    Rev 1.2   17 Nov 1990 16:11:30   FJM
 * Added version control log header
 * -------------------------------------------------------------------- */

/* --------------------------------------------------------------------
 *  History:
 *
 *  03/08/90    {zm}    Add 'softname' as the name of the software.
 *  03/17/90    FJM     added checks for farfree()
 *  03/19/90    FJM     Linted & partial cleanup
 *  03/27/90    FJM     Removed debug check for int 0x22
 *  07/08/90    FJM     Removed putenv() from shellescape() to save shell
 *                      memory.
 *  09/14/90    FJM     Fixed bug where amPrintf() could double close the
 *                      file.
 *  11/04/90    FJM     Added extremly large supershells.
 *
 * -------------------------------------------------------------------- */

/************************************************************************/
/*              External declarations in APLIC.C                        */
/************************************************************************/

#ifdef OLD_SWAP
static int tableOut(void);
static void tableIn(void);

#endif
static int write_rbbs(char *fname, struct apldata * data, char *sysop1,
char *city, char *state);
static int write_rbbs_name(FILE * fp, char *name);
static int aplreadmess(void);
static void readuserin(void);
static void writeDoorFiles(void);
static void writeAplFile(void);
static void readAplFile(void);

#define     NUM_SAVE    15

static int saveem[] = {
	0x00, 0x03, 0x22, 0x34,
    0x35, 0x36, 0x37, 0x38,
    0x39, 0x3A, 0x3B, 0x3C,
    0x3D, 0x3E, 0x3F
};

static void (interrupt FAR * int_save[NUM_SAVE]) (),
(interrupt FAR * int_temp[NUM_SAVE]) ();

/************************************************************************/
/*              External variable definitions for APLIC.C               */
/************************************************************************/

/************************************************************************/
/*     aplreadmess()  read message in from application                  */
/************************************************************************/
static int aplreadmess(void)
{
    FILE *fd;
    int roomno, in = FALSE;

    if (filexists("msginfo.apl")) {
        in = TRUE;
        if ((fd = fopen("msginfo.apl", "rb")) == NULL)
            return ERROR;
        if (fread(&mesginfo, sizeof(mesginfo), 1, fd) != 1) {
            fclose(fd);
            return ERROR;
        }
        fclose(fd);
        if ((roomno = roomExists(mesginfo.roomName)) == ERROR) {
            cPrintf(" AP: No room \"%s\"! \n", mesginfo.roomName);
            return ERROR;
        }
    }
    if ((fd = fopen("message.apl", "rb")) == NULL)
        return ERROR;
    clearmsgbuf();
    GetStr(fd, msgBuf->mbtext, cfg.maxtext);
    fclose(fd);

    if (in && mesginfo.author[0])
        strcpy(msgBuf->mbauth, mesginfo.author);
    else
        strcpy(msgBuf->mbauth, cfg.nodeTitle);

    msgBuf->mbroomno = (uchar) (in ? roomno : thisRoom);

	if (!strblank(msgBuf->mbtext)) {	/* must include at least 1 text character! */
		putMessage();
		noteMessage();
	}

    return TRUE;
}

/************************************************************************/
/*      ExeAplic() gets the name of an aplication and executes it.      */
/************************************************************************/
void ExeAplic(void)
{
    doCR();
    doCR();

    if (!roomBuf.rbflags.APLIC) {
        mPrintf("  -- Room has no application.\n\n");
        changedir(cfg.homepath);
    } else
        roomFmtRun(roomBuf.rbaplic);
    return;
}

/************************************************************************
 *      roomFmtRun - run a command in normal room command format
 *
 *      command arguments:
 *
 *              %0      command name text
 *              %1      port: LOCAL|COM1|COM2|COM3|COM4
 *              %2      baud: 300|1200|2400|4800|9600|19200|38400
 *              %3      sysop name
 *              %4      user name
 *
 *      See apsystem() for allowed command prefixes.
 *
 ************************************************************************/

void roomFmtRun(char *cmd)
{
    char comm[5];
    char stuff[100];

    if (changedir(cfg.aplpath) == ERROR) {
        mPrintf("  -- Can't find application directory.\n\n");
    } else {
        sprintf(comm, "COM%d", cfg.mdata);
        sprintf(stuff, "%s %s %d %d %s",
        cmd,
        onConsole ? "LOCAL" : comm,
        onConsole ? 2400 : bauds[speed],
        gl_user.sysop,
        logBuf.lbname);

        apsystem(stuff);
    }
    changedir(cfg.homepath);
    return;
}

/************************************************************************
 *
 *      extFmtRun - run a command in normal external command format
 *                                      used by editText()
 *
 *      command parameter substitutions:
 *
 *              %f  file name.
 *              %p  port: 0-4   (0 is local)
 *              %b  baud: 300|1200|2400|4800|9600|19200|38400
 *              %a  application file path.
 *              %n  user login name (may be more then one word).
 *              %s  sysop flag (1 if sysop, 0 if not)
 *
 *      See apsystem() for allowed command prefixes.
 *
 ************************************************************************/

void extFmtRun(char *cmd, char *file)
{
    char comm[5];
    char stuff[100];
    label dspeed;

    if (changedir(cfg.aplpath) == ERROR) {
        mPrintf("  -- Can't find application directory.\n\n");
    } else {
        sprintf(comm, "%d", onConsole ? 0 : cfg.mdata);
        sprintf(dspeed, "%d", bauds[speed]);
        sformat(stuff, cmd, "fpbans", file, comm, dspeed,
        cfg.aplpath, logBuf.lbname, gl_user.sysop ? "1" : "0");
        apsystem(stuff);
        if (debug)
            cPrintf("(%s)", stuff);
        changedir(cfg.homepath);
    }
    return;
}

/************************************************************************/
/*      readuserin()  reads userdati.apl from disk                      */
/************************************************************************/
static void readuserin(void)
{
    FILE *fd;

    if (filexists("userdati.apl")) {
        if ((fd = fopen("userdati.apl", "rb")) == NULL)
            return;
        if (fread(&userdat, sizeof(userdat), 1, fd) != 1)
            return;
        strcpy(logBuf.surname, userdat.apl_surname);
        logBuf.lbflags.EXPERT = userdat.apl_expert;
        logBuf.credits = userdat.apl_credits;
        cfg.noChat = userdat.apl_noChat;
        cfg.noBells = userdat.apl_noBells;
        if (userdat.apl_reserved_2)
            logBuf.groups[1] = grpBuf.group[1].groupgen;
        else
            logBuf.groups[1] = grpBuf.group[1].groupgen - 1;
        gl_user.twit = userdat.apl_twit;
    }
    fclose(fd);
}

/************************************************************************/
/*      shellescape()  handles the sysop '!' shell command              */
/************************************************************************/
void shellescape(char super)
{
    char command[80];

    changedir(roomBuf.rbflags.MSDOSDIR ? roomBuf.rbdirname : cfg.homepath);

    sprintf(command, "%s%s", super ? "!" : "", getenv("COMSPEC"));

    apsystem(command);

    /* putenv(oldprompt); */

    /* update25();	*/
	do_idle(0);

    changedir(cfg.homepath);
}

#ifdef OLD_SWAP
/************************************************************************/
/* tableIn   allocates RAM and reads log and msg tab file into RAM      */
/************************************************************************/
static void tableIn(void)
{
    FILE *fd;
    char scratch[64];

    doCR();
    mPrintf("Restoring system variables, please wait.");
    doCR();

    sprintf(scratch, "%s\\%s", cfg.temppath, "fcit.tmp");

    if ((fd = fopen(scratch, "rb")) == NULL) {
        mPrintf("\n Fatal System Crash!\n System tables destroyed!");
        crashout("Log table lost in application");
    }
    allocateTables();

    if (!readMsgTab()) {
        mPrintf("\n Fatal System Crash!\n Message table destroyed!");
        crashout("Message table lost in application");
    }
    if (!fread(logTab, (sizeof(*logTab) * cfg.MAXLOGTAB), 1, fd)) {
        mPrintf("\n Fatal System Crash!\n Log table destroyed!");
        crashout("Log table lost in application");
    }
    fread(roomTab, sizeof(struct rTable), MAXROOMS, fd);
    fread(extCmd, sizeof(struct ext_other), MAXEXTERN, fd);
    fread(extrn, sizeof(struct ext_prot), MAXEXTERN, fd);
    fread(msgBuf2, sizeof(struct msgB), 1, fd);
    fread(msgBuf, sizeof(struct msgB), 1, fd);
    fread(hallBuf, sizeof(struct hallBuffer), 1, fd);
    fread(edit, sizeof(struct ext_editor), MAXEXTERN, fd);

    fclose(fd);
    unlink(scratch);
}

/************************************************************************/
/* tableOut   writes msg and log tab files to disk and frees RAM        */
/************************************************************************/
static int tableOut(void)
{
    FILE *fd;
    char scratch[64];

    mPrintf("Saving system variables, please wait.");
    doCR();
    if (cfg.homepath[(strlen(cfg.homepath) - 1)] == '\\')
        cfg.homepath[(strlen(cfg.homepath) - 1)] = '\0';

    sprintf(scratch, "%s\\%s", cfg.temppath, "fcit.tmp");

    if ((fd = fopen(scratch, "wb")) == NULL) {
        mPrintf("Can not save system tables!\n ");
        return ERROR;
    }
    /* write out Msg.tab */
    writeMsgTab();
    /* write out Log.tab */
    fwrite(logTab, (sizeof(*logTab) * cfg.MAXLOGTAB), 1, fd);

    fwrite(roomTab, sizeof(struct rTable), MAXROOMS, fd);
    fwrite(extCmd, sizeof(struct ext_other), MAXEXTERN, fd);
    fwrite(extrn, sizeof(struct ext_prot), MAXEXTERN, fd);
    fwrite(msgBuf2, sizeof(struct msgB), 1, fd);
    fwrite(msgBuf, sizeof(struct msgB), 1, fd);
    fwrite(hallBuf, sizeof(struct hallBuffer), 1, fd);
    fwrite(edit, sizeof(struct ext_editor), MAXEXTERN, fd);
    fclose(fd);
    freeTables();
    return TRUE;
}

#endif

/************************************************************************
 *
 * readDoorFiles() - read in application door interface files
 *
 ************************************************************************/

void readDoorFiles(int door)
{
    char pathbuf[80];
    char path[64];
	
    getcwd(path, 64);
	changedir(cfg.aplpath);
    if (filexists("readme.apl")) {
        dumpf("readme.apl");
        doCR();
    }
    if (filexists("userdati.apl"))
        readuserin();
    if (filexists("input.apl"))
        readAplFile();

    if (filexists("message.apl") && readMessage)
        aplreadmess();

    /* delete unneeded files */
    if (door) {
        unlink("userdati.apl");
        unlink("userdato.apl");
        unlink("output.apl");
        unlink("input.apl");
    }
    if (readMessage)
        unlink("message.apl");
    if (readMessage)
        unlink("msginfo.apl");
    unlink("readme.apl");
    if (door) {
        sprintf(pathbuf, "DORINFO%d.DEF", userdat.apl_com);
        unlink(pathbuf);
    }
    readMessage = TRUE;
	changedir(path);
}

/************************************************************************
 * apsystem() turns off interupts and makes a system call
 *
 * Creates files:
 *
 *    DORINFO?.DEF    QBBS/RBBS door info file (in aplic dir).
 *    USERDATO.APL    Old DragCit style door info (see struct apldata).
 *
 * Reads files:
 *
 *    USERDATI.APL    Old DragCit style door response file.
 *                    (see struct apldata) in applic.h
 *
 * Allowed command prefixes:
 *
 *         @       Don't save & clear screen
 *         $       Use DOS COMMAND.COM to run command (allows path, batch files)
 *         !       Use supershell (all but about 2.5k free!)
 *         ?       Don't write dorinfo?.def, userdato.apl
 *
 ************************************************************************/
void apsystem(cmd)
char *cmd;
{
    int clearit = TRUE, superit = FALSE, batch = FALSE, door = TRUE;
    char pathbuf[80];
    char cmdline[128];
    static char *words[256];
    int count, i, i2, AideFlOpen;
    int rc = 0;
    unsigned char exe_ret = 0;

    while (*cmd == '!' || *cmd == '@' || *cmd == '$' || *cmd == '?') {
        if (*cmd == '!')
            superit = TRUE;
        else if (*cmd == '@')
            clearit = FALSE;
        else if (*cmd == '$')
            batch = TRUE;
        else if (*cmd == '?')
            door = FALSE;
        cmd++;
    }
#ifdef OLD_SWAP

    if (superit)
        if (tableOut() == ERROR)
            return;
#endif
    if (disabled)
        drop_dtr();

    portExit();

    /* close all files */
    /* was bug in amPrintf() double closing file here */
    if (aideFl != NULL) {
        sprintf(pathbuf, "%s\\%s", cfg.temppath, "aidemsg.tmp");
        AideFlOpen = TRUE;
    } else
        AideFlOpen = FALSE;

    fcloseall();

    if (AideFlOpen && (aideFl = fopen(pathbuf, "a")) == NULL) {
        crashout("Can not open AIDEMSG.TMP!");
    }
    if (clearit) {
        save_screen();
        cls();
    }
    if (debug)
        cPrintf("(%s)\n", cmd);

    if (stricmp(cmd, getenv("COMSPEC")) == SAMESTRING)
        cPrintf("Use the EXIT command to return to %s\n", softname);
    else if (door)
        writeDoorFiles();   /* write door interface files */

    if (!batch) {
        count = parse_it(words, cmd);
        words[count] = NULL;
    }
    /*
     * Save the Floating point emulator interupts & the overlay interupt.
     */
    for (i = 0; i < NUM_SAVE; i++) {
        int_save[i] = getvect(saveem[i]);
    }

    if (superit) {
        if (batch) {
            sprintf(cmdline, "/C %s", cmd);
            rc = swap(getenv("COMSPEC"), cmdline, &exe_ret, cfg.swapfile);
        } else {
        /* rebuild command line */
            if (words[1]) {
                strcpy(cmdline, words[1]);
                for (i = 2; words[i]; ++i) {
                    strcat(cmdline, " ");
                    strcat(cmdline, words[i]);
                }
            } else {
                *cmdline = '\0';
            }

        /* Execute program, DOS search order is: .COM, .EXE, .BAT */

        /* no changes, extension fully specified */
            if (strchr(words[0], '.')) {
                sprintf(pathbuf, "%s", words[0]);
                rc = swap(pathbuf, cmdline, &exe_ret, cfg.swapfile);
            } else {
        /* try as a .COM */
                sprintf(pathbuf, "%s.COM", words[0]);
                rc = swap(pathbuf, cmdline, &exe_ret, cfg.swapfile);

        /* try as a .EXE if failed */
                if (rc && (exe_ret == FILE_NOT_FOUND)) {
                    exe_ret = 0;
                    sprintf(pathbuf, "%s.EXE", words[0]);
                    rc = swap(pathbuf, cmdline, &exe_ret, cfg.swapfile);
                }
            }
        }
    } else {
        if (batch) {
            exe_ret = system(cmd);
        } else {
            exe_ret = spawnv(P_WAIT, words[0], words);
        }
    }

    /*
     * Load interupts for checking.
     */
    for (i = 0; i < NUM_SAVE; i++) {
        int_temp[i] = getvect(saveem[i]);
    }

    /*
     * Restore interupts.
     */
    for (i = 0; i < NUM_SAVE; i++) {
        setvect(saveem[i], int_save[i]);
    }

	do_idle(2);					/* clear & reset screen saver */
    if (clearit) {
        restore_screen();
	}

    /* test for execution failures */
    switch (rc) {
        case SWAP_NO_SHRINK:
            cPrintf("Error - Can not shrink DOS memory block\n");
            break;
        case SWAP_NO_SAVE:
            cPrintf("Error - Can not save Citadel to memory or disk\n");
            break;
        case SWAP_NO_EXEC:
            cPrintf("Error - Can not execute application '%s'\n", cmd);
        switch (exe_ret) {
            case BAD_FUNC:
                cPrintf("Bad function.\n");
                break;
            case FILE_NOT_FOUND:
                cPrintf("Program file not found.\n");
                break;
            case ACCESS_DENIED:
                cPrintf("Access to program file denied.\n");
                break;
            case NO_MEMORY:
                cPrintf("Insufficient memory to run program.\n");
                break;
            case BAD_ENVIRON:
                cPrintf("Bad environment.\n");
                break;
            case BAD_FORMAT:
                cPrintf("Bad format.\n");
                break;
        }
            break;
    }

    /* test saved interrupts */
    for (i = 0, i2 = 0; i < NUM_SAVE; i++) {
        if ((int_save[i] != int_temp[i]) && debug && (i != 0x22)) {
            if (!i2) {
                cPrintf(" '%s' changed interupt(s): ", cmd);
                doccr();
            }
            cPrintf("    0x%2X to %p from %p",
            saveem[i], int_temp[i], int_save[i]);
            doccr();

            i2++;
        }
    }

    portInit();
    baud((int) speed);
#ifdef OLD_SWAP
    if (superit)
        tableIn();
#endif
    trapfl = fopen(cfg.trapfile, "a+");
    sprintf(pathbuf, "%s\\%s", cfg.msgpath, "msg.dat");
    openFile(pathbuf, &msgfl);
    sprintf(pathbuf, "%s\\%s", cfg.homepath, "grp.dat");
    openFile(pathbuf, &grpfl);
    sprintf(pathbuf, "%s\\%s", cfg.homepath, "hall.dat");
    openFile(pathbuf, &hallfl);
    sprintf(pathbuf, "%s\\%s", cfg.homepath, "log.dat");
    openFile(pathbuf, &logfl);
    sprintf(pathbuf, "%s\\%s", cfg.homepath, "room.dat");
    openFile(pathbuf, &roomfl);

    if (disabled)
        drop_dtr();
    /* update25();	*/
	do_idle(0);

	readDoorFiles(door);
}

/************************************************************************
 *      writeDoorFiles() - write door interface files
 ************************************************************************/

void writeDoorFiles(void)
{
    FILE *fd;
    char fname[FILESIZE];
    char path[64];
	
    getcwd(path, 64);
	changedir(cfg.aplpath);

    unlink("userdati.apl");
    unlink("userdato.apl");
    unlink("dorinfo?.def");
    if (readMessage)
        unlink("message.apl");
    unlink("readme.apl");

    strcpy(userdat.apl_name, logBuf.lbname);
    strcpy(userdat.apl_surname, logBuf.surname);
    strcpy(userdat.apl_node, cfg.nodeTitle);
    strcpy(userdat.apl_room, roomBuf.rbname);
    strcpy(userdat.apl_tempPath, cfg.temppath);
    strcpy(userdat.apl_applPath, cfg.aplpath);
    userdat.apl_com = (uchar) cfg.mdata;
    if (onConsole)
        userdat.apl_com = A_LOCAL;
    userdat.apl_sysop = gl_user.sysop;
    userdat.apl_aide = gl_user.aide;
    userdat.apl_expert = (uchar) logBuf.lbflags.EXPERT;
    userdat.apl_col = logBuf.lbwidth;
    userdat.apl_nulls = logBuf.lbnulls;

    userdat.apl_termLF = (uchar) logBuf.lbflags.LFMASK;
    userdat.apl_noChat = (uchar) cfg.noChat;
    userdat.apl_noBells = (uchar) cfg.noBells;
    userdat.apl_attr = cfg.attr;
    userdat.apl_wattr = cfg.wattr;
    userdat.apl_cattr = cfg.cattr;

    userdat.apl_credits = logBuf.credits;
    userdat.apl_baud = speed;
    userdat.apl_reserved_2 =
    (uchar) (logBuf.groups[1] == grpBuf.group[1].groupgen);
    userdat.apl_twit = gl_user.twit;

    if (!logBuf.lbflags.NOACCOUNT && cfg.accounting)
        userdat.apl_accounting = 1;
    else
        userdat.apl_accounting = 0;

    userdat.apl_ansion = gl_term.ansiOn;
    /* userdat.apl_ibmon = gl_term.IBMOn;      - FJM: Needed enhancement */

    if ((fd = fopen("userdato.apl", "wb")) == NULL) {
        mPrintf("Can't make userdato.apl");
    } else {
		fwrite(&userdat, sizeof(userdat), 1, fd);
		fclose(fd);
		sprintf(fname, "DORINFO%d.DEF", userdat.apl_com);
		write_rbbs(fname, &userdat, cfg.sysop, "Anytown", "USA");
		writeAplFile();
	}
	changedir(path);
    return;
}

/*
 * FUNCTION NAME: write_rbbs_name
 *
 * DESCRIPTION: <c> Write user name to RBBS application (door) information file
 *                                      (2 lines)
 *
 * LOCAL VARIABLES USED:
 *
 * GLOBAL VARIABLES USED:
 *
 * FUNCTIONS CALLED:
 *
 * MACROS USED:
 *
 * RETURNED VALUE: 0 on success
 *
 */

static int write_rbbs_name(FILE * fp, char *name)
{
    char first_name[80];
    char last_name[80];
    char *p;
    int len;

    if (name && *name) {
        p = strchr(name, ' ');
        if (p) {
        /* I don't really care that this isn't portable to a VAX :)  */
            len = p - name; /* not allowed in VROOM? */
            strncpy(first_name, name, len);
            first_name[len] = '\0';
        /* skip blanks */
            while (*p == ' ') {
                ++p;
            }
            strcpy(last_name, p);
        /* convert blanks to '_' */
            p = last_name;
            while (*p) {
                if (*p == ' ') {
                    *p = '_';
                }
                ++p;
            }
            fprintf(fp, "%s\r\n%s\r\n", first_name, last_name);
        } else {
            fprintf(fp, "%s\r\n;\r\n", name);
        }
    } else {
        fprintf(fp, ";\r\n;\r\n");
    }
    return 0;
}

/*
 * FUNCTION NAME: write_rbbs
 *
 * DESCRIPTION: <c> Write RBBS application (door) information file
 *
 * LOCAL VARIABLES USED:
 *
 * GLOBAL VARIABLES USED:
 *
 * FUNCTIONS CALLED:
 *
 * MACROS USED:
 *
 * RETURNED VALUE: 0 on success
 *
 */

static int write_rbbs(char *fname, struct apldata * data, char *sysop1,
char *city, char *state)
{
    FILE *fp;
    int status = 0;
    const char *sfmt = "%s\r\n";
    const char *dfmt = "%d\r\n";

    fp = fopen(fname, "wb");
    if (!fp) {
        return 1;
    }
    /* system */
    fprintf(fp, sfmt, data->apl_node);
    /* sysop 1 */
    write_rbbs_name(fp, sysop1);
    /* port */
    fprintf(fp, "COM%d\r\n", data->apl_com);
    /* com parameter, includes baud rate */
	if ((data->apl_baud > 0) && (data->apl_baud < 7)) {
		fprintf(fp, "%u", bauds[data->apl_baud]);
	} else {
		fprintf(fp, "0");
	}
	fprintf(fp, sfmt, " BAUD,N,8,1");
    /* unknown - 0 */
    fprintf(fp, dfmt, 0);
    /* user first name & last name */
    write_rbbs_name(fp, data->apl_name);
    /* location */
    fprintf(fp, "%s, %s\r\n", city, state);
    /* ansi */
    fprintf(fp, dfmt, data->apl_ansion);
    /* access */
    if (data->apl_sysop) {
        fprintf(fp, dfmt, 30);
    } else if (data->apl_aide) {
        fprintf(fp, dfmt, 20);
    } else {
        fprintf(fp, dfmt, 10);
    }

    /* max time on system */
    if (data->apl_accounting) {
        fprintf(fp, dfmt, (int) data->apl_credits);
    } else {
        fprintf(fp, dfmt, 999);
    }
    return status ? status : fclose(fp);
}


/* -------------------------------------------------------------------- */
/*      writeAplFile()  writes output.apl to disk                       */
/* -------------------------------------------------------------------- */
static void writeAplFile(void)
{
    FILE *fd;
    char buff[80];
    int i;

    if ((fd = fopen("output.apl", "wb")) == NULL) {
        mPrintf("Can't make output.apl");
        return;
    }
    for (i = 0; AplTab[i].item != APL_END; i++) {
        switch (AplTab[i].type) {
            case TYP_STR:
                sprintf(buff, "%c%s\n", AplTab[i].item, AplTab[i].variable);
                break;

            case TYP_BOOL:
            case TYP_CHAR:
                sprintf(buff, "%c%d\n", AplTab[i].item,
                *((char *) AplTab[i].variable));
                break;

            case TYP_INT:
                sprintf(buff, "%c%d\n", AplTab[i].item,
                *((int *) AplTab[i].variable));
                break;

            case TYP_FLOAT:
                sprintf(buff, "%c%f\n", AplTab[i].item,
                *((float *) AplTab[i].variable));
                break;

            case TYP_LONG:
                sprintf(buff, "%c%ld\n", AplTab[i].item,
                *((long *) AplTab[i].variable));
                break;

            case TYP_OTHER:
            switch (AplTab[i].item) {
                case APL_MDATA:
                    if (onConsole) {
                        sprintf(buff, "%c0 (LOCAL)\n", AplTab[i].item);
                    } else {
                        sprintf(buff, "%c%d\n", AplTab[i].item, cfg.mdata);
                    }
                    break;

                case APL_HALL:
                    sprintf(buff, "%c%s\n", AplTab[i].item,
                    hallBuf->hall[thisHall].hallname);
                    break;

                case APL_ROOM:
                    sprintf(buff, "%c%s\n", AplTab[i].item, roomBuf.rbname);
                    break;

                case APL_ACCOUNTING:
                    if (!logBuf.lbflags.NOACCOUNT && cfg.accounting) {
                        sprintf(buff, "%c1\n", AplTab[i].item);
                    } else {
                        sprintf(buff, "%c0\n", AplTab[i].item);
                    }
                    break;

                case APL_PERMANENT:
                    sprintf(buff, "%c%d\n", AplTab[i].item, lBuf.lbflags.PERMANENT);
                    break;

                case APL_VERIFIED:
                    sprintf(buff, "%c%d\n", AplTab[i].item,
                    lBuf.VERIFIED ? 0 : 1);
                    break;

                case APL_NETUSER:
                    sprintf(buff, "%c%d\n", AplTab[i].item, lBuf.lbflags.NETUSER);
                    break;

                case APL_NOMAIL:
                    sprintf(buff, "%c%d\n", AplTab[i].item, lBuf.lbflags.NOMAIL);
                    break;

                case APL_CHAT:
                    sprintf(buff, "%c%d\n", AplTab[i].item, !cfg.noChat);
                    break;

                case APL_BELLS:
                    sprintf(buff, "%c%d\n", AplTab[i].item, !cfg.noBells);
                    break;

                default:
                    buff[0] = 0;
                    break;
            }
                break;

            default:
                buff[0] = 0;
                break;
        }

        if (strlen(buff) > 1) {
            fputs(buff, fd);
        }
    }

    fprintf(fd, "%c\n", APL_END);

    fclose(fd);
    /* parse node country later */
}

/* -------------------------------------------------------------------- */
/*      readAplFile()  reads input.apl from disk                        */
/* -------------------------------------------------------------------- */
static void readAplFile(void)
{
    FILE *fd;
    int i;
    char buff[200];
    int item;
    int roomno;
    int found;
    int slot;

    if (readMessage) {
        clearmsgbuf();
        strcpy(msgBuf->mbauth, cfg.nodeTitle);
        msgBuf->mbroomno = thisRoom;
    }
    if ((fd = fopen("input.apl", "rt")) != NULL) {
        do {
            item = fgetc(fd);
            if (feof(fd)) {
                break;
            }
            fgets(buff, 198, fd);
            buff[strlen(buff) - 1] = 0;

            found = FALSE;

            for (i = 0; AplTab[i].item != APL_END; i++) {
                if (AplTab[i].item == item && AplTab[i].keep) {
                    found = TRUE;

                    switch (AplTab[i].type) {
                        case TYP_STR:
                            strncpy((char *) AplTab[i].variable, buff, AplTab[i].length);
                            ((char *) AplTab[i].variable)[AplTab[i].length - 1] = 0;
                            break;

                        case TYP_BOOL:
                        case TYP_CHAR:
                            *((char *) AplTab[i].variable) = (char) atoi(buff);
                            break;

                        case TYP_INT:
                            *((int *) AplTab[i].variable) = atoi(buff);
                            break;

                        case TYP_FLOAT:
                            *((float *) AplTab[i].variable) = atof(buff);
                            break;

                        case TYP_LONG:
                            *((long *) AplTab[i].variable) = atol(buff);
                            break;

                        case TYP_OTHER:
                        switch (AplTab[i].item) {
                            case APL_HALL:
                                if (stricmp(buff, hallBuf->hall[thisHall].hallname)
                                != SAMESTRING) {
                                    slot = hallexists(buff);
                                    if (slot != ERROR) {
                                        mPrintf("Hall change to: %s", buff);
                                        doCR();
                                        thisHall = (unsigned char) slot;
                                    } else {
                                        cPrintf("No such hall %s!\n", buff);
                                    }
                                }
                                break;

                            case APL_ROOM:
                                if ((roomno = roomExists(buff)) != ERROR) {
                                    if (roomno != thisRoom) {
                                        mPrintf("Room change to: %s", buff);
                                        doCR();
                                        logBuf.lbroom[thisRoom].lbgen
                                        = roomBuf.rbgen;
                                        ug_lvisit = logBuf.lbroom[thisRoom].lvisit;
                                        ug_new = talleyBuf.room[thisRoom].new;
                                        logBuf.lbroom[thisRoom].lvisit = 0;
                                        logBuf.lbroom[thisRoom].mail = 0;
                                /* zero new count in talleybuffer */
                                        talleyBuf.room[thisRoom].new = 0;

                                        getRoom(roomno);

                                        if ((logBuf.lbroom[thisRoom].lbgen)
                                        != roomBuf.rbgen) {
                                            logBuf.lbroom[thisRoom].lbgen
                                            = roomBuf.rbgen;
                                            logBuf.lbroom[thisRoom].lvisit
                                            = (MAXVISIT - 1);
                                        }
                                    }
                                } else {
                                    cPrintf("No such room: %s!\n", buff);
                                }
                                break;

                            case APL_PERMANENT:
                                lBuf.lbflags.PERMANENT = atoi(buff);
                                break;

                            case APL_VERIFIED:
                                lBuf.VERIFIED = !atoi(buff);
                                break;

                            case APL_NETUSER:
                                lBuf.lbflags.NETUSER = atoi(buff);
                                break;

                            case APL_NOMAIL:
                                lBuf.lbflags.NOMAIL = atoi(buff);
                                break;

                            case APL_CHAT:
                                cfg.noChat = !atoi(buff);
                                break;

                            case APL_BELLS:
                                cfg.noBells = !atoi(buff);
                                break;

                            default:
                                mPrintf("Bad value %d \"%s\"", item, buff);
                                doCR();
                                break;
                        }
                            break;

                        default:
                            break;
                    }
                }
            }

            if (!found && readMessage) {
                found = TRUE;

                switch (item) {
                    case MSG_NAME:
                        strcpy(msgBuf->mbauth, buff);
                        break;

                    case MSG_TO:
                        strcpy(msgBuf->mbto, buff);
                        break;

                    case MSG_GROUP:
                        strcpy(msgBuf->mbgroup, buff);
                        break;

                    case MSG_ROOM:
                        if ((roomno = roomExists(buff)) == ERROR) {
                            cPrintf(" AP: No room \"%s\"!\n", buff);
                        } else {
                            msgBuf->mbroomno = roomno;
                        }
                        break;

                    default:
                        doCR();
                        found = FALSE;
                        break;
                }
            }
            if (!found && AplTab[i].item != APL_END) {
                mPrintf("Bad value %d \"%s\"", item, buff);
                doCR();
            }
        }
        while (item != APL_END && !feof(fd));

#ifdef BAD
        unlink("input.apl");
#endif

        fclose(fd);
    }
    /* update25();	*/
	do_idle(0);

    if (readMessage) {
        if ((fd = fopen("message.apl", "rb")) != NULL) {
            GetStr(fd, msgBuf->mbtext, cfg.maxtext);
            fclose(fd);
            putMessage();
            noteMessage();
        }
        unlink("message.apl");
    }
    if (filexists("readme.apl")) {
        dumpf("readme.apl");
        unlink("readme.apl");
        doCR();
    }
}


/* EOF */
