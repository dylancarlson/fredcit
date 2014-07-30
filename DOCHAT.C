/************************************************************************
 * $Header:   D:/VCS/FCIT/DOCHAT.C_V   1.30   01 Nov 1991 11:19:52   FJM  $
 *              Command-interpreter code for Citadel
 ************************************************************************/

#include <string.h>
#include <time.h>
#include "ctdl.h"

#ifndef ATARI_ST
#include <conio.h>
#include <dos.h>
#endif

#include "keywords.h"
#include "proto.h"
#include "global.h"

/************************************************************************
 *                              Contents
 *
 *      doChat()                handles C(hat)          command
 *
 ************************************************************************/


/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/DOCHAT.C_V  $
 * 
 *    Rev 1.30   01 Nov 1991 11:19:52   FJM
 * Added gl_ structures
 *
 *    Rev 1.29   21 Sep 1991 10:18:38   FJM
 * FredCit release
 *
 *    Rev 1.28   06 Jun 1991  9:18:42   FJM
 *
 *    Rev 1.27   27 May 1991 11:42:08   FJM
 *
 *    Rev 1.12   18 Jan 1991 16:53:48   FJM
 *    Rev 1.8   11 Jan 1991 12:44:26   FJM
 *    Rev 1.4   06 Jan 1991 12:45:12   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.3   27 Dec 1990 20:17:00   FJM
 * Changes for porting.
 *
 *    Rev 1.0   22 Dec 1990 13:40:06   FJM
 * Initial revision.
 *
 * -------------------------------------------------------------------- */

/************************************************************************/
/*      doChat()                                                        */
/************************************************************************/
void doChat(moreYet, first)
char moreYet;           /* TRUE to accept following parameters  */
char first;         /* first parameter if TRUE              */
{
    moreYet = moreYet;      /* -W3 */
    first = first;      /* -W3 */

    chatReq = TRUE;

    mPrintf("\bChat\n");
	if (getYesNo(confirm, 0)) {
		trap("Chat request", T_CHAT);
	
		if (cfg.noChat) {
			nextblurb("nochat", &(cfg.cnt.chatnum), 1);
		} else {
			if (whichIO == MODEM)
				ringSysop();
			else {
				chat();
			}
		}
	}
}

/* -------------------------------------------------------------------- */
/*  chat()          This is the chat mode                               */
/* -------------------------------------------------------------------- */
void chat(void)
{
    int c, from, lastfrom, wsize = 0, i;
    char word[50];

    chatkey = FALSE;
    chatReq = FALSE;

    if (!gotCarrier()) {
        dial_out();
        return;
    }
    lastfrom = 2;

    nextblurb("chat", &(cfg.cnt.chatcnt), 1);
    outFlag = IMPERVIOUS;
    /* outFlag = OUTOK;         - changed per Peter's suggestion */

    do {
        c = 0;

        if (KBReady()) {
            c = getch();
            if ((c == -1) || (c == 0xff)) {
                c = 0x1a;
            }
            getkey = 0;
            from = 0;
        }
        if (MIReady()) {
            if (!onConsole) {
                c = getMod();
                if ((c == -1) || (c == 0xff)) {
                    c = 0x1a;
                }
                from = 1;
            } else {
                getMod();
            }
        }
        if (c < 128)
            c = filt_in[c];

        if (c && c != 0x1a /* CNTRLZ */ ) {
            if (from != lastfrom) {
                if (from) {
                    termCap(TERM_NORMAL);
                } else {
                    termCap(TERM_BOLD);
                }
                lastfrom = from;
            }
            if (c == '\r' || c == '\n' || c == ' ' || c == '\t' || wsize == 50) {
                wsize = 0;
            } else if (crtColumn >= (gl_term.termWidth - 1)) {
                if (wsize) {
                    for (i = 0; i < wsize; i++)
                        doBS();
                    doCR();
                    for (i = 0; i < wsize; i++)
                        echocharacter(word[i]);
                } else {
                    doCR();
                }

                wsize = 0;
            } else {
                word[wsize] = c;
                wsize++;
            }

            echocharacter(c);
        }
    } while ((c != 0x1a /* CNTRLZ */ ) && gotCarrier());

    lasttime=cit_time();
    termCap(TERM_NORMAL);

    doCR();
    outFlag = OUTOK;
}

/* -------------------------------------------------------------------- */
/*  ringSysop()     ring the sysop                                      */
/* -------------------------------------------------------------------- */
#define FANCYBELL 1

void ringSysop(void)
{
    char i;
    char answered = FALSE;
    int oldBells;
    char ringlimit = 30;

    /* turn the ringer on */
    oldBells = cfg.noBells;
    cfg.noBells = FALSE;

    mPrintf(" Ringing %s.", cfg.sysop);

    answered = FALSE;
    for (i = 0; (i < ringlimit) && !answered && gotCarrier(); i++) {
#ifdef FANCYBELL
        outMod(7);
        if (!cfg.noBells)   /* retain ablility to turn off with F7  */
            sound(400 - i * 10);
        delay(30);
        if (!cfg.noBells)
            sound(800 + i * 10);
        delay(30);
        nosound();
        delay(1940);
#else
        oChar(7 /* BELL */ );
        delay(800);
#endif
        if (BBSCharReady() || KBReady())
            answered = TRUE;
    }

    cfg.noBells = oldBells;

    if (KBReady()) {
        chat();
    } else if (i >= ringlimit) {
        nextblurb("nosop", &(cfg.cnt.nosop), 1);
    } else
        iChar();
}


