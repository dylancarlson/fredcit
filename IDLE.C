/* --------------------------------------------------------------------
 *  IDLE.C                   Citadel
 * --------------------------------------------------------------------
 *  This file contains the IDLE functions
 * -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Includes                                                            */
/* -------------------------------------------------------------------- */
#include <string.h>
#include <time.h>
#include <alloc.h>
#include "ctdl.h"

#ifndef ATARI_ST
#include <conio.h>
#endif

#include "proto.h"
#include "global.h"

/* --------------------------------------------------------------------
 *                              Contents
 * --------------------------------------------------------------------
 *
 *
 *
 * -------------------------------------------------------------------- */


/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/IDLE.C_V  $
 * 
 *    Rev 1.13   01 Nov 1991 11:20:10   FJM
 * Added gl_ structures
 *
 *    Rev 1.12   21 Sep 1991 10:18:54   FJM
 * FredCit release
 *
 *    Rev 1.11   06 Jun 1991  9:19:04   FJM
 *
 *    Rev 1.10   27 May 1991 11:42:40   FJM
 *
 *    Rev 1.9   22 May 1991  2:16:08   FJM
 *    Rev 1.8   16 May 1991  8:44:12   FJM
 * Speedups.
 *
 *    Rev 1.5   17 Apr 1991 12:56:12   FJM
 *    Rev 1.0   28 Feb 1991 12:17:10   FJM
 * Initial revision.
 * -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  External data                                                       */
/* -------------------------------------------------------------------- */

/*
 * idle_flag true if idle
 *
 */

void do_idle(int idle_flag)
{
	static enum {I_INITIAL,I_NORMAL,I_IN_IDLE} idle_state=I_INITIAL;
	long timer;
	static long last_time=0L;
	static int loopcnt=0;
	char save_attr;

	/* need to optimize timers like in iChar() */
	timer = cit_timer();

	if (cfg.screen_save) {
		switch (idle_flag) {
			case 0:				/* clear idle state */
				if (idle_state == I_IN_IDLE) {
					idle_state = I_NORMAL;
					restore_screen();
					textbackground(cfg.battr);
					curson();
				}
				++loopcnt;
				if (parm.memcheck && !(loopcnt % 100)) {
					/* do these one time in 100 */
					if (farheapcheck() < 0)
						crashout("memory corrupted 2");
				}
				update25();
				last_time = timer;
			break;
			case 1:				/* maybe idle state	*/
				if (idle_state == I_NORMAL) {
					if (!loggedIn &&
						timer - last_time >= (long)cfg.screen_save) {
						idle_state = I_IN_IDLE;
						/* save all screen info	*/
						save_screen();
						cursoff();
						save_attr = cfg.attr;
						cfg.attr = 7;
						setscreen();
						cls();
						textbackground(0);
						cfg.attr = save_attr;
					} else {
						/* display the time */
						update25();
					}
				} else if (idle_state == I_INITIAL) {
					idle_state = I_NORMAL;
					last_time = timer;
				}				/* else still idle		*/
			break;
			case 2:				/* ignore elapsed time	*/
				if (idle_state == I_IN_IDLE)
					do_idle(0);		/* recurse to do restore */
				idle_state = I_INITIAL;
			break;
		}
	} else
		update25();
}

