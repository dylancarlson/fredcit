/************************************************************************/
/*                              grphall.c                               */
/*        hall and group code for Citadel bulletin board system         */
/************************************************************************/

#include <string.h>
#include "ctdl.h"
#include "proto.h"
#include "global.h"

/************************************************************************/
/*                              Contents                                */
/*                                                                      */
/*      accesshall()            returns TRUE if person can access hall  */
/*      cleargroupgen()         removes logBuf from all groups          */
/*      defaulthall()           handles .ed  (enter Default-hallway)    */
/*      enterhall()             handles .eh                             */
/*      gotodefaulthall()       goes to user's default hallway          */
/*      groupexists()           returns # of named group, else ERROR    */
/*      groupseesroom()         indicates if group can see room #       */
/*      groupseeshall()         indicates if group can see hall #       */
/*      hallexists()            returns # of named hall,  else ERROR    */
/*      ingroup()               returns TRUE if log is in named group   */
/*      iswindow()              for .kw .kvw is # room a window         */
/*      knownhalls()            handles .kh, .kvh                       */
/*      partialgroup()          returns slot of partially named group   */
/*      partialhall()           returns slot of partially named hall    */
/*      readhalls()             handles .rh, .rvh                       */
/*      roominhall()            indicates if room# is in hall           */
/*      setgroupgen()           sets unmatching group generation #'s    */
/*      stephall()              handles previous, next hall             */
/*                                                                      */
/************************************************************************/


/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/GRPHALL.C_V  $
 * 
 *    Rev 1.40   01 Nov 1991 11:20:06   FJM
 * Added gl_ structures
 *
 *    Rev 1.39   15 Jun 1991  8:41:20   FJM
 *
 *    Rev 1.22   13 Jan 1991  0:31:14   FJM
 * Name overflow fixes.
 *
 *    Rev 1.20   11 Jan 1991 12:43:04   FJM
 *    Rev 1.16   06 Jan 1991 12:44:34   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.15   27 Dec 1990 20:16:16   FJM
 * Changes for porting.
 *
 *    Rev 1.12   22 Dec 1990 13:38:38   FJM
 *    Rev 1.6   09 Dec 1990 15:22:48   FJM
 * Improved messages from next hall code when no next hall.
 * Consolidated duplicate code.
 *
 *    Rev 1.4   23 Nov 1990 13:24:54   FJM
 * Header change
 *
 *    Rev 1.2   17 Nov 1990 16:12:04   FJM
 * Added version control log header
 * -------------------------------------------------------------------- */

/************************************************************************/
/*    accesshall() returns true if hall can be accessed                 */
/*    from current room                                                 */
/************************************************************************/
int accesshall(int slot)
{
    int accessible = 0;

    if ((slot == thisHall) || (hallBuf->hall[slot].h_inuse
        && hallBuf->hall[slot].hroomflags[thisRoom].window
    && groupseeshall(slot))) {
        accessible = TRUE;
    }
    return accessible;
}


/************************************************************************/
/*    cleargroupgen()  removes logBuf from all groups                   */
/************************************************************************/
void cleargroupgen(void)
{
    int groupslot;

    for (groupslot = 0; groupslot < MAXGROUPS; groupslot++) {
        logBuf.groups[groupslot] =
        (grpBuf.group[groupslot].groupgen
        + (MAXGROUPGEN - 1)) % MAXGROUPGEN;
    }
}


/************************************************************************/
/*      enterhall()  handles .eh                                        */
/************************************************************************/
void enterhall(void)
{
    label hallname;
    int slot, accessible;

    getString("hall", hallname, NAMESIZE, FALSE, ECHO, "");

    slot = hallexists(hallname);
    if (slot == ERROR)
        slot = partialhall(hallname);

    if (slot != ERROR)
        accessible = accesshall(slot);

    if ((slot == ERROR) || !strlen(hallname) || !accessible) {
        mPrintf(" No such hall.");
        return;
    } else {
        thisHall = (unsigned char) slot;
    }
}


/************************************************************************/
/*      gotodefaulthall()  goes to user's default hallway               */
/************************************************************************/
void gotodefaulthall(void)
{
    int i;

    if (logBuf.hallhash) {
        for (i = 1; i < MAXHALLS; ++i) {
            if (hash(hallBuf->hall[i].hallname) == logBuf.hallhash
            && hallBuf->hall[i].h_inuse) {
                if (groupseeshall(i))
                    thisHall = (unsigned char) i;
            }
        }
    }
}

/************************************************************************/
/*      groupseeshall()  returns true if group can see hallway          */
/************************************************************************/
int groupseeshall(int hallslot)
{
    if  ((!hallBuf->hall[hallslot].owned) ||

    /* generation in logBuf for this hall's owning group */
        (logBuf.groups[hallBuf->hall[hallslot].grpno] ==

    /* generation in groupbuffer for this hall's owning group */
        grpBuf.group[hallBuf->hall[hallslot].grpno].groupgen))
        return (TRUE);

    return (FALSE);
}

/************************************************************************/
/*      groupseesroom()  returns true if group can see room             */
/************************************************************************/
int groupseesroom(int groupslot)
{
    if  (
        (!roomTab[groupslot].rtflags.GROUPONLY) ||

    /* generation in logBuf for this room's owning group == */
    /* generation in groupbuffer for this room's owning group */
        (logBuf.groups[roomTab[groupslot].grpno] ==
        grpBuf.group[roomTab[groupslot].grpno].groupgen)
    ) {
        return (TRUE);
    } else {
        return (FALSE);
    }
}


/************************************************************************/
/*      groupexists()  return # of named group, else ERROR              */
/************************************************************************/
int groupexists(char *groupname)
{
    int i;

    for (i = 0; i < MAXGROUPS; i++) {
        if (grpBuf.group[i].g_inuse &&
            strcmpi(groupname, grpBuf.group[i].groupname) == SAMESTRING)
            return (i);
    }
    return (ERROR);
}


/************************************************************************/
/*      hallexists()  return # of named hall, else ERROR                */
/************************************************************************/
int hallexists(char *hallname)
{
    int i;

    for (i = 0; i < MAXHALLS; i++) {
        if (hallBuf->hall[i].h_inuse &&
            strcmpi(hallname, hallBuf->hall[i].hallname) == SAMESTRING)
            return (i);
    }
    return (ERROR);
}

/************************************************************************/
/*      ingroup()  returns TRUE if person is in named group             */
/************************************************************************/
int ingroup(int groupslot)
{
    if  ((logBuf.groups[groupslot] == grpBuf.group[groupslot].groupgen)
        &&  grpBuf.group[groupslot].g_inuse)
        return (TRUE);
    return (FALSE);
}

/************************************************************************/
/*      iswindow()  is room a window into accessible halls?             */
/************************************************************************/
int iswindow(int roomslot)
{
    int i, window = 0;

    if (!roomTab[roomslot].rtflags.INUSE)
        return (FALSE);

    for (i = 0; i < MAXHALLS && !window; i++) {
        if (hallBuf->hall[i].h_inuse &&
            hallBuf->hall[i].hroomflags[roomslot].window)
            window = TRUE;
    }
    return (window);
}


/************************************************************************/
/*      knownhalls()  handles .kh .kvh                                  */
/************************************************************************/
void knownhalls(void)
{
    int i;

    doCR();

    mPrintf(" Hallways accessible.");

    doCR();

    for (i = 0; i < MAXHALLS; i++) {
        if (accesshall(i))
            mPrintf(" %s:", hallBuf->hall[i].hallname);
    }
}


/************************************************************************/
/*      listgroup()  aide fn: to list groups                            */
/************************************************************************/
void getgroup(void)
{
    label groupname;
    int groupslot;

    mf.mfGroup[0] = EOS;

    getString("group", groupname, NAMESIZE, FALSE, ECHO, "");

    if (!(*groupname))
        return;

    groupslot = groupexists(groupname);
    if (groupslot == ERROR)
        groupslot = partialgroup(groupname);

    if (groupslot == ERROR) {
        mPrintf("\n No such group!");
        doCR();
        mf.mfLim = FALSE;
        return;
    }
    if (!(ingroup(groupslot) || gl_user.sysop || gl_user.aide)) {
        mf.mfLim = FALSE;
        return;
    }
    if (grpBuf.group[groupslot].lockout && (!logBuf.lbflags.SYSOP)) {
        mf.mfLim = FALSE;
        return;
    }
    mPrintf("\n Reading group %s only.\n ", grpBuf.group[groupslot].groupname);

    strcpy(mf.mfGroup, grpBuf.group[groupslot].groupname);
}

/************************************************************************/
/*      partialgroup()  returns slot # of partial group name, else error*/
/*      used for .EL Message, .EL Room and .AG .AL                      */
/************************************************************************/
int partialgroup(char *groupname)
{
    label compare;
    int i, length;

    length = strlen(groupname);

    for (i = 0; i < MAXGROUPS; i++) {
        if (grpBuf.group[i].g_inuse) {
            strcpy(compare, grpBuf.group[i].groupname);

            compare[length] = '\0';

            if ((strcmpi(compare, groupname) == SAMESTRING)
                && (ingroup(i) || gl_user.aide))
                return (i);
        }
    }
    return (ERROR);
}

/************************************************************************/
/*      partialhall()  returns slot # of partial hall name, else error  */
/*      used for .Enter Hallway and .Enter Default-hallway  only!       */
/************************************************************************/
int partialhall(char *hallname)
{
    label compare;
    int i, length;

    length = strlen(hallname);

    for (i = 0; i < MAXHALLS; i++) {
        if (hallBuf->hall[i].h_inuse) {
            strcpy(compare, hallBuf->hall[i].hallname);

            compare[length] = '\0';

            if ((strcmpi(compare, hallname) == SAMESTRING)
                && groupseeshall(i))
                return (i);
        }
    }
    return (ERROR);
}

/************************************************************************/
/*      readhalls()  handles .rh .rvh                                   */
/************************************************************************/
void readhalls(void)
{
    int i;

    doCR();
    doCR();

    mPrintf("Room %s is contained in ", roomBuf.rbname);

    for (i = 0; i < MAXHALLS; i++) {
        if (hallBuf->hall[i].h_inuse
            && hallBuf->hall[i].hroomflags[thisRoom].inhall
            && groupseeshall(i))
            mPrintf("%s: ", hallBuf->hall[i].hallname);
    }
}

/************************************************************************/
/*    roominhall()  returns TRUE if room# is in current hall            */
/************************************************************************/
int roominhall(int roomslot)
{
    if  (hallBuf->hall[thisHall].hroomflags[roomslot].inhall)
        return (TRUE);

    return (FALSE);
}


/************************************************************************/
/*    setgroupgen()  sets unmatching group generation #'s               */
/************************************************************************/
void setgroupgen(void)
{
    int groupslot;

    for (groupslot = 0; groupslot < MAXGROUPS; groupslot++) {
        if (logBuf.groups[groupslot] != grpBuf.group[groupslot].groupgen) {
            logBuf.groups[groupslot] =
            (grpBuf.group[groupslot].groupgen
            + (MAXGROUPGEN - 1)) % MAXGROUPGEN;
        }
    }
}

/************************************************************************/
/*    stephall()  handles previous, next hall                           */
/************************************************************************/
void stephall(int direction)
{
    int i;
    char done = FALSE;

    i = thisHall;

    if (!iswindow(thisRoom)) {
        if (gl_user.expert)
            mPrintf("? ");
        else
            mPrintf("%s is not a window room. Try .kw ",
            roomTab[thisRoom].rtname);
        return;
    }
    do {
    /* step */
        if (direction == 1) {
            ++i;
            if (i == MAXHALLS)
                i = 0;
        } else {
            --i;
            if (i == -1)
                i = MAXHALLS - 1;
        }

    /* keep from looping endlessly */
        if (i == thisHall) {
            return;
        } else if (accesshall(i)) {
            mPrintf("%s ", hallBuf->hall[i].hallname);
            thisHall = (unsigned char) i;
            done = TRUE;
        }
    } while (!done);

    if (hallBuf->hall[thisHall].described && gl_user.roomtell) {
        if (changedir(cfg.roompath) == -1)
            return;

        if (checkfilename(hallBuf->hall[thisHall].htell, 0) == ERROR) {
            changedir(cfg.homepath);
            return;
        }
        doCR();

        if (!filexists(hallBuf->hall[thisHall].htell)) {
            doCR();
            mPrintf("No hallway description %s",
				hallBuf->hall[thisHall].htell);
            changedir(cfg.homepath);
            doCR();
            return;
        }
        if (!gl_user.expert) {
            doCR();
            mPrintf("<J>ump <P>ause <S>top");
            doCR();
        }
    /* print it out */
        dumpf(hallBuf->hall[thisHall].htell);

    /* go to our home-path */
        changedir(cfg.homepath);

        outFlag = OUTOK;
    }
}
