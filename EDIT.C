/* -------------------------------------------------------------------- */
/*  EDIT.C                   Citadel                                    */
/* -------------------------------------------------------------------- */
/*                Message editor and related code.                      */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Includes                                                            */
/* -------------------------------------------------------------------- */
#include <string.h>
#include <io.h>
#include <limits.h>
#include "ctdl.h"
#include "proto.h"
#include "global.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  editText()      handles the end-of-message-entry menu.              */
/*  putheader()     prints header for message entry                     */
/*  getText()       reads a message from the user                       */
/*  matchString()   searches for match to given string.                 */
/*  replaceString() corrects typos in message entry                     */
/*  wordcount()     talleys # lines, word & characters message contains */
/*  ismostlyupper() tests if string is mostly uppercase.                */
/*  fakeFullCase()  converts an uppercase-only message to mixed case.   */
/*  xPutStr()       Put a string to a file w/o trailing blank           */
/* -------------------------------------------------------------------- */


/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/EDIT.C_V  $
 * 
 *    Rev 1.50   01 Nov 1991 11:20:00   FJM
 * Added gl_ structures
 *
 *    Rev 1.49   21 Sep 1991 10:18:46   FJM
 * FredCit release
 *
 *    Rev 1.48   19 Jun 1991 19:03:38   FJM
 * Fix for memory loss in the internal editor.
 *
 *    Rev 1.47   15 Jun 1991  8:37:16   FJM
 *
 *    Rev 1.46   06 Jun 1991  9:18:50   FJM
 *
 *    Rev 1.45   27 May 1991 11:42:30   FJM
 *
 *    Rev 1.44   22 May 1991  2:15:36   FJM
 * Seperated out buffer editing.
 *
 *    Rev 1.43   18 May 1991  9:31:32   FJM
 * Better checkinf for blank apl messages in doenter()
 *
 *    Rev 1.36   28 Feb 1991 12:17:32   FJM
 *    Rev 1.34   10 Feb 1991 17:33:02   FJM
 * Fixes for editor prompt.
 *
 *    Rev 1.31   28 Jan 1991 13:10:46   FJM
 * Made edit prompt inverse.  Made edit/continue impervious.
 *
 *    Rev 1.24   13 Jan 1991  0:30:54   FJM
 * Name overflow fixes.
 *
 *    Rev 1.18   06 Jan 1991 12:45:18   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.17   27 Dec 1990 20:16:44   FJM
 * Changes for porting.
 *
 *    Rev 1.14   22 Dec 1990 13:39:20   FJM
 *    Rev 1.11   16 Dec 1990 19:17:20   FJM
 *    Rev 1.9   16 Dec 1990 18:12:54   FJM
 *    Rev 1.8   09 Dec 1990 15:22:18   FJM
 * Now allows ^Aa-^Ah, ^AA-^AH in messages.
 *
 *    Rev 1.7   07 Dec 1990 23:10:10   FJM
 *    Rev 1.5   24 Nov 1990  3:07:24   FJM
 * Changes for shell/door mode.
 *
 *    Rev 1.4   23 Nov 1990 13:24:48   FJM
 * Header change
 *
 *    Rev 1.2   17 Nov 1990 16:11:40   FJM
 * Added version control log header
 *
 * --------------------------------------------------------------------
 *
 *  EARLY HISTORY:
 *
 *  06/06/89    (PAT)   Made history, cleaned up comments, reformated
 *                      icky code.
 *  06/18/89    (LWE)   Added wordwrap to message entry
 *  03/07/90    {zm}    Added ismostlyupper()
 *  03/12/90    FJM     Added IBM Graphics character translation
 *  03/15/90    {zm}    Add [title] name [surname], 30 characters long.
 *  03/16/90    FJM     Made graphics entry work
 *  03/19/90    FJM     Linted & partial cleanup
 *
 * -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Static Data                                                         */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  editText()      handles the end-of-message-entry menu.              */
/*      return TRUE  to save message to disk,                           */
/*             FALSE to abort message, and                              */
/*             ERROR if user decides to continue                        */
/* -------------------------------------------------------------------- */
int editText(char *buf, int lim)
{
    char ch, x;
    FILE *fd;
    int eom;

    dowhat = PROMPT;

    do {
        outFlag = IMPERVIOUS;
        while (MIReady())   /* flush modem input buffer */
            getMod();
		doCR();
		strcpy(gprompt,"Entry command:");	/* in case of ^A? */
        mtPrintf(TERM_REVERSE,gprompt);
		mPrintf(" ");
        switch (ch = (char) toupper(iChar())) {
            case 'A':
                mPrintf("\bAbort\n ");
                if (strblank(buf))
                    return FALSE;
                else if (getYesNo(confirm, 0)) {
                    heldMessage = TRUE;

                    memcpy(msgBuf2, msgBuf, sizeof(struct msgB));

                    dowhat = DUNO;
                    return FALSE;
                }
                break;
            case 'C':
                mPrintf("\bContinue");
				/* dump message to display */
				outFlag = IMPERVIOUS;
                doCR();
                putheader();
                doCR();
				outFlag = OUTOK;
                mFormat(buf);
                doBS();
                eom = strlen(buf);
                if (eom > 0)
                    buf[eom - 1] = '\0';    /* zap last character ('\n') */
				outFlag = IMPERVIOUS;
                return ERROR;				/* to return to this routine */
            case 'F':
                mPrintf("\bFind & Replace text\n ");
                replaceString(buf, lim, TRUE);
                break;
            case 'P':
                mPrintf("\bPrint formatted\n ");
                doCR();
				outFlag = IMPERVIOUS;
                putheader();
                doCR();
				outFlag = OUTOK;
                mFormat(buf);
                termCap(TERM_NORMAL);
                doCR();
                break;
            case 'R':
                mPrintf("\bReplace text\n ");
                replaceString(buf, lim, FALSE);
                break;
            case 'S':
                mPrintf("\bSave buffer\n ");
                entered++;      /* increment # messages entered */
                dowhat = DUNO;
                return TRUE;
            case 'W':
                mPrintf("\bWord count\n ");
                wordcount(buf);
                break;
            case '?':
                nextmenu("edit", &(cfg.cnt.edittut), 1);
                break;
            default:
                if ((x = (char) strpos((char) tolower(ch), editcmd)) != 0) {
                    x--;
                    mPrintf("\b%s", edit[x].ed_name);
                    doCR();
                    if (edit[x].ed_local && !onConsole) {
                        mPrintf("\n Local editor only!\n ");
                    } else {
                        changedir(cfg.aplpath);
                        if ((fd = fopen("message.apl", "wb")) != NULL) {
                            xPutStr(fd, msgBuf->mbtext);
                            fclose(fd);
                        }
                        readMessage = FALSE;
                        extFmtRun(edit[x].ed_cmd, "message.apl");
                        changedir(cfg.aplpath);
                        if ((fd = fopen("message.apl", "rb")) != NULL) {
                            GetStr(fd, msgBuf->mbtext, cfg.maxtext);
                            fclose(fd);
                            unlink("message.apl");
                        }
                    }
                    break;
                }
                if (!gl_user.expert)
                    nextmenu("edit", &(cfg.cnt.edittut), 1);
                else
                    mPrintf("\n '?' for menu.\n \n");
                break;
        }
    } while (!ExitToMsdos && (haveCarrier || onConsole));
    dowhat = DUNO;
    return FALSE;
}

/* -------------------------------------------------------------------- */
/*  putheader()     prints header for message entry                     */
/* -------------------------------------------------------------------- */
void putheader(void)
{
    char dtstr[80];

    cit_strftime(dtstr, 79, cfg.datestamp, 0l);

    termCap(TERM_BOLD);
    mPrintf("    %s", dtstr);
    if (loggedIn) {
        if (msgBuf->mbtitle[0]) {
            mPrintf(" From [%s] %s", msgBuf->mbtitle, msgBuf->mbauth);
        } else {
            mPrintf(" From %s", msgBuf->mbauth);
        }
        if (msgBuf->mbsur[0]) {
            mPrintf(" [%s]", msgBuf->mbsur, msgBuf->mbauth);
        }
    }
    if (msgBuf->mbto[0])
        mPrintf(" To %s", msgBuf->mbto);
    if (msgBuf->mbzip[0])
        mPrintf(" @ %s", msgBuf->mbzip);
    if (msgBuf->mbrzip[0])
        mPrintf(", %s", msgBuf->mbrzip);
    if (msgBuf->mbczip[0])
        mPrintf(", %s", msgBuf->mbczip);
    if (msgBuf->mbfwd[0])
        mPrintf(" Forwarded to %s", msgBuf->mbfwd);
    if (msgBuf->mbgroup[0])
        mPrintf(" (%s Only)", msgBuf->mbgroup);
    termCap(TERM_NORMAL);
}

/* --------------------------------------------------------------------
 *  fedit()			Internal file editor
 *                  Returns TRUE if user saves the file, else FALSE
 *					No protection for bad file names, check first!
 *					Will not save 'blank' files
 * -------------------------------------------------------------------- */

BOOL fedit(char *filename)
{
	FILE *fp;
	char *buf;
	unsigned int lim;			/* can't edit more then a segment anyway */
	long filesize;
	int rc = FALSE;
	int err;
	char path[64];
	
	getcwd(path,64);

	err = errno = 0;
	/* set the message entry flags and buffer */
	clearmsgbuf();
    setmem(msgBuf, sizeof(struct msgB), 0);
    mailFlag = FALSE;
    oldFlag = FALSE;
    limitFlag = FALSE;
    linkMess = FALSE;
	if (loggedIn)
		strcpy(msgBuf->mbauth,logBuf.lbname);
	
	/* try to edit an existing file */
	if (filexists(filename)) {
		fp = fopen(filename,"rt");
		if (fp) {
			filesize = filelength(fileno(fp));
			if ((unsigned int) (filesize + (long) cfg.maxtext) > UINT_MAX) {
				err = -2;
				cPrintf("\nFile too long\n");
			} else {
				lim = cfg.maxtext + (int) filesize;
				buf = calloc(1,lim);
				if (buf) {
					errno = 0;
					if (!fread(buf,1,lim,fp)) {
						cPrintf("\nFile read error\n");
						err = errno;
					}
					if (fclose(fp) && !err) {/* we need to 'recreate' later */
						cPrintf("\nFile close error\n");
						err = errno;
					}
					if (!err)
						mFormat(buf);		/* display the file */
					oldFlag = TRUE;
					
				}
			}
		} else {
			lim = cfg.maxtext;
			buf = calloc(1,lim);
			oldFlag = FALSE;
		}
	}
	
	if (buf && !err && editBuf(buf,lim)) {
		changedir(path);		/* because silly edit routines loose path */
		if (!strblank(buf)) {
			rc = TRUE;
			lim = strlen(buf);
			errno = 0;			/* because silly video routines set it */
			fp = fopen(filename,"wt");
			if (!fp && !err) {
				cPrintf("\nFile open error 2\n");
				err = errno;
			}
			if (fp) {
				/* because silly DOS doesn't 'error' on disk full	*/
				if(!fwrite(buf,1,lim,fp) && !errno) {
					cPrintf("\nFile write error\n");
					err = -1;
				}
				errno = 0;
				if (!fclose(fp)&& !err) {
					cPrintf("\nFile close error 2\n");
					err = errno;
				}
			}
		} else {				/* we won't save empty files */
			rc = FALSE;
		}
	}
	if (!buf) {					/* out of memory condition */
		mtPrintf(TERM_BOLD," \n Out of memory editing file %s.\n",filename);
		rc = FALSE;
	} else {
		switch (err) {
			case 0:		/* no error */
			break;
			
			case -1:	/* disk full condition */
				mtPrintf(TERM_BOLD,
					" \n Disk full saving %s, file truncated.\n",filename);
				rc = FALSE;
			break;
			
			case -2:	/* file >64k condition */
				mtPrintf(TERM_BOLD,
					" \n File %s is too large to edit.\n",filename);
				rc = FALSE;
			break;
			
			default:	/* DOS error condition */
				mtPrintf(TERM_BOLD, " \n DOS error '%s' editing %s: \n",
					strerror(err),filename);
				rc = FALSE;
			break;
		}
	}
	if (buf) {
		free(buf);
		buf = NULL;
	}
	return rc;
}

/* --------------------------------------------------------------------
 *  editBuf()       let user edit a buffer
 *                  Returns TRUE if user decides to save it, else FALSE
 * -------------------------------------------------------------------- */

BOOL editBuf (char *buf, int lim)
{
#define MAXEWORD 50
	int i;
	unsigned int c = 0;
    int done = FALSE;
	char beeped = FALSE;
	int wsize = 0;
    unsigned int lastCh;
    int toReturn;
    unsigned char word[MAXEWORD];
	
    if (!oldFlag) {
		*buf='\0';
    }

	i = strlen(buf);
	/* kill trailing CR's */
    while (i && buf[i - 1] == '\n') {
        buf[i - 1] = 0;
		--i;
	}
    mFormat(buf);
	
    do {
        i = strlen(buf);
        if (i)
            c = buf[i];
        else
            c = '\n';

        while (!done && i < lim && (!ExitToMsdos && (haveCarrier || onConsole))) {
            if (i)
                lastCh = c;

            c = iChar();

            switch (c) {    /* Analize what they typed       */
                case 1:     /* CTRL-A>nsi   */
				{
					unsigned int temp, d;
					
                    temp = echo;
                    echo = NEITHER;
                    d = (char) iChar();
                    echo = temp;

                    if (d >= '0' && toupper(d) <= 'H' && gl_term.ansiOn) {
                        if (d == '?') { /* ansi help */
                            mPrintf("ansi.hlp");
                            nexthelp("ansi", &(cfg.cnt.ansitut), 1);
                        } else {
                            termCap(d);
                            buf[i++] = 0x01;
                            buf[i++] = d;
                        }
                    } else {
                        oChar(7);
                    }
                    break;
				}
                case '\n':      /* NEWLINE      */
                    if ((lastCh == '\n') || (i == 0))
                        done = TRUE;
                    if (!done)
                        buf[i++] = '\n';  /* A CR might be nice   */
                    break;
                case 27:        /* An ESC? No, No, NO!  */
                    oChar('\a');
                    break;
                case 0x1a:      /* CTRL-Z       */
                    done = TRUE;
                    entered++;  /* increment # messages entered */
                    break;
                case '\b':      /* CTRL-H bkspc */
                    if ((i > 0) && (buf[i - 1] == '\t')) {  /* TAB  */
                        i--;
                        while ((crtColumn % 8) != 1)
                            doBS();
                    } else if ((i > 0) && (buf[i - 1] != '\n')) {
                        i--;
                        if (wsize > 0)
                            wsize--;
                    } else {
                        oChar(32);
                        oChar('\a');
                    }
                    break;
                default:        /* '\r' and '\n' never get here */
                    if ((c == ' ') || (c == '\t') || (wsize == MAXEWORD)) {
                        wsize = 0;
                    } else if (crtColumn >= (gl_term.termWidth - 1)) {
                        if (wsize) {
							int j;
						
                            for (j = 0; j < (wsize + 1); j++)
                                doBS();
                            doCR();
                            for (j = 0; j < wsize; j++)
                                echocharacter(word[j]);
                            echocharacter(c);
                        } else {
                            doBS();
                            doCR();
                            echocharacter(c);
                        }
                        wsize = 0;
                    } else {
                        word[wsize] = c;
                        wsize++;
                    }

                    if (c != 0)
                        buf[i++] = c;

                    if (i > cfg.maxtext - 80 && !beeped) {
                        beeped = TRUE;
                        oChar('\a');
                    }
                    break;
            }

            buf[i] = '\0';  /* null to terminate message */
            if (i)
                lastCh = buf[i - 1];

            if (i == lim)
                mPrintf(" Buffer overflow.\n ");
        }
        done = FALSE;       /* In case they Continue */

        termCap(TERM_NORMAL);

        if (c == 0x1a && i != lim) {    /* if was CTRL-Z */
            buf[i++] = '\n';    /* end with NEWLINE */
            buf[i] = '\0';
            toReturn = TRUE;    /* yes, save it */
            doCR();
            mPrintf(" Saving message");
            doCR();
        } else          /* done or lost carrier */
            toReturn = editText(buf, lim);

    } while ((toReturn == ERROR) && (!ExitToMsdos && (haveCarrier || onConsole)));
    /* ERROR returned from editor means continue    */

    if (toReturn == TRUE) { /* Filter null messages */
        toReturn = FALSE;
        for (i = 0; buf[i] != 0 && !toReturn; i++)
            toReturn = (buf[i] > ' ' && buf[i] < 127);
    }
	return (BOOL) toReturn;
}

/* -------------------------------------------------------------------- */
/*  getText()       reads a message from the user                       */
/*                  Returns TRUE if user decides to save it, else FALSE */
/* -------------------------------------------------------------------- */
BOOL getText(void)
{
    char *buf;

    if (!gl_user.expert) {
        nextblurb("entry", &(cfg.cnt.ecount), 1);
        outFlag = OUTOK;
        doCR();
        mPrintf(" You may enter up to %d characters.", cfg.maxtext);
        mPrintf("\n Please enter message.  Use an empty line to end.");
    }
	
    outFlag = IMPERVIOUS;
    doCR();
    putheader();
    doCR();
	outFlag = OUTOK;

    buf = msgBuf->mbtext;

	/* do text entry */
	
    return (BOOL) editBuf(buf,cfg.maxtext-1);
}

/* -------------------------------------------------------------------- */
/*  matchString()   searches for match to given string.                 */
/*                  Runs backward  through buffer so we get most recent */
/*                  error first. Returns loc of match, else ERROR       */
/* -------------------------------------------------------------------- */
char *matchString(char *buf, char *pattern, char *bufEnd, char ver)
{
    char *loc, *pc1, *pc2;
    char subbuf[11];
    char foundIt;

	subbuf[10] = '\0';
    for (loc = bufEnd, foundIt = FALSE; !foundIt && --loc >= buf;) {
        for (pc1 = pattern, pc2 = loc, foundIt = TRUE; *pc1 && foundIt;) {
            if (!(tolower(*pc1++) == tolower(*pc2++)))
                foundIt = FALSE;
        }
        if (ver && foundIt) {
            doCR();
            strncpy(subbuf,
            buf + 10 > loc ? buf : loc - 10,
            (unsigned) (loc - buf) > 10 ? 10 : (unsigned) (loc - buf));
            subbuf[(unsigned) (loc - buf) > 10 ? 10 : (unsigned) (loc - buf)] = 0;
            mPrintf("%s", subbuf);
            if (gl_term.ansiOn)
                termCap(TERM_BOLD);
            else
                mPrintf(">");
            mPrintf("%s", pattern);
            if (gl_term.ansiOn)
                termCap(TERM_NORMAL);
            else
                mPrintf("<");
            strncpy(subbuf, loc + strlen(pattern), 10);
            subbuf[10] = 0;
            mPrintf("%s", subbuf);
            if (!getYesNo("Replace", 0))
                foundIt = FALSE;
        }
    }
    return foundIt ? loc : NULL;
}

/* -------------------------------------------------------------------- */
/*  replaceString() corrects typos in message entry                     */
/* -------------------------------------------------------------------- */
void replaceString(char *buf, int lim, char ver)
{
    char oldString[256];
    char newString[256];
    char *loc, *textEnd;
    char *pc;
    int incr, length;

    /* find terminal null */
    for (textEnd = buf, length = 0; *textEnd; length++, textEnd++);

    getString("text", oldString, 256, FALSE, ECHO, "");
    if (!*oldString) {
        mPrintf(" Text not found.\n");
        return;
    }
    if ((loc = matchString(buf, oldString, textEnd, ver)) == NULL) {
        mPrintf(" Text not found.\n ");
        return;
    }
    getString("replacement text", newString, 256, FALSE, ECHO, "");
    if (strlen(newString) > strlen(oldString)
    && ((strlen(newString) - strlen(oldString)) >= lim - length)) {
        mPrintf(" Buffer overflow.\n ");    /* FJM: should be trapped ??? */
        return;         /* nope, it's a user error.   */
    }
    /* delete old string: */
    for (pc = loc, incr = strlen(oldString); (*pc = *(pc + incr)) != 0; pc++)
        ;
    textEnd -= incr;

    /* make room for new string: */
    for (pc = textEnd, incr = strlen(newString); pc >= loc; pc--) {
        *(pc + incr) = *pc;
    }

    /* insert new string: */
    for (pc = newString; *pc; *loc++ = *pc++)
        ;
}

/* -------------------------------------------------------------------- */
/*  wordcount()     talleys # lines, word & characters message contains */
/* -------------------------------------------------------------------- */
void wordcount(char *buf)
{
    char *counter;
    int lines = 0, words = 0, chars;

    counter = buf;

    chars = strlen(buf);

    while (*counter++) {
        if (*counter == ' ') {
            if ((*(counter - 1) != ' ') && (*(counter - 1) != '\n'))
                words++;
        }
        if (*counter == '\n') {
            if ((*(counter - 1) != ' ') && (*(counter - 1) != '\n'))
                words++;
            lines++;
        }
    }
    mPrintf(" %d lines, %d words, %d characters\n ", lines, words, chars);
}

/* -------------------------------------------------------------------- */
/*  ismostlyupper() -- tests to see if string has n % uppercase letters */
/*  returns true if it thinks so...                                     */
/* -------------------------------------------------------------------- */
int ismostlyupper(char *s, int n)
/* s = string to inspect,  n = percentage value */
{
    int nupper, nlower;
    long percent_upper;
    char *cp;

    nupper = nlower = 0;
    for (cp = s; *cp; cp++) {
        if (isupper(*cp))
            nupper++;
        if (islower(*cp))
            nlower++;
    }

    if (nupper == 0)
        return FALSE;

    percent_upper = (((long) nupper) * 100L) / ((long) nupper + (long) nlower);
    return (percent_upper >= (long) n);
}

/* -------------------------------------------------------------------- */
/*  fakeFullCase()  converts a message in uppercase-only to a           */
/*      reasonable mix.  It can't possibly make matters worse...        */
/*      Algorithm: First alphabetic after a period is uppercase, all    */
/*      others are lowercase, excepting pronoun "I" is a special case.  */
/*      We assume an imaginary period preceding the text.               */
/* -------------------------------------------------------------------- */
void fakeFullCase(char *text)
{
    char *c;
    char lastWasPeriod;
    char state;

    for (lastWasPeriod = TRUE, c = text; *c; c++) {
        if ((*c != '.') && (*c != '?') && (*c != '!')) {
            if (isalpha(*c)) {
                if (lastWasPeriod)
                    *c = (char) toupper(*c);
                else
                    *c = (char) tolower(*c);
                lastWasPeriod = FALSE;
            }
        } else {
            lastWasPeriod = TRUE;
        }
    }

    /* little state machine to search for ' i ': */
#define NUTHIN          0
#define FIRSTBLANK      1
#define BLANKI          2
    for (state = NUTHIN, c = text; *c; c++) {
        switch (state) {
            case NUTHIN:
                if (isspace(*c))
                    state = FIRSTBLANK;
                else
                    state = NUTHIN;
                break;
            case FIRSTBLANK:
                if (*c == 'i')
                    state = BLANKI;
                else
                    state = NUTHIN;
                break;
            case BLANKI:
                if (isspace(*c))
                    state = FIRSTBLANK;
                else
                    state = NUTHIN;

                if (!isalpha(*c))
                    *(c - 1) = 'I';
                break;
        }
    }
}

/* -------------------------------------------------------------------- */
/*  xPutStr()       Put a string to a file w/o trailing blank           */
/* -------------------------------------------------------------------- */
void xPutStr(FILE * fl, char *str)
{
    while (*str) {
        fputc(*str, fl);
        str++;
    }
}
/* EOF */
