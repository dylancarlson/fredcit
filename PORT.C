/* $Header:   D:/VCS/FCIT/PORT.C_V   1.43   01 Nov 1991 11:20:46   FJM  $ */
/* -------------------------------------------------------------------- */
/*  PORT.C                    Citadel                                   */
/* -------------------------------------------------------------------- */
/*  This module should contain all the code specific to the modem       */
/*  hardware. This is done in an attempt to make the code more portable */
/*                                                                      */
/*  Note: under the MS-DOS implementation there is also an ASM file     */
/*  contains some of the very low-level io rutines.                     */
/* -------------------------------------------------------------------- */



/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/PORT.C_V  $
 * 
 *    Rev 1.43   01 Nov 1991 11:20:46   FJM
 * Added gl_ structures
 *
 *    Rev 1.42   08 Jul 1991 16:19:42   FJM
 *
 *    Rev 1.41   29 May 1991 11:20:08   FJM
 * Removed redundent includes, added portDump() and portFlush(), macro usage.
 *
 *    Rev 1.38   17 Apr 1991 12:55:50   FJM
 *    Rev 1.21   11 Jan 1991 12:43:44   FJM
 *    Rev 1.17   06 Jan 1991 12:46:02   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.16   27 Dec 1990 20:16:32   FJM
 * Changes for porting.
 *
 *    Rev 1.13   22 Dec 1990 13:39:02   FJM
 *    Rev 1.8   16 Dec 1990 18:13:32   FJM
 *    Rev 1.5   23 Nov 1990 13:25:22   FJM
 * Header change
 *
 *    Rev 1.3   17 Nov 1990 22:42:30   FJM
 * Prevented output of modem init string in shell mode.
 *
 *    Rev 1.2   17 Nov 1990 16:12:00   FJM
 * Added version control log header
 * -------------------------------------------------------------------- */

/* --------------------------------------------------------------------
 *  History:
 *
 *  03/02/90    FJM     Added IBMCOM-C drivers to replace ASM ones
 *  06/15/90    FJM     Added option for modem output pacing.
 *                      Mods for running from another BBS as a door.
 * -------------------------------------------------------------------- */


/* -------------------------------------------------------------------- */
/*  Includes                                                            */
/* -------------------------------------------------------------------- */
#include <time.h>
#include <dos.h>
#include "ctdl.h"
#include "proto.h"
#include "global.h"

void raise_dtr(void);

#ifdef OLD_COMM
extern void INT_INIT(int, int);
extern void COM_EXIT(void);
extern void COM_INIT(int, int, int, int);
extern unsigned int COM_STAT(void);
extern int COM_READ(void);
extern void COM_WRITE(unsigned char);
extern void DROP_DTR(void);
extern void RAISE_DTR(void);

#endif

/* --------------------------------------------------------------------
                                Contents
   --------------------------------------------------------------------
        baud()          sets serial baud rate
        carrier()       checks carrier
        drop_dtr()      turns DTR off
        getMod()        bottom-level modem-input
        outMod()        bottom-level modem output
        Hangup()        breaks modem connection
        Initport()      sets up modem port and modem
        portExit()      Exit cserl.obj pakage
        portInit()      Init cserl.obj pakage
        ringdetect()    returns 1 if ring detect port is true
		portFlush() 	sends remaining characters out port, if no timeout
		portDump() 		grab remaining characters on input, or timeout in
						8 seconds
   --------------------------------------------------------------------

   --------------------------------------------------------------------
    HISTORY:

    07/23/89    (RGJ)   Fixed outMod to not overrun 300 baud on a 4.77
    04/06/89    (PAT)   Speed up outMod to keep up with 2400 on a 4.77
    04/01/89    (RGJ)   Changed outp() calls to not screw with the
                        CSERL.OBJ package.
    04/01/89    (PAT)   Moved outMod() into port.c
    03/22/89    (RGJ)   Change gotCarrier() and ringdetect() to
                        use COM_STAT() instead of polled I/O.
                        Also removed all outp's from baud().
                        And changed drop_dtr() and Hangup() so interupt
                        package is off when fiddling with DTR.
    02/22/89    (RGJ)   Change to Hangup, disable, initport
    02/07/89    (PAT)   Module created from MODEM.C
    03/02/90    FJM     Use of IBMCOM-C serial package

   -------------------------------------------------------------------- */


/* -------------------------------------------------------------------- */
/*  External data                                                       */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Static Data                                                         */
/* -------------------------------------------------------------------- */
static char modcheck = 1;

 /* COM     #1     #2  */
static char modemcheck[] = {1, 5, 10, 20, 40, 0};

/* -------------------------------------------------------------------- */
/*      ringdetect() returns true if the High Speed line is up          */
/*                   if there is no High Speed line avalible to your    */
/*                   hardware it should return the ring indicator line  */
/*                   In this way you can make a custom cable and use it */
/*                   that way                                           */
/* -------------------------------------------------------------------- */

/**********************************
*int ringdetect(void)             *
*{                                *
*    return com_ring();  * RI *   *
*}                                *
**********************************/

/* -------------------------------------------------------------------- */
/*      MOReady() is modem port ready for output                        */
/* -------------------------------------------------------------------- */
/***************************
*int MOReady(void)         *
*{                         *
*    return com_tx_ready();*
*}                         *
***************************/

/* -------------------------------------------------------------------- */
/*      MIReady() Ostensibly checks to see if input from modem ready    */
/* -------------------------------------------------------------------- */
/****************************
*int MIReady(void)          *
*{                          *
*    return !com_rx_empty();*
*}                          *
****************************/

/* -------------------------------------------------------------------- */
/*      Initport()  sets up the modem port and modem. No return value   */
/* -------------------------------------------------------------------- */
void Initport(void)
{
    haveCarrier = modStat = (char) gotCarrier();

    Hangup();

    disabled = FALSE;

    baud(cfg.initbaud);

    if (!parm.door) {
        outstring(cfg.modsetup);
        outstring("\r");
        delay(1000);
    }
    /* update25();	*/
	do_idle(0);
}

/* -------------------------------------------------------------------- */
/*      Hangup() breaks the modem connection                            */
/* -------------------------------------------------------------------- */
void Hangup(void)
{
	portFlush();
    drop_dtr();
    raise_dtr();
    delay(500);
}

/* -------------------------------------------------------------------- */
/*      gotCarrier() returns nonzero on valid carrier, else zero        */
/* -------------------------------------------------------------------- */
/**************************
*int gotCarrier(void)     *
*{                        *
*    return com_carrier();*
*}                        *
**************************/

/* -------------------------------------------------------------------- */
/*      getMod() is bottom-level modem-input routine                    */
/* -------------------------------------------------------------------- */
/*********************
*int getMod(void)    *
*{                   *
*    received++;     *
*    return com_rx();*
*}                   *
*********************/

/* -------------------------------------------------------------------- */
/*      drop_dtr() turns dtr off                                        */
/* -------------------------------------------------------------------- */
void drop_dtr(void)
{
    disabled = TRUE;

    if (parm.door)      /* supress initial hangup in door mode */
        return;

    com_lower_dtr();
    delay(500);
}

/* -------------------------------------------------------------------- */
/*      raise_dtr() turns dtr on                                        */
/* -------------------------------------------------------------------- */

void raise_dtr(void)
{
    com_raise_dtr();
}

/* -------------------------------------------------------------------- */
/*      baud() sets up serial baud rate  0=300; 1=1200; 2=2400; 3=4800  */
/*                                       4=9600 5=19200 6=38400         */
/*      and initializes port for general bbs usage   N81                */
/* -------------------------------------------------------------------- */


void baud(int baudrate)
{
    com_set_speed((long) bauds[baudrate]);
    com_set_parity(COM_NONE, 1);
    com_install(cfg.mdata);
    speed = baudrate;
    modcheck = modemcheck[speed];
}

/* -------------------------------------------------------------------- */
/*      outMod() stuffs a char out the modem port                       */
/* -------------------------------------------------------------------- */
void outMod(unsigned char ch)
{
    if (!modem && !callout)
        return;

    /* dont go faster than the modem, check every modcheck */
    if (!(transmitted % modcheck) && !MOReady()) {
		portFlush();
    }
    com_tx(ch);

    if (parm.pace) {        /* output pacing delay in 1/1000 seconds     */
        delay(parm.pace);
    }
    ++transmitted;      /* keep track of how many chars sent */
}

/* -------------------------------------------------------------------- */
/*      portInit() sets up the interupt driven I/O package              */
/* -------------------------------------------------------------------- */
void portInit(void)
{
    com_install(cfg.mdata);
}

/* -------------------------------------------------------------------- */
/*      portExit() removes the interupt driven I/O package              */
/* -------------------------------------------------------------------- */
void portExit(void)
{
	portFlush();
    com_deinstall();
}

/* --------------------------------------------------------------------
 *	portFlush() sends remaining characters out port, if no timeout (8 sec)
 * -------------------------------------------------------------------- */

void portFlush(void)
{
	if (!com_tx_empty()) {
		long timer;
	
		timer = cit_timer();
		while (!com_tx_empty()) {
			if ((cit_timer() - timer) >= 8L) {
                cPrintf("Modem write failed!\n");
				break;
			}
		}
	}
}

/* --------------------------------------------------------------------
 *	portDump() grab remaining characters on input, or timeout (8 seconds)
 * -------------------------------------------------------------------- */

void portDump(void)
{
	int ch;
	if (!com_tx_empty()) {
		long timer;
	
		timer = cit_timer();
		while (!com_rx_empty()) {
			ch = com_rx();
			if (debug)
				outCon(ch);
			if ((cit_timer() - timer) >= 8L) {
				break;
			}
		}
	}
}

