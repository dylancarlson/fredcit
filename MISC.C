/* -------------------------------------------------------------------- */
/*  MISC.C                    Citadel                                   */
/* -------------------------------------------------------------------- */
/*  Citadel garbage dump, if it aint elsewhere, its here.               */
/* -------------------------------------------------------------------- */

/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/MISC.C_V  $
 * 
 *    Rev 1.44   14 Jun 1991  7:43:06   FJM
 *
 *    Rev 1.43   06 Jun 1991  9:19:24   FJM
 *
 *    Rev 1.42   27 May 1991 11:43:04   FJM
 *    Rev 1.41   22 May 1991  2:17:28   FJM
 *    Rev 1.37   04 Apr 1991 14:13:36   FJM
 *    Rev 1.18   06 Jan 1991 12:45:54   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.17   27 Dec 1990 20:16:00   FJM
 * Changes for porting.
 *
 *    Rev 1.14   22 Dec 1990 13:37:42   FJM
 *    Rev 1.7   02 Dec 1990  0:55:38   FJM
 *    Rev 1.6   01 Dec 1990  2:55:38   FJM
 *    Rev 1.5   24 Nov 1990  3:44:28   FJM
 *    Rev 1.4   23 Nov 1990 13:25:08   FJM
 * Header change
 *
 *    Rev 1.2   17 Nov 1990 16:11:56   FJM
 * Added version control log header
 *
 * --------------------------------------------------------------------
 *
 *  Early History:
 *
 *  02/08/89    (PAT)   History Re-Started
 *                      InitAideMess and SaveAideMess added
 *  05/26/89    (PAT)   Many of the functions move to other modules
 *  03/07/90    {zm}    Add 'softname' as the name of the software.
 *  03/17/90    FJM     added checks for farfree()
 *  04/01/90    FJM     added checks reconfigure on new version
 *  05/17/90    FJM     Added external commands
 *  06/06/90    FJM     Changed strftime to cit_strftime
 *  06/16/90    FJM     Changed reconfigure message.
 *
 * -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  filexists()     Does the file exist?                                */
/*  hash()          Make an int out of their name                       */
/*  ctrl_c()        Used to catch CTRL-Cs                               */
/*  openFile()      Special to open a .cit file                         */
/*  trap()          Record a line to the trap file                      */
/*  hmemcpy()       Terible hack to copy over 64K, beacuse MSC cant.    */
/*  h2memcpy()      Terible hack to copy over 64K, beacuse MSC cant. PT2*/
/*  SaveAideMess()  Save aide message from AIDEMSG.TMP                  */
/*  amPrintf()      aide message printf                                 */
/*  amZap()         Zap aide message being made                         */
/*  changedir()     changes curent drive and directory                  */
/*  changedisk()    change to another drive                             */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Includes                                                            */
/* -------------------------------------------------------------------- */

#define MISC

#include <signal.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>

/* Cit */
#include "ctdl.h"

#ifndef ATARI_ST
#include <bios.h>
#include <conio.h>
#include <dos.h>
#include <io.h>
#include <alloc.h>
#endif

#include "proto.h"
#include "global.h"

/* --------------------------------------------------------------------
 *  Macro functions
 * -------------------------------------------------------------------- */

#define changedisk(disk)	(setdisk((disk) - 'A'))

/* --------------------------------------------------------------------
 *  External data
 * -------------------------------------------------------------------- */

/* --------------------------------------------------------------------
 *  Static Data definitions
 * -------------------------------------------------------------------- */

/* --------------------------------------------------------------------
 *  filexists()     Does the file exist?
 * -------------------------------------------------------------------- */
BOOL filexists(char *filename)
{
    return (BOOL) ((access(filename, 4) == 0) ? TRUE : FALSE);
}

/* -------------------------------------------------------------------- */
/*  hash()          Make an int out of their name                       */
/* -------------------------------------------------------------------- */
uint hash(char *str)
{
    int h, shift;

    for (h = shift = 0; *str; shift = (shift + 1) & 7, str++) {
        h ^= (toupper(*str)) << shift;
    }
    return h;
}

/* -------------------------------------------------------------------- */
/*  ctrl_c()        Used to catch CTRL-Cs                               */
/* -------------------------------------------------------------------- */
void ctrl_c(void)
{
    uchar row, col;

    signal(SIGINT, ctrl_c);
    readpos(&row, &col);
    position(row - 1, 19);
    ungetch('\r');
    getkey = TRUE;
}

/* -------------------------------------------------------------------- */
/*  openFile()      Special to open a .cit file                         */
/* -------------------------------------------------------------------- */
void openFile(char *filename, FILE ** fd)
{
    /* open message file */
    if  ((*fd = fopen(filename, "r+b")) == NULL) {
        crashout(".DAT file missing!");
    }
}

/* -------------------------------------------------------------------- */
/*  trap()          Record a line to the trap file                      */
/* -------------------------------------------------------------------- */
void trap(char *string, int what)
{
    char dtstr[20];

    /* check to see if we are supposed to log this event */
    if (!cfg.trapit[what])
        return;

    cit_strftime(dtstr, 19, "%y%b%D %X", 0l);

    fprintf(trapfl, "%s:  %s\n", dtstr, string);

    fflush(trapfl);
}

/* -------------------------------------------------------------------- */
/*  hmemcpy()       Terible hack to copy over 64K, beacuse MSC cant.    */
/* -------------------------------------------------------------------- */
#define K32  (32840L)
void hmemcpy(void HUGE * xto, void HUGE * xfrom, long size)
{
    char HUGE *from;
    char HUGE *to;

    to = xto;
    from = xfrom;

    if (to > from) {
        h2memcpy(to, from, size);
        return;
    }
    while (size > K32) {
        memcpy((char FAR *) to, (char FAR *) from, (unsigned int) K32);
        size -= K32;
        to += K32;
        from += K32;
    }

    if (size)
        memcpy((char FAR *) to, (char FAR *) from, (uint) size);
}

/* -------------------------------------------------------------------- */
/*  h2memcpy()     Terible hack to copy over 64K, beacuse MSC cant. PT2 */
/* -------------------------------------------------------------------- */
void h2memcpy(char HUGE * to, char HUGE * from, long size)
{
    to += size;
    from += size;

    size++;

    while (size--)
        *to-- = *from--;
}

/* -------------------------------------------------------------------- */
/*  SaveAideMess()  Save aide message from AIDEMSG.TMP                  */
/* -------------------------------------------------------------------- */
void SaveAideMess(void)
{
    char temp[90];
    FILE *fd;

    sprintf(temp, "%s\\%s", cfg.temppath, "aidemsg.tmp");

    if (aideFl == NULL) {
        return;
    }
    fclose(aideFl);

    if ((fd = fopen(temp, "rb")) == NULL) {
        crashout("AIDEMSG.TMP file not found during aide message save!");
    }
    clearmsgbuf();

    GetStr(fd, msgBuf->mbtext, cfg.maxtext);

    fclose(fd);
    unlink(temp);

    aideFl = NULL;

    if (strlen(msgBuf->mbtext) < 10)
        return;

    strcpy(msgBuf->mbauth, cfg.nodeTitle);

    msgBuf->mbroomno = AIDEROOM;

    putMessage();
    noteMessage();
}

/* -------------------------------------------------------------------- */
/*  amPrintf()      aide message printf                                 */
/* -------------------------------------------------------------------- */
void amPrintf(char *fmt,...)
{
    va_list ap;
    char temp[90];

    /* no message in progress? */
    if (aideFl == NULL) {
        sprintf(temp, "%s\\%s", cfg.temppath, "aidemsg.tmp");

        unlink(temp);

        if ((aideFl = fopen(temp, "w")) == NULL) {
            crashout("Can not open AIDEMSG.TMP!");
        }
    }
    va_start(ap, fmt);
    vfprintf(aideFl, fmt, ap);
    va_end(ap);

}

/* -------------------------------------------------------------------- */
/*  amZap()         Zap aide message being made                         */
/* -------------------------------------------------------------------- */
void amZap(void)
{
    char temp[90];

    if (aideFl != NULL) {
        fclose(aideFl);
    }
    sprintf(temp, "%s\\%s", cfg.temppath, "aidemsg.tmp");

    unlink(temp);

    aideFl = NULL;
}

/* -------------------------------------------------------------------- */
/*  changedir()     changes curent drive and directory                  */
/* -------------------------------------------------------------------- */
int changedir(char *path)
{
    /* uppercase   */
    path[0] = (char) toupper(path[0]);

    /* change disk */
    changedisk(path[0]);

    /* change path */
    if (chdir(path) == -1) {
        mPrintf(" \n Directory path invalid.");
        amPrintf(" \n Invalid directory '%s' on drive '%c:'",
			path, path[0]);
		if (loggedIn)
			amPrintf(" in room '%s'",roomBuf.rbname);
        SaveAideMess();
        return -1;
    }
    return TRUE;
}

/* -------------------------------------------------------------------- */
/*  changedisk()    change to another drive                             */
/* -------------------------------------------------------------------- */
/***************************
*void changedisk(char disk)*
*{                         *
*    setdisk(disk - 'A');  *
*}                         *
***************************/

/* -------------------------------------------------------------------- */
/*  GetStr()        gets a null-terminated string from a file           */
/* -------------------------------------------------------------------- */
void GetStr(FILE * fl, char *str, int mlen)
{
    int l;
    char ch;

    l = 0;
    ch = 1;
    while (!feof(fl) && ch && l < mlen) {
        ch = (uchar) fgetc(fl);
        if (ch != '\r' && ch != '\xFF') {
            str[l] = ch;
            l++;
        }
    }
    str[l] = '\0';
}

/* --------------------------------------------------------------------
 *  PutStr()        puts a null-terminated string to a file
 * -------------------------------------------------------------------- */
void PutStr(FILE * fl, char *str)
{
    fwrite(str, sizeof(char), (strlen(str) + 1), fl);
}
/* EOF */
