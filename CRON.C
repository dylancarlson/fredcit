/* $Header:   D:/VCS/FCIT/CRON.C_V   1.48   01 Nov 1991 11:19:48   FJM  $ */
/* -------------------------------------------------------------------- */
/*  CRON.C                       Citadel                                */
/* -------------------------------------------------------------------- */
/*  This file contains all the code to deal with the cron events        */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Includes                                                            */
/* -------------------------------------------------------------------- */
#include <string.h>
#include <time.h>
#include "ctdl.h"
#include "keywords.h"
#include "proto.h"
#include "global.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*                                                                      */
/*  readcron()      reads cron.cit values into events structure         */
/*  do_cron()       called when the system is ready to do an event      */
/*  cando_event()   Can we do this event?                               */
/*  do_event()      Actualy do this event                               */
/*  list_event()    List all events                                     */
/*  cron_commands() Sysop Fn: Cron commands                             */
/*  zap_event()     Zap an event out of the cron list                   */
/*  reset_event()   Reset an even so that it has not been done          */
/*  done_event()    Set event so it seems to have been done             */
/*  did_net()       Set all events for a node to done                   */
/*                                                                      */
/* -------------------------------------------------------------------- */


/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/CRON.C_V  $
 *
 *    Rev 1.48   01 Nov 1991 11:19:48   FJM
 * Added gl_ structures
 *
 *    Rev 1.47   21 Sep 1991 10:18:32   FJM
 * FredCit release
 *
 *    Rev 1.46   08 Jul 1991 16:17:58   FJM
 * Changed auto-cron loop method.
 *
 *    Rev 1.45   15 Jun 1991  8:37:00   FJM
 *
 *    Rev 1.44   06 Jun 1991  9:18:34   FJM
 *
 *    Rev 1.34   05 Feb 1991 14:30:52   FJM
 * Changes for screen saver.
 *
 *    Rev 1.33   28 Jan 1991 13:10:18   FJM
 * Improved cron list display.
 * Added cronNextEvent().
 *
 *    Rev 1.26   13 Jan 1991  0:30:40   FJM
 * Name overflow fixes.
 *
 *    Rev 1.20   06 Jan 1991 12:45:08   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.19   27 Dec 1990 20:16:08   FJM
 * Changes for porting.
 *
 *    Rev 1.16   22 Dec 1990 13:37:54   FJM
 *    Rev 1.11   16 Dec 1990 18:12:48   FJM
 *    Rev 1.10   09 Dec 1990 15:22:02   FJM
 * Prettier cron table.
 *
 *    Rev 1.9   07 Dec 1990 23:10:06   FJM
 *    Rev 1.8   02 Dec 1990  0:53:46   FJM
 * Enhanced local network events.
 *
 *    Rev 1.7   01 Dec 1990  2:57:44   FJM
 *    Rev 1.6   01 Dec 1990  2:54:24   FJM
 * Added support for local netting.
 *
 *    Rev 1.5   24 Nov 1990  3:44:14   FJM
 *    Rev 1.4   23 Nov 1990 13:22:34   FJM
 *    Rev 1.2   17 Nov 1990 16:11:40   FJM
 * Added version control log header
 *
 * --------------------------------------------------------------------
 *
 *  EARLY HISTORY:
 *
 *  05/07/89    (PAT)   Made history, cleaned up comments, reformated
 *                      icky code. Also added F6CAll Done
 *  03/19/90    FJM     Linted & partial cleanup
 *  04/17/90    FJM     Clean up on readcron()
 *  06/06/90    FJM     Changed strftime to cit_strftime
 *  06/09/90    FJM     Added readCrontab & writeCrontab
 *  07/30/90    FJM     Corrected bug in readcron() case for any day,
 *                      was expecting MAXGROUPS days, not 7!
 *  08/20/90    FJM     EXIT events added.
 *
 *
 * -------------------------------------------------------------------- */

static void do_event(int evnt);
static void zap_event(void);
static void reset_event(void);
static void done_event(void);

/* -------------------------------------------------------------------- */
/*  Static Data                                                         */
/* -------------------------------------------------------------------- */
static struct event events[MAXCRON];
static int on_event = 0;
static int numevents = 0;

/* -------------------------------------------------------------------- */
/*  External data                                                       */
/* -------------------------------------------------------------------- */
extern int local_mode;

/* -------------------------------------------------------------------- */
/*      readcron()     reads cron.cit values into events structure      */
/* -------------------------------------------------------------------- */
void readcron(void)
{
    FILE *fBuf;
    char line[91];
    char *words[50];
    int i, j, k, l, count;
    int cronslot = ERROR;
    int hour;


    /* move to home-path */
    changedir(cfg.homepath);

    if ((fBuf = fopen("cron.cit", "r")) == NULL) {  /* ASCII mode */
        cPrintf("Can't find Cron.cit!");
        doccr();
        exit(1);
    }
    on_event = 0;
    line[90] = '\0';

    /* clear cron table before reading file. */
    for (i = 0; i < MAXCRON; ++i) {
        events[i].e_type = ERROR;
    }

    while (fgets(line, 90, fBuf)) {
        if (line[0] != '#')
            continue;

        count = parse_it(words, line);

        for (i = 0; cronkeywords[i]; i++) {
            if (strcmpi(words[0], cronkeywords[i]) == SAMESTRING) {
                break;
            }
        }

        switch (i) {
            case CR_DAYS:
                if (cronslot == ERROR)
                    break;

        /* init days */
                for (j = 0; j < 7; j++)
                    events[cronslot].e_days[j] = 0;

                for (j = 1; j < count; j++) {
                    for (k = 0; daykeywords[k]; k++) {
                        if (strcmpi(words[j], daykeywords[k]) == SAMESTRING) {
                            break;
                        }
                    }
                    if (k < 7)
                        events[cronslot].e_days[k] = TRUE;
                    else if (k == 7) {  /* any */
                        for (l = 0; l < 7; ++l) /* was in error!  NOT MAXGROUPS */
                            events[cronslot].e_days[l] = TRUE;
                    } else {
                        doccr();
                        cPrintf("Cron.cit - Warning: Unknown day %s ", words[j]);
                        doccr();
                    }
                }
                break;

            case CR_DO:
                cronslot = (cronslot == ERROR) ? 0 : (cronslot + 1);

                if (cronslot > MAXCRON) {
                    doccr();
                    illegal("Cron.Cit - too many entries");
                }
                for (k = 0; crontypes[k] != NULL; k++) {
                    if (strcmpi(words[1], crontypes[k]) == SAMESTRING)
                        events[cronslot].e_type = (uchar) k;
                }

                strncpy(events[cronslot].e_str, words[2], NAMESIZE);
                events[cronslot].e_str[NAMESIZE-1] = '\0';
                events[cronslot].l_sucess = (long) 0;
                events[cronslot].l_try = (long) 0;
                break;

            case CR_HOURS:
                if (cronslot == ERROR)
                    break;

        /* init hours */
                for (j = 0; j < 24; j++)
                    events[cronslot].e_hours[j] = 0;

                for (j = 1; j < count; j++) {
                    if (strcmpi(words[j], "Any") == SAMESTRING) {
                        for (l = 0; l < 24; ++l)
                            events[cronslot].e_hours[l] = TRUE;
                    } else {
                        hour = atoi(words[j]);

                        if (hour > 23) {
                            doccr();
                            cPrintf("Cron.Cit - Warning: Invalid hour %d ",
                            hour);
                            doccr();
                        } else
                            events[cronslot].e_hours[hour] = TRUE;
                    }
                }
                break;

            case CR_REDO:
                if (cronslot == ERROR)
                    break;

                events[cronslot].e_redo = atoi(words[1]);
                break;

            case CR_RETRY:
                if (cronslot == ERROR)
                    break;

                events[cronslot].e_retry = atoi(words[1]);
                break;

            default:
                cPrintf("Cron.cit - Warning: Unknown variable %s", words[0]);
                doccr();
                break;
        }
    }
    fclose(fBuf);

    numevents = cronslot;

    for (i = 0; i < MAXCRON; i++) {
        events[i].l_sucess = (long) 0;
        events[i].l_try = (long) 0;
    }
}

/* --------------------------------------------------------------------
 *  cronNextEvent()		Set next cron event to do.
 * -------------------------------------------------------------------- */
int cronNextEvent(int evnt)
{
	if(evnt < 0 || evnt > numevents)
		return ERROR;
	else {
		on_event = evnt;
		return SUCCESS;
	}
}

/* --------------------------------------------------------------------
 * do_cron_loop()		Loop through cron events
 * -------------------------------------------------------------------- */

void do_cron_loop(void)
{
	int was_event;
	int lastFlag;

	/* hold last output state & become impervious */
	lastFlag = outFlag;
	outFlag = IMPERVIOUS;
	was_event = on_event;
	/* spin loop */
	while (do_cron(1) && was_event != on_event)
		;
	/* restore last output state */
	outFlag = lastFlag;
}

/* -------------------------------------------------------------------- */
/*  do_cron()       called when the system is ready to do an event      */
/* -------------------------------------------------------------------- */
int do_cron(int why_called)
{
    int was_event, done, rc;

    /*
     * why_called = why_called; to prevent a -W3 warning, the varible will be used latter
     */
    was_event = on_event;
    done = FALSE;

    do {
		cit_init_timer();		/* resync cit_time() and cit_timer()	*/
        if (cando_event(on_event)) {
			do_idle(0);			/* clear screen save state */
            do_event(on_event);
			do_idle(2);			/* reset screen saver state */
            done = TRUE;
        }
        on_event = on_event < numevents ? on_event + 1 : 0;
    } while (!done && on_event != was_event);

    if (!done || (events[was_event].e_type == CR_EXIT)) {
    /* done or */
    /* exit event dumps loops */
        if (debug)
            mPrintf("(No event to do) ");
        Initport();
        rc = FALSE;
    } else {
		doCR();
		rc = !(on_event == was_event);
	}
	return rc;
}

/* -------------------------------------------------------------------- */
/*  cando_event()   Can we do this event?                               */
/* -------------------------------------------------------------------- */
int cando_event(int evnt)
{
    long last;
	int rc;

	last = cit_time();
	
    /* not a valid (posible zaped) event */
    if (events[evnt].e_type == ERROR) {
        rc = FALSE;

    /* not right time || day */
    } else if (!events[evnt].e_hours[hour()] ||
		!events[evnt].e_days[dayofweek()]) {
        rc = FALSE;

    /* already done, wait a little longer */
    } else if ((last -
		events[evnt].l_sucess) / 60L < (long) events[evnt].e_redo) {
        rc = FALSE;

    /* didnt work, give it time to get un-busy */
    } else if ((last -
		events[evnt].l_try) / 60L < (long) events[evnt].e_retry) {
        rc = FALSE;
	} else {
		rc = TRUE;
	}
	
	return rc;
}

/* -------------------------------------------------------------------- */
/*  do_event()      Actualy do this event                               */
/* -------------------------------------------------------------------- */
static void do_event(int evnt)
{
	static char no_net[] = "Can not network with user logged in.\n ";

    switch (events[evnt].e_type) {
        case CR_SHELL_1:
            mPrintf("SHELL: \"%s\"", events[evnt].e_str);
            if (changedir(cfg.aplpath) == ERROR) {
                mPrintf("  -- Can't find application directory.\n\n");
                changedir(cfg.homepath);
                return;
            }
            apsystem(events[evnt].e_str);
            changedir(cfg.homepath);
            events[evnt].l_sucess = events[evnt].l_try = cit_time();
            Initport();
            break;
        case CR_LOCAL:
            if (!loggedIn) {
                local_mode = TRUE;
                mPrintf("LOCAL: with \"%s\"", events[evnt].e_str);
                if (net_callout(events[evnt].e_str))
                    did_net(events[evnt].e_str);
                events[evnt].l_try = cit_time();
            } else {
                mPrintf(no_net);
            }
            break;
        case CR_NET:
            if (!loggedIn) {
                mPrintf("NETWORK: with \"%s\"", events[evnt].e_str);
                if (net_callout(events[evnt].e_str))
                    did_net(events[evnt].e_str);
                events[evnt].l_try = cit_time();
            } else {
                mPrintf(no_net);
            }
            break;
        case CR_EXIT:
            ExitToMsdos = atoi(events[evnt].e_str) + 1;
            break;
        default:
            mPrintf(" Unknown event type %d, slot %d\n ", events[evnt].e_type, evnt);
            break;
    }
}

/* -------------------------------------------------------------------- */
/*  list_event()    List all events                                     */
/* -------------------------------------------------------------------- */
void list_event(void)
{
    int i;
    char dtstr[20];
    char next_chr = 175;
    char endnext_chr = 174;
    char do_chr = 254;

    mtPrintf(TERM_BOLD,
		"   No.     Type                Text    Redo Retry Last");
    doCR();

    for (i = 0; i <= numevents; i++) {
        if (events[i].e_type != ERROR) {

			if (on_event == i)
				termCap(TERM_REVERSE);
            mPrintf(" %c%c%02d%10s, %20s, %4d, %4d ",
				on_event == i ? next_chr : ' ',
				cando_event(i) ? do_chr : ' ',
				i,
				crontypes[events[i].e_type],
				events[i].e_str,
				events[i].e_redo,
				events[i].e_retry);
            if (events[i].l_try) {
                cit_strftime(dtstr, 19, "%X %y%b%D", events[i].l_try);
                mPrintf("%s", dtstr);
            } else
                mPrintf("                ");
            mPrintf("%c%c", cando_event(i) ? do_chr : ' ',
				on_event == i ? endnext_chr : ' ');
			if (on_event == i)
				termCap(TERM_NORMAL);

            doCR();
        }
    }
}

/* -------------------------------------------------------------------- */
/*  cron_commands() Sysop Fn: Cron commands                             */
/* -------------------------------------------------------------------- */
void cron_commands(void)
{
    int i;
    static char no_event[] = "\n No cron events\n";

    switch (toupper(iChar())) {
        case 'A':
            mPrintf("\bAll Done\n ");
            doCR();
            mPrintf("Seting all events to done...");
            for (i = 0; i < MAXCRON; i++) {
                events[i].l_try = events[i].l_sucess = cit_time();
            }
            doCR();
            break;
        case 'D':
            mPrintf("\bDone event\n ");
            if (numevents == ERROR) {
                mPrintf(no_event);
            } else {
				done_event();
			}
            break;
        case 'E':
            mPrintf("\bEnter Cron file\n ");
            readcron();
            break;
        case 'L':
            mPrintf("\bList events");
            doCR();
            doCR();
            list_event();
            break;
		case 'N':
            mPrintf("\bNext event");
			cronNextEvent((int)getNumber("next event number",0L,
				(long)numevents,(long)on_event));
			break;
        case 'R':
            mPrintf("\bReset event\n ");
            if (numevents == ERROR) {
                mPrintf(no_event);
            } else {
				reset_event();
			}
            break;
//        case 'U':
//            mPrintf("\bUnzap event\n ");
//            if (numevents == ERROR) {
//                mPrintf(no_event);
//            } else {
//				unzap_event();
//			}
//            break;
        case 'Z':
            mPrintf("\bZap event\n ");
            if (numevents == ERROR) {
                mPrintf(no_event);
            } else {
				zap_event();
			}
            break;
        case '?':
            doCR();
            doCR();
            mpPrintf(" <A>ll done\n");
			mpPrintf(" <D>one event\n");
			mpPrintf(" <E>nter Cron file\n");
            mpPrintf(" <L>ist event\n");
			mpPrintf(" <N>ext event\n");
			mpPrintf(" <R>eset event\n");
//			mpPrintf(" <U>nzap event\n");
			mpPrintf(" <Z>ap event\n");
			mpPrintf(" <?> -- this menu\n ");
            break;
        default:
            if (!gl_user.expert)
                mPrintf("\n '?' for menu.\n ");
            else
                mPrintf(" ?\n ");
            break;
    }
}

/* -------------------------------------------------------------------- */
/*  zap_event()     Zap an event out of the cron list                   */
/* -------------------------------------------------------------------- */
static void zap_event(void)
{
    int i;

    i = (int) getNumber("event", 0L, (long) numevents, (long) on_event);
    if (i == ERROR)
        return;
    events[i].e_type = ERROR;
}

/* -------------------------------------------------------------------- */
/*  reset_event()   Reset an even so that it has not been done          */
/* -------------------------------------------------------------------- */
static void reset_event(void)
{
    int i;

    i = (int) getNumber("event", 0L, (long) numevents, (long) on_event);
    if (i == ERROR)
        return;
    events[i].l_sucess = 0L;
    events[i].l_try = 0L;
}

/* -------------------------------------------------------------------- */
/*  done_event()    Set event so it seems to have been done             */
/* -------------------------------------------------------------------- */
static void done_event(void)
{
	int i;
    if((i=getNumber("event", 0L, (long) numevents, (long) on_event)) != ERROR)
		events[i].l_sucess = events[i].l_try = cit_time();
}

/* -------------------------------------------------------------------- */
/*  did_net()       Set all events for a node to done                   */
/* -------------------------------------------------------------------- */
void did_net(char *callnode)
{
    int i;

    for (i = 0; i <= numevents; i++) {
        if (strcmpi(events[i].e_str, callnode) == SAMESTRING &&
            (events[i].e_type == CR_NET) ||
			(events[i].e_type == CR_LOCAL)) {
            events[i].l_sucess = events[i].l_try = cit_time();
        }
    }
}

/* --------------------------------------------------------------------

        writeCrontab() -  write cron events to a file

   -------------------------------------------------------------------- */

void writeCrontab(void)
{
    FILE *fp;
    const char *name = "cron.tab";

    if ((fp = fopen(name, "wb")) == NULL) {
        crashout("Can't make Cron.tab");
    }
    /* write out Cron.tab */
    fwrite(events, sizeof(struct event), MAXCRON, fp);
    fclose(fp);
}

/* --------------------------------------------------------------------

        readCrontab() -  read cron events from file

   -------------------------------------------------------------------- */

void readCrontab(void)
{
    FILE *fp;
    const char *name = "cron.tab";

    fp = fopen(name, "rb");
    if (fp) {
        fread(events, sizeof(struct event), MAXCRON, fp);
        fclose(fp);
        unlink(name);
    }
}
