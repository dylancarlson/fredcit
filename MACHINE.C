/* $Header:   D:/VCS/FCIT/MACHINE.C_V   1.30   01 Nov 1991 11:20:26   FJM  $ */
/************************************************************************
 *
 *                              CONTENTS
 *
 *      bytesfree()             returns #bytes free on current drive
 *      getattr()               returns a file attribute
 *      setattr()               sets file attributes
 *
 ************************************************************************/

/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/MACHINE.C_V  $
*
*   Rev 1.30   01 Nov 1991 11:20:26   FJM
*Added gl_ structures
*
*   Rev 1.29   21 Sep 1991 10:19:08   FJM
*FredCit release
 *
 *   Rev 1.28   29 May 1991 11:18:30   FJM
 * Removed redundent includes.
 *
 *   Rev 1.26   19 Apr 1991 23:27:06   FJM
 * New harderror handler.
 *
 *   Rev 1.5   06 Jan 1991 12:45:58   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.0   27 Dec 1990 20:17:10   FJM
 * Changes for porting.
 *
 *   Rev 1.1   22 Dec 1990 13:39:26   FJM
 *   Rev 1.0   22 Dec 1990  0:27:42   FJM
 * Initial revision.
 *
 * -------------------------------------------------------------------- */

#include <time.h>
#include <dos.h>
#include <io.h>
#include "ctdl.h"
#include "proto.h"

/************************************************************************/
/*      getattr() returns file attribute                                */
/************************************************************************/
unsigned char getattr(char *filename)
{
    union REGS inregs;
    union REGS outregs;

    inregs.x.dx = FP_OFF(filename);
    inregs.h.ah = 0x43;     /* CHMOD */
    inregs.h.al = 0;        /* GETATTR */

    intdos(&inregs, &outregs);

    return ((int) outregs.x.cx);
}


/************************************************************************/
/*      setattr() sets file attributes                                  */
/************************************************************************/
void setattr(char FAR * filename, unsigned char attr)
{
    union REGS inregs;
    union REGS outregs;

    inregs.x.dx = FP_OFF(filename);
    inregs.h.ah = 0x43;     /* CHMOD */
    inregs.h.al = 1;        /* SET ATTR */

    inregs.x.cx = attr;     /* attribute */

    intdos(&inregs, &outregs);
}

/************************************************************************
 *      bytesfree() returns # bytes free on drive
 ************************************************************************/
long bytesfree(void)
{
    char path[64];
    long bytes;
    union REGS REG;

    getcwd(path, 64);

    REG.h.ah = 0x36;        /* select drive */

    REG.h.dl = (path[0] - '@');

    intdos(&REG, &REG);

    bytes = (long) ((long) REG.x.cx * (long) REG.x.ax * (long) REG.x.bx);

	return (bytes);
}
#define IGNORE  0
#define RETRY   1
#define ABORT   2

/* define the error messages for trapping disk problems */
static char *err_msg[] = {
	"write protect",
	"unknown unit",
	"drive not ready",
	"unknown command",
	"data error (CRC)",
	"bad request",
	"seek error",
	"unknown media type",
	"sector not found",
	"printer out of paper",
	"write fault",
	"read fault",
	"general failure",
	"reserved",
	"reserved",
	"invalid disk change"
};

/************************************************************************
 *      cit_herror_handler()	handle hardware errors
 ************************************************************************/
int cit_herror_handler(int errval,int ax,int bp,int si)
{
   static char msg[80];
   unsigned di;
   int drive;
   int errorno;

   di= _DI;
/* if this is not a disk error then it was another device having trouble */

   if (ax < 0)
   {
	  /* report the error */
	  mPrintf("\nDevice error\n");
	  /* and return to the program directly
	  requesting abort */
	  hardretn(ABORT);
   }
/* otherwise it was a disk error */
   drive = ax & 0x00FF;
   errorno = di & 0x00FF;
/* report which error it was */
   sprintf(msg,"\nError: %s on drive %c\n",err_msg[errorno],'A'+drive);

   mPrintf(msg);
/* return to the program via dos interrupt 0x23 with abort, retry or */
/* ignore as input by the user.  */
   hardretn(ABORT);
   return ABORT;
}

#define CT_SECS_DAY (60L*60L*24L)
/*FF*********************************************************************
FUNCT: CLOCK_ADDRESS                                     10-20-1991 12:41
CALLS: MK_FP
************************************************************************/
#define CLOCK_ADDRESS   ((unsigned long far *) MK_FP(0x0000,0x046C))

static unsigned long ct_last_time = 0L;
static unsigned long ct_time_ofs = 0L;
/*FF*********************************************************************
FUNCT: cit_init_timer                                    10-20-1991 12:41
USERS: main
CALLS: time              cit_timer
GLOBL: ct_last_time      ct_time_ofs
************************************************************************/

/************************************************************************
 *      cit_init_timer()    Initialize the fast citadel timer & timer
 *                          routines cit_timer() and cit_time().
 ************************************************************************/

void cit_init_timer(void)
{
    ct_last_time = 0L;
    ct_time_ofs = time(0L) - cit_timer();
}
/*FF*********************************************************************
FUNCT: cit_time                                          10-20-1991 12:41
USERS: main
CALLS: cit_timer
GLOBL: ct_time_ofs
************************************************************************/

/************************************************************************
 *      cit_time()      fast approximate time for Citadel
 *                      returns time in seconds
 *                      accounts for midnight rollover
 ************************************************************************/

unsigned long cit_time(void)
{
    return cit_timer() + ct_time_ofs;
}
/*FF*********************************************************************
FUNCT: cit_timer                                         10-20-1991 12:41
USERS: cit_init_timer    cit_time          main
DEFIN: CLOCK_ADDRESS     CT_SECS_DAY
GLOBL: ct_last_time
LOC/P: timer
************************************************************************/

/************************************************************************
 *      cit_timer()     fast approximate timer for Citadel
 *                      returns timer in seconds
 *                      accounts for midnight rollover
 ************************************************************************/

unsigned long cit_timer(void)
{
    unsigned long timer;

    timer = *(CLOCK_ADDRESS);
    timer /= 18L;
    if (timer < ct_last_time) {
        timer += CT_SECS_DAY;  	/* increment by one day                */
		ct_time_ofs = time(0L) - cit_timer();
	}
    ct_last_time = timer;
    return timer;
}
/************************************************************************
 *      special_pressed()	true if ALT, Shift, or CTRL pressed
 ************************************************************************/
#define SPECIAL_KEYS	0xF00F
int special_pressed(void)
{
	return (*((unsigned int far *) MK_FP(0x0000,0x0417)) & SPECIAL_KEYS);
}

