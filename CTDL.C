/************************************************************************
 *                              ctdl.c
 *              Command-interpreter code for Citadel
 ************************************************************************/

#define MAIN

#include <string.h>
#include <time.h>
#include "ctdl.h"

#ifndef ATARI_ST
#include <dos.h>
#include <conio.h>
#endif

#include "keywords.h"
#include "proto.h"
#include "global.h"

unsigned int _stklen = 1024 * 18;

/* unsigned int _ovrbuffer = 0x800; */

void interrupt ( *oldhandler)(void);

extern void interrupt criterr(unsigned bp,unsigned si,unsigned ds,unsigned es,unsigned dx,
	unsigned cx,unsigned bx,unsigned ax);
	
/************************************************************************
 *                              Contents
 *
 *      main()                  has the central menu code
 *
 ************************************************************************/


/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/CTDL.C_V  $
 *
 *    Rev 1.51   01 Nov 1991 11:19:50   FJM
 * Added gl_ structures
 *
 *    Rev 1.50   21 Sep 1991 10:18:34   FJM
 * FredCit release
 *
 *    Rev 1.49   08 Jul 1991 16:18:12   FJM
 * Changed cron looping.
 *
 *    Rev 1.48   15 Jun 1991  8:37:06   FJM
 *
 *    Rev 1.47   06 Jun 1991  9:18:38   FJM
 *
 *    Rev 1.46   27 May 1991 11:41:42   FJM
 *
 *    Rev 1.45   22 May 1991  2:15:14   FJM
 * Made memory check optional.
 *
 *    Rev 1.44   06 May 1991 17:28:26   FJM
 * Added new prompt code.
 *
 *    Rev 1.42   19 Apr 1991 23:26:22   FJM
 *    Rev 1.41   17 Apr 1991 12:55:06   FJM
 *    Rev 1.35   10 Feb 1991 17:32:50   FJM
 * Changes to shell mode carrier detect.
 *
 *    Rev 1.33   05 Feb 1991 14:31:04   FJM
 *    Rev 1.32   28 Jan 1991 13:10:42   FJM
 *    Rev 1.30   19 Jan 1991 14:14:48   FJM
 * Added time display, and properly init parm struct.
 *
 *    Rev 1.25   13 Jan 1991  0:30:46   FJM
 * Name overflow fixes.
 *
 *    Rev 1.23   11 Jan 1991 12:42:52   FJM
 *    Rev 1.22   06 Jan 1991 16:49:46   FJM
 * Added -l parameter.  Made -d work.
 *
 *    Rev 1.19   06 Jan 1991 12:45:02   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.18   27 Dec 1990 20:16:30   FJM
 * Changes for porting.
 *
 *    Rev 1.15   22 Dec 1990 13:38:58   FJM
 *    Rev 1.10   16 Dec 1990 18:12:52   FJM
 *    Rev 1.8   07 Dec 1990 23:10:08   FJM
 *    Rev 1.6   24 Nov 1990  3:07:24   FJM
 * Changes for shell/door mode.
 *
 *    Rev 1.5   23 Nov 1990 18:24:06   FJM
 * Changes to login for shell mode.
 *
 *    Rev 1.4   23 Nov 1990 13:22:38   FJM
 *    Rev 1.2   17 Nov 1990 16:11:38   FJM
 * Added version control log header
 *
 * --------------------------------------------------------------------
 *
 *  Early History:
 *
 *  03/08/90    {zm}    Add 'softname' as the name of the software.
 *  03/15/90    {zm}    Add [title] name [surname], 30 characters long.
 *  03/19/90    FJM     Linted & partial cleanup
 *  05/15/90    FJM     Changed Next/previous room/hall prompts
 *  05/17/90    FJM     Added external commands
 *  05/18/90    FJM     Made = an alias for + at room prompt
 *  05/20/90    FJM     Improved external commands. Message if can't enter
 *                      surname.  Made NOCHAT.BLB,NOCHAT1.BLB... cycle
 *  06/06/90    FJM     Changed strftime to cit_strftime
 *  06/15/90    FJM     Added option for modem output pacing, -p.
 *                      Added help display if incorrect option.
 *                      Mods for running from another BBS as a door.
 *  06/16/90    FJM     Made IBM Graphics characters a seperate option.
 *  06/20/90    FJM     Moved most functions to COMMAND.C
 *  07/22/90    FJM     Added -b (baud) option.
 *  07/23/90    FJM     Added -v (events) option.
 *  07/24/90    FJM     Increased overlay buffer size from 22k to 32k.
 * -------------------------------------------------------------------- */

/************************************************************************
 *              Local function definitions for CTDL.C
 ************************************************************************/

static void command_loop(void);

/************************************************************************
 *      command_loop() contains the central menu code
 ************************************************************************/

static void command_loop(void)
{
	char c, more = FALSE, help = FALSE;
    int i;

    while (!ExitToMsdos) {
		
        if (sysReq && !loggedIn && !haveCarrier) {
            sysReq = FALSE;
			if (cfg.offhook) {
                offhook();
            } else {
                drop_dtr();
            }
            ringSystemREQ();
        }
		
		if (parm.door) {
			if (!loggedIn && cfg.forcelogin) {
				for (i = 0; !loggedIn && i < 4; ++i)
					doLogin(2);
				if (!loggedIn) {
					ExitToMsdos = 1;
					break;
				}
			}
		}
		
		more = getCommand(&c);
		
		outFlag = IMPERVIOUS;

		if (chatkey)
			chat();
		
		if (eventkey && !haveCarrier) {
			do_cron(CRON_TIMEOUT);
			eventkey = FALSE;
		}
		
		if (sysopkey)
			help = doSysop();
		else
			help = doRegular(more, c);

		if (help) {
			if (!gl_user.expert)
				mPrintf("\n '?' for menu, 'H' for help.\n \n");
			else
				mPrintf(" ?\n \n");
		}
    }
}

/************************************************************************
 *      main() Initialize & start citadel
 ************************************************************************/
void main(int argc, char *argv[])
{
    int i, cfg_ctdl = FALSE;
    long b;
    char init_baud;
    static char prompt[92];
    char *envprompt;

    cfg.bios = 1;
	cit_init_timer();			/* initialize cit_time() and cit_timer()	*/
    cfg.attr = 7;       		/* logo gets white letters                  */
    setscreen();				/* initialize screen system					*/
	memset(&parm,0,sizeof(parm));
    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '/'
        || argv[i][0] == '-') {
            switch (tolower(argv[i][1])) {
#ifndef ATARI_ST
                case 'd':       /* DesqView/TopView     */
                    parm.dv = 1;
                    cPrintf("DesqView/TopView mode\n");
                    break;

                case 'b':       /* baud rate (for shell) */
                    if (argv[i + 1]) {
						b = atol(argv[++i]);
                        for (init_baud = 0; bauds[init_baud]; ++init_baud)
                            if (bauds[init_baud] == b) {
                                parm.baud = init_baud;
                                cPrintf("Initial baud rate fixed at %ld\n", b);
                                break;
							}
                    }
                    break;
				case 'm':
					parm.memcheck = !parm.memcheck;
					break;
#endif
                case 'c':       /* Configure system     */
                    cfg_ctdl = TRUE;
                    break;

                case 'p':       /* pace output          */
                    if (argv[i + 1]) {
                        parm.pace = atoi(argv[++i]);
                        cPrintf("Output pacing %d\n", parm.pace);
                    }
                    break;

                case 's':       /* run in shell from another BBS (door).        */
                    cPrintf("Shell mode\n");
                    parm.door = TRUE;
                    break;
/*#ifndef ATARI_ST*/
#ifndef FLOPPY
                case 'e':       /* use EMS                      */
                    if (_OvrInitEms(0, 0, 0)) {
                        cPrintf("EMS memory initialization failed!\n");
                        parm.ems = 1;
                    }
                    break;
#endif
/*#endif*/
				case 'v':       /* just do events       */
                    parm.events = 1;
                    break;
/*#ifndef ATARI_ST*/
#ifndef FLOPPY
                case 'x':       /* use exteneded memory */
					if (_OvrInitExt(0, 0))
                        cPrintf("Extended memory initialization failed!\n");
                    parm.ext = 1;
                    break;
#endif
/*#endif*/
				case 'l':		/* log in user */
                    if (argv[i + 1]) {
						parm.login = argv[++i];
						cPrintf("Auto-login\n");
					}
					break;

				case 'u':		/* log in user */
                    if (argv[i + 1]) {
						parm.user = argv[++i];
						cPrintf("Auto-login %s\n", parm.user);
					}
					break;
					
                default:
                    cPrintf("\nUnknown commandline switch '%s'.\n", argv[i]);
                    cPrintf("Valid DOS command line switches:\n");
                    cPrintf("    -b baud   Starting baud rate (300-19200)\n");
                    cPrintf("    -c        Read configuration files\n");
                    cPrintf("    -d        DesqView/TopView\n");
#ifndef FLOPPY
                    cPrintf("    -e        Use EMS memory for overlays\n");
#endif
					cPrintf("    -l str    Log in using initials;password in str\n");
					cPrintf("    -m        Memory check during idle time, start\n");
                    cPrintf("    -p num    Set output pacing to num\n");
                    cPrintf("    -s        Run as a shell from another BBS\n");
					cPrintf("    -u 'name' Log in using specifed user name");
                    cPrintf("    -v        Just run cron events\n");
#ifndef FLOPPY
					cPrintf("    -x        Use extended memory for overlays (386/486 only!)\n");
#endif
                    exit(1);
            }
        }
    }

    if (cfg_ctdl)				/* force reconfigure?						*/
        unlink("etc.tab");

    logo();						/* prints out system logo                   */

    if (cit_time() < 607415813L) {
        cPrintf("\n\nPlease set your time and date!\n");
        exit(1);
    }
	/* set prompt for shells */
    envprompt = getenv("PROMPT");
	if (!envprompt)
		envprompt = "$p$g";
    sprintf(prompt, "PROMPT=\r\nType EXIT to return to FredCit\r\n%s", envprompt);
    if (putenv(prompt)) {
        cPrintf("\n\nCan not set DOS prompt!\n");
		delay (5000);
	}
	/* initialize citadel */
    initCitadel();

    if (parm.baud) {
        cfg.initbaud = parm.baud;
        baud(cfg.initbaud);
    }
	if (parm.door) {
		detectflag = 1;
//		carrier();
//		if (haveCarrier) {
//			carrdetect();
//			newCarrier = 1;		/* make hello blurb show up */
//		}
	}
    greeting();

	sysReq = FALSE;

	if (cfg.f6pass[0])
		ConLock = TRUE;

	if (parm.dv) {
		cfg.bios = 1;
		directvideo = 0;
	}

	if (parm.login) {
		normalizeString(parm.login);	/* normalize string in environment */
		login(parm.login,NULL);
	} else if (parm.user && !loggedIn) {
		normalizeString(parm.user);	/* normalize string in environment */
		if (findPerson(parm.user, &lBuf) != ERROR)
			login(lBuf.lbin,lBuf.lbpw);
	}

		/* read in door interface files */
	if (parm.door) {
		readDoorFiles(0);
	}
	/* update25();	*/
	do_idle(0);

	/* install critical error handler */
	harderr(cit_herror_handler);
	
	/* execute main command loop */
    if (!parm.events)
        command_loop();
    else {
        do_cron_loop();
    }

    exitcitadel();
}

/* EOF */

