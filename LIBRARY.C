/************************************************************************/
/*                            library.c                                 */
/*                                                                      */
/*                  Routines used by Ctdl & Confg                       */
/************************************************************************/

#include <string.h>

/* Citadel */
#include "ctdl.h"

#ifndef ATARI_ST
#include <alloc.h>
#endif

#include "proto.h"
#include "global.h"

/************************************************************************/
/*                              contents                                */
/*                                                                      */
/*      getGroup()              loads groupBuffer                       */
/*      putGroup()              stores groupBuffer to disk              */
/*                                                                      */
/*      getHall()               loads hallBuffer                        */
/*      putHall()               stores hallBuffer to disk               */
/*                                                                      */
/*      getLog()                loads requested CTDLLOG record          */
/*      putLog()                stores a logBuffer into citadel.log     */
/*                                                                      */
/*      getRoom()               load given room into RAM                */
/*      putRoom()               store room to given disk slot           */
/*                                                                      */
/************************************************************************/


/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/LIBRARY.C_V  $
 * 
 *    Rev 1.40   21 Sep 1991 10:19:02   FJM
 * FredCit release
 *
 *    Rev 1.17   06 Jan 1991 12:45:20   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.16   27 Dec 1990 20:16:10   FJM
 * Changes for porting.
 *
 *    Rev 1.13   22 Dec 1990 13:38:28   FJM
 *
 *    Rev 1.6   07 Dec 1990 23:10:24   FJM
 *
 *    Rev 1.5   02 Dec 1990  0:55:02   FJM
 * Added aide message for failed changedir()
 *
 *    Rev 1.4   23 Nov 1990 13:25:02   FJM
 * Header change
 *
 *    Rev 1.2   17 Nov 1990 16:12:20   FJM
 * Added version control log header
 * --------------------------------------------------------------------
 *  Early History:
 *
 *  04/01/90    FJM     added checks reconfigure on new version
 *  04/08/90    FJM     cleanup
 *  06/09/90    FJM     Added readCrontab & writeCrontab
 *  08/13/90    FJM     Moved more allocation to allocateTables()
 *
 * -------------------------------------------------------------------- */

/************************************************************************/
/*      getGrooup() loads group data into RAM buffer                    */
/************************************************************************/
void getGroup(void)
{
    fseek(grpfl, 0L, 0);

    if (fread(&grpBuf, sizeof grpBuf, 1, grpfl) != 1) {
        crashout("getGroup-read fail EOF detected!");
    }
}

/************************************************************************/
/*      putGroup() stores group data into grp.cit                       */
/************************************************************************/
void putGroup(void)
{
    fseek(grpfl, 0L, 0);

    if (fwrite(&grpBuf, sizeof grpBuf, 1, grpfl) != 1) {
        crashout("putGroup-write fail");
    }
    fflush(grpfl);
}

/************************************************************************/
/*      getHall() loads hall data into RAM buffer                       */
/************************************************************************/
void getHall(void)
{
    fseek(hallfl, 0L, 0);

    if (fread(hallBuf, sizeof(struct hallBuffer), 1, hallfl) != 1) {
        crashout("getHall-read fail EOF detected!");
    }
}

/************************************************************************/
/*      putHall() stores group data into hall.cit                       */
/************************************************************************/
void putHall(void)
{
    fseek(hallfl, 0L, 0);

    if (fwrite(hallBuf, sizeof(struct hallBuffer), 1, hallfl) != 1) {
        crashout("putHall-write fail");
    }
    fflush(hallfl);
}

/************************************************************************/
/*      getLog() loads requested log record into RAM buffer             */
/************************************************************************/
void getLog(struct logBuffer * lBuf, int n)
{
    long int s;

    if (lBuf == &logBuf)
        thisLog = n;

    s = (long) n *(long) sizeof logBuf;

    fseek(logfl, s, 0);

    if (fread(lBuf, sizeof logBuf, 1, logfl) != 1) {
        crashout("getLog-read fail EOF detected!");
    }
}

/************************************************************************/
/*      putLog() stores given log record into log.cit                   */
/************************************************************************/
void putLog(struct logBuffer * lBuf, int n)
{
    long int s;

    s = (long) n *(long) (sizeof(struct logBuffer));

    fseek(logfl, s, 0);

    if (fwrite(lBuf, sizeof logBuf, 1, logfl) != 1) {
        crashout("putLog-write fail");
    }
    fflush(logfl);
}

/************************************************************************/
/*      getRoom()                                                       */
/************************************************************************/
void getRoom(int rm)
{
    long int s;

    /* load room #rm into memory starting at buf */
    thisRoom = rm;
    s = (long) rm *(long) sizeof roomBuf;

    fseek(roomfl, s, 0);
    if (fread(&roomBuf, sizeof roomBuf, 1, roomfl) != 1) {
        crashout("getRoom(): read failed error or EOF!");
    }
}

/************************************************************************/
/*      putRoom() stores room in buf into slot rm in room.buf           */
/************************************************************************/
void putRoom(int rm)
{
    long int s;

    s = (long) rm *(long) sizeof roomBuf;

    fseek(roomfl, s, 0);
    if (fwrite(&roomBuf, sizeof roomBuf, 1, roomfl) != 1) {
        crashout("putRoom() crash! 0 returned.");
    }
    fflush(roomfl);
}

