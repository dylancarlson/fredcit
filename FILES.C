/************************************************************************
 * Files.c
 * File handling routines for ctdl
 ************************************************************************/

#include <string.h>
#include <time.h>
#include "ctdl.h"

#ifndef ATARI_ST
#include <dos.h>
#include <alloc.h>
#include <io.h>
#endif

#include "proto.h"
#include "global.h"

/************************************************************************
 *
 *                              CONTENTS
 *
 *      ambig()                 returns true if filename is ambiguous
 *      ambigUnlink()           unlinks ambiguous filenames
 *      attributes()            aide fn to set file attributes
 *      blocks()                displays how many blocks file is
 *      checkfilename()         returns ERROR on illegal filenames
 *      checkup()               returns TRUE if filename can be uploaded
 *      dir()                   very high level, displays directory
 *      download()              menu level download routine
 *      dltime()                computes dl time from size & global rate
 *      dump()                  does Unformatted dump of specified file
 *      dumpf()                 does Formatted dump of specified file
 *      entertextfile()         menu level .et
 *      enterwc()               menu level .ew file
 *      entrycopy()             readable struct -> directory array
 *      entrymake()             dos transfer struct -> readable struct
 *      filexists()             returns TRUE if a specified file exists
 *      filldirectory()         fills our directory structure
 *      getfirst()              low level, read first item of directory
 *      getnext()               low level, read next item of directory
 *      hello()                 prints random hello blurbs
 *      hide()                  hides a file. for limited-access u-load
 *      nextblurb()             show next .BLB or .BL@ file
 *      readdirectory()         menu level .rd .rvd routine
 *      readinfofile()          menu level .ri .rvi routine
 *      readtextfile()          menu level .rt routine
 *      readwc()                menu level .rw file
 *      renamefile()            aide fn to rename a file
 *      strlwr()                makes any string lower case
 *      textdown()              does wildcarded unformatted file dumps
 *      textup()                handles actual text upload
 *      tutorial()              handles wildcarded helpfile dumps
 *      upload()                menu level file upload routine
 *      unlinkfile()            handles the .au command
 *      wcdown()                calls xmodem downloading routines
 *      wcup()                  calls xmodem uploading routines
 *
 ************************************************************************/


/*
 * --------------------------------------------------------------------
 * Version control log: $Log:   D:/VCS/FCIT/FILES.C_V  $
 * 
 *    Rev 1.49   01 Nov 1991 11:20:02   FJM
 * Added gl_ structures
 *
 *    Rev 1.48   21 Sep 1991 10:18:48   FJM
 * FredCit release
 *
 *    Rev 1.47   08 Jul 1991 16:18:52   FJM
 *
 *    Rev 1.46   06 Jun 1991  9:18:54   FJM
 *
 *    Rev 1.45   27 May 1991 11:42:34   FJM
 *
 *    Rev 1.44   22 May 1991  2:15:48   FJM
 * Added description (.DES) files.
 *
 *    Rev 1.34   05 Feb 1991 14:31:12   FJM
 *    Rev 1.33   28 Jan 1991 13:11:10   FJM
 * Added batchinfo(FALSE) to batch uploads.
 * Made reset file info not reset files that exist.
 *
 *    Rev 1.26   13 Jan 1991  0:31:04   FJM
 * Name overflow fixes.
 *    Rev 1.20   06 Jan 1991 12:44:44   FJM
 * Porting, formating, fixes and misc changes.
 *
 * Rev 1.19   27 Dec 1990 20:16:40   FJM Changes for porting.
 * Rev 1.16   22 Dec 1990 13:39:12   FJM
 * Rev 1.9   07 Dec 1990 23:10:12   FJM
 * Rev 1.8   02 Dec 1990  0:54:16   FJM
 * Rev 1.7   01 Dec 1990  2:54:42   FJM Fixed bugs related to non-existant room
 * directories defaulting to the home path.
 *
 * Rev 1.6   23 Nov 1990 18:24:24   FJM Added countdown() for autologoff.
 *
 * Rev 1.5   23 Nov 1990 13:24:50   FJM Header change
 *
 * Rev 1.3   18 Nov 1990 19:25:48   FJM Cleaned up filldirectory(), fillinfo().
 * Fixed error of one in fillinfo() and buffer mis-allocation in
 * readinfofile().
 *
 * Rev 1.2   17 Nov 1990 16:11:34   FJM Added version control log header
 * --------------------------------------------------------------------
 * Early History:
 *
 * 03/15/90    {zm}    Use FILESIZE (20) instead of NAMESIZE.
 * 03/17/90    FJM     added checks for farfree()
 * 04/14/90    FJM     Changed checkfilename(), renamefile() to use FILESIZE
 * 04/16/90    FJM     Changed checkfilename(), renamefile() to use
 *                     FILESIZE+1 for storage.
 * 04/20/90    FJM     Created nextblurb().
 * 06/28/90    FJM     Added CLOCK$, COM3, COM4 as devices.
 * 07/08/90    FJM     Made nextblurb() a bit smarter.
 * 08/07/90    FJM     Made textup,wcup blurbs rotate (why not?).
 * 09/20/90    FJM     Corrected bug that always set screen height to 0
 *                     in goodbye()!
 * 10/15/90    FJM     Added console only protocol flag (field #1)
 * 11/04/90    FJM     Added code to terminate after up/download.
 *
 * -------------------------------------------------------------------- */

/* --------------------------------------------------------------------
 * local functions
 * -------------------------------------------------------------------- */
static int countdown(char *prompt, long timeout);


/* --------------------------------------------------------------------
 * local defs
 * -------------------------------------------------------------------- */
#define MAXWORD 256

/*
 * --------------------------------------------------------------------
 * local structs
 * -------------------------------------------------------------------- */

/* our readable transfer structure */
static struct {
    char            filename[13];
    unsigned char   attribute;
    char            date[9];
    char            time[9];
    long            size;
}               directory;

/* --------------------------------------------------------------------
 * local data
 * --------------------------------------------------------------------*/
static char    *devices[] = {
    "CON", "AUX", "COM1", "COM2", "COM3", "COM4", "LPT1", "PRN", "LPT2",
    "LPT3", "NUL", NULL
};

static char    *badfiles[] ={
    "LOG.DAT", "MSG.DAT", "GRP.DAT", "CONFIG.CIT", "HALL.DAT", "ROOM.DAT",
    "FILEINFO.CIT", "CLOCK$", NULL
};



/************************************************************************/
/* ambig() returns TRUE if string is an ambiguous filename         		*/
/************************************************************************/
int
ambig(char *filename)
{
    int	rc = FALSE;

    if (strchr(filename,'*') || strchr(filename,'?')) {
       rc = TRUE;
	}
    return rc;
}

/************************************************************************/
/* ambigUnlink() unlinks ambiguous filenames                       		*/
/************************************************************************/
int
ambigUnlink(char *filename, char change)
{
    char            file[FILESIZE];
    int             i = 0;

    if (change)
        if (changedir(roomBuf.rbdirname) == -1) {
            return 0;
        }
    filldirectory(filename, TRUE);

    while (filedir[i].entry[0]) {
        filedir[i].entry[13] = ' ';
        sscanf(filedir[i].entry, "%s ", file);
        if (file[0])
            unlink(file);
        i++;
    }

    /* free file directory structure */
    if (filedir != NULL) {
        if (filedir)
            free((void *) filedir);
        filedir = NULL;
    }
    return (i);
}

/************************************************************************/
/* attributes() aide fn to set file attributes                     		*/
/************************************************************************/
void
attributes(void)
{
    label           filename;
    char            hidden = 0, readonly = 0;
    unsigned char   attr, getattr();

    doCR();
    getNormStr("filename", filename, FILESIZE, ECHO);

    if (changedir(roomBuf.rbdirname) == -1) {
        return;
    }
    if ((checkfilename(filename, 0) == ERROR)
    || ambig(filename)) {
        mPrintf("\n Invalid filename.");
        changedir(cfg.homepath);
        return;
    }
    if (!filexists(filename)) {
        mPrintf(" File not found\n");
        changedir(cfg.homepath);
        return;
    }
    attr = getattr(filename);

    readonly = attr & 1;
    hidden = ((attr & 2) == 2);

    /* set readonly and hidden bits to zero */
    attr = (attr ^ readonly);
    attr = (attr ^ (hidden * 2));

    readonly = getYesNo("Read only", readonly);
    hidden = getYesNo("Hidden", hidden);

    /* set readonly and hidden bits */
    attr = (attr | readonly);
    attr = (attr | (hidden * 2));

    setattr(filename, attr);

    sprintf(msgBuf->mbtext,
    "Attributes of file %s changed in %s] by %s",
    filename,
    roomBuf.rbname,
    logBuf.lbname);

    trap(msgBuf->mbtext, T_AIDE);

    aideMessage();

    changedir(cfg.homepath);
}

/************************************************************************/
/* blocks()  displays how many blocks file is upon download        		*/
/************************************************************************/
void
blocks(char *filename, int bsize)
{
    FILE           *stream;
    long            length;
    int             blks;

    double          dltime();

    outFlag = OUTOK;

    stream = fopen(filename, "r");
	if (stream) {
		length = filelength(fileno(stream));
		fclose(stream);

		if (length == -1l)
			return;
	
		if (bsize)
			blks = ((int) (length / (long) bsize) + 1);
		else
			blks = 0;
	
		doCR();
	
		if (!bsize)
			mPrintf("File Size: %ld %s",
			length, (length == 1l) ? "byte" : "bytes");
		else
			mPrintf("File Size: %d %s, %ld %s",
			blks, (blks == 1) ? "block" : "blocks",
			length, (length == 1l) ? "byte" : "bytes");
	
		doCR();
		mPrintf("Transfer Time: %.0f minutes", dltime(length));
		doCR();
	}
}

/************************************************************************/
/* checkfilename() checks a filename for illegal characters        		*/
/************************************************************************/
int checkfilename(char *filename, char xtype)
{
    char *s, *s2;
    char i;
    char device[6];
    char invalid[] = "'\"/\\[]:|&^<>+=;,";

    for (s = filename; *s; ++s)
        for (s2 = invalid; *s2; ++s2)
            if ((*s == *s2) || *s < ' ' ||
				(*s==' ' && !extrn[xtype-1].ex_batch))
                return (ERROR);

	/* FJM: There was no check for terminating null in file name here before!*/
    for (i = 0; i <= 4 && filename[i] != '.' && filename[i]; ++i) {
        device[i] = filename[i];
    }
    device[i] = '\0';

    for (i = 0; devices[i] != NULL; ++i)
        if (strcmpi(device, devices[i]) == SAMESTRING)
            return (ERROR);

    for (i = 0; badfiles[i] != NULL; ++i)
        if (strcmpi(filename, badfiles[i]) == SAMESTRING)
            return (ERROR);

    return (TRUE);
}

/************************************************************************/
/* checkup()  returns TRUE if filename can be uploaded            		*/
/************************************************************************/
int
checkup(char *filename)
{
    if (ambig(filename))
        return (ERROR);

    /* no bad files */
    if (checkfilename(filename, 0) == ERROR) {
        mPrintf("\n Invalid filename.");
        return (ERROR);
    }
    if (changedir(roomBuf.rbdirname) == -1) {
        return FALSE;
    }
    if (filexists(filename)) {
        mPrintf("\n File exists.");
        changedir(cfg.homepath);
        return (ERROR);
    }
    return (TRUE);
}

/************************************************************************/
/* filldirectory()  this routine will fill the directory structure  	*/
/* according to wildcard                                            	*/
/************************************************************************/
void
filldirectory(char *filename, char verbose)
{
    int             i, rc;
    struct ffblk    file_buf;
    int             strip;

    /*
     * allocate the first record of the file dir structure
     *
     * Since we null terminate, really only cfg.maxfiles-1 files allowed.
     *
     */

    filedir = farcalloc((long) (sizeof(struct fDir)), cfg.maxfiles);
    /* return on error allocating */
    if (!filedir) {
        cPrintf("Failed to allocate memory for FILEDIR\n");
        return;
    }
    /* keep going till it errors, which is end of directory */
    for (rc = findfirst("*.*", &file_buf, (gl_user.aide ? FA_HIDDEN : 0)), i = 0;
    !rc; rc = findnext(&file_buf), ++i) {
    /* Only cfg.maxfiles # of files files (was error of one here) */
        if (i >= cfg.maxfiles - 1) {
            amPrintf(" \n More then %d files in room %s. \n",
				cfg.maxfiles, roomBuf.rbname);
			SaveAideMess();
			mtPrintf(TERM_BOLD,"\n Warning: more then %d files.\n\n",
				cfg.maxfiles);
            break;
		}

    /* translate dos's structure to something we can read */
        entrymake(&file_buf);

        if (!strpos('.', directory.filename)) {
            strcat(directory.filename, ".");
            strip = TRUE;
        } else {
            strip = FALSE;
        }

    /*
     * copy "directory" to "filedir" NO zero length filenames
     *
     * Probably should check against 'badfiles' here.
     *
     */

        if ((!(directory.attribute &
			(FA_HIDDEN | FA_SYSTEM | FA_DIREC | FA_LABEL)))
			/* either aide or not a hidden file */
			/* && (gl_user.aide || !(directory.attribute & FA_HIDDEN) ) */
		
			/* filename match wildcard? */
            && (u_match(directory.filename, filename))
			/* never display fileinfo.cit */
			&& (strcmpi(directory.filename, "fileinfo.cit") != SAMESTRING)) {

			/* if passed, put into structure, else loop again */
            if (strip)
                directory.filename[strlen(directory.filename) - 1] = '\0';
            entrycopy(i, verbose);
        } else {
            i--;
        }

    }
    filedir[i].entry[0] = '\0'; /* tie it off with a null */

    /* alphabetical order */
    qsort(filedir, i, sizeof(struct fDir), strcmp);
}

/***********************************************************************/
/* dir() highest level file directory display function                 */
/***********************************************************************/
void dir(char *filename, char verbose)
{
    int     i;
	int		printed=0;
    long    bytesfree();

    outFlag = OUTOK;

    /* no bad files */
    /*
     * if (checkfilename(filename, 0) == ERROR) { mPrintf("\n No file %s",
     * filename); return; }
     */

    if (changedir(roomBuf.rbdirname) == -1) {
        return;
    }
    /* load our directory structure according to filename */
    filldirectory(filename, verbose);

    for (i = 0;
        (filedir[i].entry[0] && (outFlag != OUTSKIP) && !mAbort());
		++i) {
        if (verbose) {
            filedir[i].entry[0] = filedir[i].entry[13];
            filedir[i].entry[13] = ' ';
            filedir[i].entry[40] = '\0';    /* cut timestamp off */
            doCR();
        }
		/* display filename */
		if (checkfilename(filename, 0) != ERROR) {
			if (!printed) {
				if (verbose) {
					mtPrintf(TERM_BOLD,
						"\n Filename     Date      Size   D/L Time\n");
				} else {
					doCR();
				}
			}
			mPrintf(filedir[i].entry);
			++printed;
		}
    }

    /* check for matches */
    if (!printed && outFlag != OUTSKIP)
        mPrintf("\n No file %s", filename);
	
    if (verbose && outFlag != OUTSKIP) {
        doCR();
        mtPrintf(TERM_BOLD, "        %d %s    %ld bytes free", printed,
        (printed == 1) ? "File" : "Files", bytesfree());
    }
    /* free file directory structure */
    if (filedir)
        farfree((void *) filedir);
    filedir = NULL;

    /* go to our home-path */
    changedir(cfg.homepath);
}

/************************************************************************/
/* dltime()  give this routine the size of your file and it will    	*/
/* return the amount of time it will take to download according to    	*/
/* speed                                                              	*/
/************************************************************************/
double dltime(long size)
{
    double          dl_time;
    static long     fudge_factor[] = {
		/* 300 1200   2400    4800    9600   19.2k   38.4k */
		1800L, 7200L, 14400L, 28800L, 57600L,115200, 230400
	};
    /* could be more accurate */

    dl_time = (double) size / (double) (fudge_factor[speed]);

    return (dl_time);
}



/************************************************************************/
/* dump()  does unformatted dump of specified file                 		*/
/* returns ERROR if carrier is lost, or file is aborted            		*/
/************************************************************************/
int dump(char *filename)
{
    FILE           *fbuf;
    int             c, returnval = TRUE;

    /* last itteration might have been N>exted */
    outFlag = OUTOK;
    doCR();

	// was mode "r"
    if ((fbuf = fopen(filename, "rt")) == NULL) {
        mPrintf("\n No file %s", filename);
        return (ERROR);
    }
	
//    /* looks like a kludge, but we need speed!! */
//
//    while ((c = getc(fbuf)) != ERROR && (c != 0x1a /* CPMEOF */ )
//    && (outFlag != OUTNEXT) && (outFlag != OUTSKIP) && !mAbort()) {
//        if (c == '\n')
//            doCR();
//        else
//            oChar(c);
//    }

    while ((c = getc(fbuf)) != EOF) {
		if ((MIReady() || KBReady()) && mAbort())
			break;
        if (c == '\n')
            doCR();
        else
            oChar(c);
    }

    if (outFlag == OUTSKIP)
        returnval = ERROR;

    fclose(fbuf);

    return returnval;
}

/************************************************************************/
/* dumpf()  does formatted dump of specified file                  		*/
/* returns ERROR if carrier is lost, or file is aborted            		*/
/************************************************************************/
int
dumpf(char *filename)
{
    FILE           *fbuf;
    char            line[MAXWORD];
    int             returnval = TRUE;

    /* last itteration might have been N>exted */
    outFlag = OUTOK;
    doCR();

    if ((fbuf = fopen(filename, "r")) == NULL) {
        mPrintf("\n No helpfile %s", filename);
        return (ERROR);
    }
    /* looks like a kludge, but we need speed!! */

    while (fgets(line, MAXWORD, fbuf) && (outFlag != OUTNEXT)
    && (outFlag != OUTSKIP) && !mAbort()) {
        mFormat(line);
    }
    if (outFlag == OUTSKIP)
        returnval = ERROR;

    fclose(fbuf);

    return returnval;
}

/************************************************************************/
/* entertextfile()  menu level .et                                    	*/
/************************************************************************/
void
entertextfile(void)
{
    label           filename;
    char            comments[64];

    doCR();
    getNormStr("filename", filename, FILESIZE, ECHO);

    if (checkup(filename) == ERROR)
        return;

    getString("comments", comments, 64, FALSE, TRUE, "");

    if (strlen(filename))
        textup(filename);

    if (changedir(roomBuf.rbdirname) == -1) {
        return;
    }
    if (filexists(filename)) {
        entercomment(filename, logBuf.lbname, comments);
        if (!comments[0])
            sprintf(msgBuf->mbtext, " %s uploaded by %s", filename, logBuf.lbname);
        else
            sprintf(msgBuf->mbtext, " %s uploaded by %s\n Comments: %s",
            filename, logBuf.lbname, comments);
        specialMessage();
    }
    changedir(cfg.homepath);
}

/* WC isn't needed on DOS machines which use DSZ or equiv. */
#ifndef __MSDOS__

/************************************************************************/
/* enterwc()  menu level .ew  HIGH level routine                   		*/
/************************************************************************/
void
enterwc(void)
{
    label           filename;
    char            comments[64];

    doCR();
    getNormStr("filename", filename, FILESIZE, ECHO);

    if (checkup(filename) == ERROR)
        return;

    getString("comments", comments, 64, FALSE, TRUE, "");

    if (strlen(filename))
        wcup(filename);

    if (changedir(roomBuf.rbdirname) == -1) {
        return;
    }
    if (filexists(filename)) {
        entercomment(filename, logBuf.lbname, comments);
        if (!comments[0])
            sprintf(msgBuf->mbtext, " %s uploaded by %s", filename, logBuf.lbname);
        else
            sprintf(msgBuf->mbtext, " %s uploaded by %s\n Comments: %s",
            filename, logBuf.lbname, comments);
        specialMessage();
    }
    changedir(cfg.homepath);
}

#endif

/************************************************************************/
/* entrycopy()                                                        	*/
/* This routine copies the single readable "directory" array to the   	*/
/* to the specified element of the "dir" array according to verbose.  	*/
/************************************************************************/
void
entrycopy(int element, char verbose)
{
    double          dltime();

    if (verbose) {
    /* need error checks here! */
        sprintf(filedir[element].entry, " %-12s %s %7ld %9.2f %s ",
        directory.filename,
        directory.date,
        directory.size,
        dltime(directory.size),
        directory.time
        );

        if ((directory.attribute & FA_HIDDEN))
            filedir[element].entry[13] = '*';

    } else
        sprintf(filedir[element].entry, "%-12s ", directory.filename);
}

/************************************************************************/
/* entrymake()                                                        	*/
/* This routine converts one filename from the entry structure to the 	*/
/* "directory" structure.                                             	*/
/************************************************************************/
void
entrymake(struct ffblk * file_buf)
{
    char            string[20];

    /* copy filename   */
    strcpy(directory.filename, file_buf->ff_name);
    strlower(directory.filename);   /* make it lower case */

    /* copy attribute  */
    directory.attribute = file_buf->ff_attrib;

    /* copy date       */
    getdstamp(string, file_buf->ff_fdate);
    strcpy(directory.date, string);

    /* copy time       */
    gettstamp(string, file_buf->ff_ftime);
    strcpy(directory.time, string);

    /* copy filesize   */
    directory.size = file_buf->ff_fsize;
}

/************************************************************************
 *	next_hfile () print next 1) blurb, 2) menu, or 3) helpfile
 ************************************************************************/

void next_hfile(char *name, int *count, int help, int extnum)
{
    char filename[FILESIZE];
    char filename2[FILESIZE];
	char ext2[4];
	char *ext1;

	if (extnum == 1)
		ext1 = "blb";
	else if (extnum == 2)
		ext1 = "mnu";
	else						/* default to ".hlp"	*/
		ext1 = "hlp";
		
    if (changedir(cfg.helppath) == -1)
        return;
    if (*count) {
		if (gl_term.ansiOn) {
			strncpy(ext2,ext1,3);
			ext2[2]= '@';
			ext2[3]= '\0';
			sprintf(filename2, "%.6s%d.%.3s", name, *count,ext2);
		}
        sprintf(filename, "%.6s%d.%.3s", name, *count,ext1);

        if (!filexists(filename) && !(gl_term.ansiOn && filexists(filename2)))
            *count = 0;
        if (debug) {
            cPrintf("checking files %s and %s\n", filename, filename2);
        }
    }
    if (!*count) {
        sprintf(filename, "%.8s.%.3s", name,ext1);
    }
    changedir(cfg.homepath);
    if (debug) {
        cPrintf("Next file %s\n", filename);
    }
    tutorial(filename, help);
    *count += 1;
}

/************************************************************************/
/* hello()  prints random hello blurbs                             		*/
/************************************************************************/
void
hello(void)
{
    gl_user.expert = TRUE;
    nextblurb("hello", &(cfg.cnt.whichHello), 0);
    gl_user.expert = FALSE;
}

/************************************************************************/
/* goodbye()  prints random goodbye blurbs                         		*/
/************************************************************************/
void
goodbye(void)
{
    uchar           screen_height;

	screen_height = logBuf.linesScreen;
    logBuf.linesScreen = 0;
    nextblurb("logout", &(cfg.cnt.whichBye), 0);
    logBuf.linesScreen = screen_height;
}

/************************************************************************/
/* hide()  hides a file. for limited-access u-load                 		*/
/************************************************************************/
void
hide(char *filename)
{
    unsigned char   attr, getattr();
	char *desc_name;

    attr = getattr(filename);

    /* set hidden bit on */
    attr = (attr | 2);

    setattr(filename, attr);
	desc_name = getdescname(filename);
	/* hide the description file too */
	if (desc_name && filexists(desc_name))
		setattr(desc_name, attr);
}

/************************************************************************/
/* readdirectory()  menu level .rd .rvd routine  HIGH level routine 	*/
/************************************************************************/
void
readdirectory(char verbose)
{
    label           filename;

    getNormStr("", filename, FILESIZE, ECHO);

    if (strlen(filename))
        dir(filename, verbose);
    else
        dir("*.*", verbose);
}

/************************************************************************/
/* readtextfile()  menu level .rt  HIGH level routine              		*/
/************************************************************************/
void
readtextfile(void)
{
    label           filename;

    doCR();
    getNormStr("filename", filename, FILESIZE, ECHO);
    if (strlen(filename))
        textdown(filename);
}

/* WC isn't needed on DOS machines which use DSZ or equiv. */
#ifndef __MSDOS__

/************************************************************************/
/* readwc()  menu level .rw  HIGH level routine                    		*/
/************************************************************************/
void
readwc(void)
{
    label           filename;

    doCR();
    getNormStr("filename", filename, FILESIZE, ECHO);

    if (strlen(filename))
        wcdown(filename);
}

#endif

/************************************************************************/
/* renamefile()  aide fn to rename a file                          		*/
/************************************************************************/
void
renamefile(void)
{
    char            source[FILESIZE + 1], destination[FILESIZE + 1];

    doCR();
    getNormStr("source filename", source, FILESIZE, ECHO);
    if (!strlen(source))
        return;

    getNormStr("destination filename", destination, FILESIZE, ECHO);
    if (!strlen(destination))
        return;

    if (changedir(roomBuf.rbdirname) == -1) {
        return;
    }
    if ((checkfilename(source, 0) == ERROR)
        || (checkfilename(destination, 0) == ERROR)
    || ambig(source) || ambig(destination)) {
        mPrintf("\n Invalid filename.");
        changedir(cfg.homepath);
        return;
    }
    if (!filexists(source)) {
        mPrintf(" No file %s", source);
        changedir(cfg.homepath);
        return;
    }
    if (filexists(destination)) {
        mPrintf("\n File exists.");
        changedir(cfg.homepath);
        return;
    }
    /* if successful */
    if (rename(source, destination) == 0) {
        sprintf(msgBuf->mbtext,
        "File %s renamed to file %s in %s] by %s",
        source, destination,
        roomBuf.rbname,
        logBuf.lbname);

        trap(msgBuf->mbtext, T_AIDE);

        aideMessage();
    } else
        mPrintf("\n Cannot rename %s\n", source);

    changedir(cfg.homepath);
}

/************************************************************************/
/* strlwr()   makes a string lower case                               	*/
/************************************************************************/
void
strlower(char *string)
{
    char           *s;
    for (s = string; *s; ++s)
        *s = tolower(*s);
}

/************************************************************************/
/* textdown() dumps a host file with no formatting                 		*/
/* this routine handles wildcarding of text downloads              		*/
/************************************************************************/
void
textdown(char *filename)
{
    int             i;

    outFlag = OUTOK;

    /* no bad files */
    if (checkfilename(filename, 0) == ERROR) {
        mPrintf("\n No file %s", filename);
        return;
    }
    if (changedir(roomBuf.rbdirname) == -1) {
        return;
    }
    if (ambig(filename)) {
    /* fill our directory array according to filename */
        filldirectory(filename, 0);

    /* print out all the files */
        for (i = 0; filedir[i].entry[0] && (dump(filedir[i].entry) != ERROR);
            i++);

        if (!i)
            mPrintf("\n No file %s", filename);

    /* free file directory structure */
        if (filedir != NULL)
            farfree((void *) filedir);
        filedir = NULL;
    } else
        dump(filename);

    sprintf(msgBuf->mbtext, "Text download of file %s in room %s]",
    filename, roomBuf.rbname);

    trap(msgBuf->mbtext, T_DOWNLOAD);

    doCR();

    /* go to our home-path */
    changedir(cfg.homepath);
}

/************************************************************************/
/* textup()  handles textfile uploads                              		*/
/************************************************************************/
void
textup(char *filename)
{
    int             i;

    if (!gl_user.expert)
        nextblurb("textup", &(cfg.cnt.tcount), 1);

    if (changedir(roomBuf.rbdirname) == -1) {
        return;
    }
    doCR();

    if ((upfd = fopen(filename, "wt")) == NULL) {
        mPrintf("\n Can't create %s!\n", filename);
    } else {
        while (((i = iChar()) != 0x1a /* CNTRLZ */ )
            && outFlag != OUTSKIP
        && (onConsole || gotCarrier())) {
            fputc(i, upfd);
        }
        fclose(upfd);

        sprintf(msgBuf->mbtext, "Text upload of file %s in room %s]",
        filename, roomBuf.rbname);

        if (limitFlag && filexists(filename))
            hide(filename);

        trap(msgBuf->mbtext, T_UPLOAD);
    }

    changedir(cfg.homepath);
}

/************************************************************************
 *      tutorial() dumps fomatted help files
 *      this routine handles wildcarding of formatted text downloads
 *              will not print "can't find %s.blb" if showhelp is 0
 ************************************************************************/
void tutorial(char *filename, int showhelp)
{
    int             i;
    char            temp[FILESIZE];

    outFlag = OUTOK;

    if (!gl_user.expert)
        mPrintf("\n <J>ump <N>ext <P>ause <S>top\n");
    doCR();

    if (changedir(cfg.helppath) == -1) {
        return;
    }
    /* no bad files */
    if (checkfilename(filename, 0) == ERROR) {
        if (showhelp) {
            mPrintf("\n No helpfile %s", filename);
        }
        changedir(cfg.homepath);
        return;
    }
    if (ambig(filename)) {
    /* fill our directory array according to filename */
        filldirectory(filename, 0);

    /* print out all the files */
        for (i = 0; filedir[i].entry[0] && (dumpf(filedir[i].entry) != ERROR);
            i++);

        if (!i && showhelp)
            mPrintf("\n No helpfile %s", filename);

    /* free file directory structure */
        if (filedir != NULL)
            farfree((void *) filedir);
        filedir = NULL;
    } else {
        strcpy(temp, filename);
        temp[strlen(temp) - 1] = '@';

        if (gl_term.ansiOn && filexists(temp))
            dump(temp);
        else
            dumpf(filename);
    }

    /* go to our home-path */
    changedir(cfg.homepath);
}

/************************************************************************/
/* unlinkfile()  handles .au  aide unlink                          		*/
/************************************************************************/
void
unlinkfile(void)
{
    label           filename;

    getNormStr("filename", filename, FILESIZE, ECHO);

    if (changedir(roomBuf.rbdirname) == -1) {
        return;
    }
    if (checkfilename(filename, 0) == ERROR) {
        mPrintf(" No file %s", filename);
        changedir(cfg.homepath);
        return;
    }
    if (!filexists(filename)) {
        mPrintf(" No file %s", filename);
        changedir(cfg.homepath);
        return;
    }
    /* if successful */
    if (unlink(filename) == 0) {
        sprintf(msgBuf->mbtext,
        "File %s unlinked in %s] by %s",
        filename,
        roomBuf.rbname,
        logBuf.lbname);

        trap(msgBuf->mbtext, T_AIDE);

        aideMessage();

        killinfo(filename);

    } else
        mPrintf("\n Cannot unlink %s\n", filename);

    changedir(cfg.homepath);
}

/* WC isn't needed on DOS machines which use DSZ or equiv. */
#ifndef __MSDOS__
/************************************************************************/
/* wcdown() calls xmodem downloading routines                      		*/
/* 0=wc, 1=wx                                                      		*/
/************************************************************************/
void
wcdown(char *filename)
{
    long            transTime1, transTime2;

    if (changedir(roomBuf.rbdirname) == -1) {
        return;
    }
    /* no bad files */
    if (checkfilename(filename, 0) == ERROR) {
        mPrintf("\n No file %s", filename);
        changedir(cfg.homepath);
        return;
    }
    /* no ambiguous xmodem downloads */
    if (ambig(filename)) {
        changedir(cfg.homepath);
        return;
    }
    if (!filexists(filename)) {
        mPrintf("\n No file %s", filename);
    } else {
        if (!gl_user.expert)
            nextblurb("wcdown", &(cfg.cnt.wcdcount), 1);

    /* display # blocks & download time */
        blocks(filename, 128);

        if (getYesNo("Ready for file transfer", 0)) {
            transTime1=cit_time();
            trans(filename);
            transTime2=cit_time();

            if (cfg.accounting && !logBuf.lbflags.NOACCOUNT && !specialTime) {
                calc_trans(transTime1, transTime2, 0);
            }
            sprintf(msgBuf->mbtext, "WC download of file %s in room %s]",
            filename, roomBuf.rbname);

            trap(msgBuf->mbtext, T_DOWNLOAD);
        }
    }
    /* go back to home */
    changedir(cfg.homepath);
}

/************************************************************************/
/* wcup() calls xmodem uploading routines                          		*/
/************************************************************************/
void
wcup(char *filename)
{
    long            transTime1, transTime2;

    if (!gl_user.expert)
        nextblurb("wcup", &(cfg.cnt.wcucount), 1);

    if (changedir(roomBuf.rbdirname) == -1) {
        return;
    }
    doCR();

    if (getYesNo("Ready for file transfer", 0)) {
    /* later to be replaced by my own xmodem routines */

        transTime1=cit_time();  /* when did they start the Uload    */
        rxfile(filename);
        transTime2=cit_time();

        if (cfg.accounting && !logBuf.lbflags.NOACCOUNT && !specialTime) {
            calc_trans(transTime1, transTime2, 1);
        }
        sprintf(msgBuf->mbtext, "WC upload of file %s in room %s]",
        filename, roomBuf.rbname);

        if (limitFlag && filexists(filename))
            hide(filename);

        trap(msgBuf->mbtext, T_UPLOAD);

    }
    /* go back to home */
    changedir(cfg.homepath);
}
#endif

/*
 * Dragon Mods
 *
 * These rutines were added for external proticals suport, it used to be hard
 * coded.. Now it is configurable. I promise to replace the xmodem rutine
 * that is still internal... It needs to die.. ( peice of trash that it is )
 */

extern int      bauds[];

/************************************************************************
 *  Extended Download
 ************************************************************************/
void
wxsnd(char *path, char *file, char trans)
{
    char            stuff[100];
    label           tmp1, tmp2;

    if (changedir(path) == -1) {
        return;
    }
    sprintf(tmp1, "%d", cfg.mdata);
    sprintf(tmp2, "%d", bauds[speed]);
    sformat(stuff, extrn[trans - 1].ex_snd,
		"fpsan", file, tmp1, tmp2, cfg.aplpath, logBuf.lbname);
    apsystem(stuff);

    if (debug)
        cPrintf("(%s)", stuff);
}

/************************************************************************
 *  Extended Upload
 ************************************************************************/
void
wxrcv(char *path, char *file, char trans)
{
    char            stuff[100];
    label           tmp1, tmp2;

    if (changedir(path) == -1) {
        return;
    }
    sprintf(tmp1, "%d", cfg.mdata);
    sprintf(tmp2, "%d", bauds[speed]);
    sformat(stuff, extrn[trans - 1].ex_rcv,
    "fpsan", file, tmp1, tmp2, cfg.aplpath, logBuf.lbname);
    apsystem(stuff);

    if (debug)
        cPrintf("(%s)", stuff);
}

/************************************************************************
 * countdown - return nonzero on abort
 ************************************************************************/
static int countdown(char *prompt, long timeout)
{
    long            timer, cur, last;

    cur = timer = cit_timer();
    last = timeout;
    timer += timeout;
    mPrintf("\n\nCountdown to %s, ESC to abort.\n\n%ld\a", prompt, timeout);
    while (timer > cur) {
        cur = cit_timer();
        if (last > timer - cur) {
            last = timer - cur;
            mPrintf(" . %ld", last);
        }
        if (BBSCharReady()) {
            if (iChar() == 0x1b) {
                mPrintf(" Aborted!\n");
                return 1;
            }
        }
    }
    mPrintf("\n\n%s!\a\n\n", prompt);
    return 0;
}

/************************************************************************/
/* download()  menu level download routine                         		*/
/************************************************************************/
void
download(char c)
{
    long transTime1, transTime2; /* to give no cost uploads       */
    char filename[80];
    char ch, xtype;
    int  term_user,findex,num_files;
	int  found_bad=0,total_files=0;

    if (!c)
        ch = tolower(iChar());
    else
        ch = c;

    xtype = strpos(ch, extrncmd);

    if (!xtype) {
        if (ch == '?')
            upDownMnu('D');
        else {
            mPrintf(" ?");
            if (!gl_user.expert)
                upDownMnu('D');
        }
        return;
    } else {
        if (!onConsole && extrn[xtype - 1].ex_console) {
            doCR();
            mPrintf("\b%s is console or network only.\n",
				extrn[xtype - 1].ex_name);
            return;
        } else
            mPrintf("\b%s", extrn[xtype - 1].ex_name);
    }

    doCR();

    if (changedir(roomBuf.rbdirname) == -1) {
        return;
    }
    getNormStr("filename", filename,
		(extrn[xtype - 1].ex_batch) ? 80 : FILESIZE, ECHO);

    if (extrn[xtype - 1].ex_batch) {
        char           *words[256];
        int             count, i;
        char            temp[80];

        strcpy(temp, filename);

        count = parse_it(words, temp);

        if (count == 0) {
			changedir(cfg.homepath);
            return;
		}

        for (i = 0; i < count; i++) {
			found_bad = num_files = 0;
			if (ambig(words[i])) {
				/* load our directory structure according to filename */
				found_bad = 0;
				filldirectory(words[i], 0);
				for (findex = 0;filedir[findex].entry[0];++findex) {
					if (checkfilename(words[i], 0) == ERROR) {
						++found_bad;
					} else {
						++num_files;
					}
				}
				/* free file directory structure */
				if (filedir != NULL)
					farfree((void *) filedir);
				filedir = NULL;
			} else {
				if (filexists(words[i]) && (checkfilename(words[i],0)!=ERROR)){
					++num_files;
					doCR();
					mPrintf("%s: ", words[i]);
					blocks(words[i], extrn[xtype - 1].ex_block);
				}
			}
			if (!num_files || found_bad) {
				if (!num_files)
					mPrintf("\n No file %s.", words[i]);
				else if (found_bad)
					mPrintf("\n Invalid file in %s.", words[i]);
				break;
			}
			total_files += num_files;
        }
		
    } else {
		found_bad = 0;
		num_files = 1;
        if (checkfilename(filename, 0) == ERROR) {
            mPrintf("\n Invalid file %s.", filename);
			found_bad = 1;
			
        } else if (ambig(filename)) {
            mPrintf("\n Not a batch protocol.");
			found_bad = 1;
			
        } else if (!filexists(filename)) {
            mPrintf("\n No file %s.", filename);
			num_files = 0;
		}
		if (!found_bad && num_files)
			blocks(filename, extrn[xtype - 1].ex_block);
    }

	if (!num_files || found_bad || !strlen(filename)) {
		changedir(cfg.homepath);
		return;
	}

    if (!gl_user.expert)
        nextblurb("wcdown", &(cfg.cnt.downcount), 1);

	if (total_files > 1)
		mtPrintf(TERM_BOLD,"\n %d files.", total_files);
    term_user = getYesNo("Hang up after file transfer", 0);

    if (getYesNo("Ready for file transfer", 0)) {
        transTime1=cit_time();
        wxsnd(roomBuf.rbdirname, filename, xtype);
        transTime2=cit_time();

        if (cfg.accounting && !logBuf.lbflags.NOACCOUNT && !specialTime) {
            calc_trans(transTime1, transTime2, 0);
        }
        sprintf(msgBuf->mbtext, "%s download of file %s in room %s]",
			extrn[xtype - 1].ex_name, filename, roomBuf.rbname);

        trap(msgBuf->mbtext, T_DOWNLOAD);
        if (term_user && !countdown("logoff", 10)) { /* 10 second countdown */
            terminate( /* hangUp == */ TRUE, TRUE);
        }
    }
    /* go back to home */
    changedir(cfg.homepath);
}

/************************************************************************/
/* upload()  menu level routine                                    		*/
/************************************************************************/
void
upload(char c)
{
    long            transTime1, transTime2;
    label           filename;
    char            comments[64];
    char            ch, xtype;
    int             term_user = 0;

    if (!c)
        ch = tolower(iChar());
    else
        ch = c;

    xtype = strpos(ch, extrncmd);

    if (!xtype) {
        if (ch == '?')
            upDownMnu('U');
        else {
            mPrintf(" ?");
            if (!gl_user.expert)
                upDownMnu('U');
        }
        return;
    } else {
        if (!onConsole && extrn[xtype - 1].ex_console) {
            doCR();
            mPrintf("\b%s is console or network only.",
				extrn[xtype - 1].ex_name);
            return;
        } else {
            mPrintf("\b%s", extrn[xtype - 1].ex_name);
        }
    }

    doCR();

    if (!extrn[xtype - 1].ex_batch) {
        getNormStr("filename", filename, FILESIZE, ECHO);

        if (checkup(filename) == ERROR)
            return;

        if (strlen(filename)) {
            getString("comments", comments, 64, FALSE, TRUE, "");
			enterdesc(filename,0);
        } else
            return;
		term_user = getYesNo("Hang up after file transfer", 0);
    }

    if (!gl_user.expert)
        nextblurb("wcup", &(cfg.cnt.upcount), 1);

    doCR();

    if (getYesNo("Ready for file transfer", 0)) {
        batchinfo(0);		/* reset file comments */
        transTime1=cit_time();  /* when did they start the Uload    */
        wxrcv(roomBuf.rbdirname, extrn[xtype - 1].ex_batch ? "" : filename,
        xtype);
        transTime2=cit_time();  /* when did they get done           */

        if (cfg.accounting && !logBuf.lbflags.NOACCOUNT && !specialTime) {
            calc_trans(transTime1, transTime2, 1);
        }
        if (!extrn[xtype - 1].ex_batch) {
            if (limitFlag && filexists(filename))
                hide(filename);

            if (filexists(filename)) {
                entercomment(filename, logBuf.lbname, comments);

                sprintf(msgBuf->mbtext, "%s upload of file %s in room %s]",
                extrn[xtype - 1].ex_name, filename, roomBuf.rbname);

                trap(msgBuf->mbtext, T_UPLOAD);

                if (comments[0])
                    sprintf(msgBuf->mbtext,
						"%s uploaded by %s\n Comments: %s",
						filename, logBuf.lbname, comments);
                else
                    sprintf(msgBuf->mbtext, "%s uploaded by %s", filename,
                    logBuf.lbname);

                specialMessage();
            }
        } else {
            sprintf(msgBuf->mbtext, "%s file upload in room %s]",
            extrn[xtype - 1].ex_name, roomBuf.rbname);

            trap(msgBuf->mbtext, T_UPLOAD);

            if (batchinfo(1))
                specialMessage();
        }
        if (term_user && !countdown("logoff", 10))
            terminate( /* hangUp == */ TRUE, TRUE);
    }
    changedir(cfg.homepath);
}

/*
 * Up/Down menu
 */
void
upDownMnu(char cmd)
{
    int             i;

    doCR();
    doCR();
    for (i = 0; i < strlen(extrncmd); i++)
        if (!onConsole && extrn[i].ex_console)
            continue;
        else
            mPrintf(" .%c%c>%s\n", cmd, *(extrn[i].ex_name),
            (extrn[i].ex_name + 1));
    mPrintf(" .%c?> -- this list\n", cmd);
    doCR();
}

/* EOF */
