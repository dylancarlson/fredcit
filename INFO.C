/* -------------------------------------------------------------------- */
/*  INFO.C                    Citadel                                   */
/* -------------------------------------------------------------------- */
/*  This module deals with the fileinfo.cits and the .ri commands       */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Includes                                                            */
/* -------------------------------------------------------------------- */
#include <string.h>
#include <io.h>
#include "ctdl.h"
#include "proto.h"
#include "global.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  updateFile()    Update or add file to fileinfo.cit                  */
/*  newFile()       Add a new file to the fileinfo.cit                  */
/*  entercomment()  Update/add comment, high level (assumes cur room)   */
/*  setfileinfo()   menu level .as routine sets entry to aide's name    */
/*                  if none present or leaves original uploader         */
/*  getInfo()       get infofile slot for this file (current room)      */
/* -------------------------------------------------------------------- */


/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/INFO.C_V  $
 * 
 *    Rev 1.42   21 Sep 1991 10:18:56   FJM
 * FredCit release
 *
 *    Rev 1.41   06 Jun 1991  9:19:06   FJM
 *
 *    Rev 1.40   22 May 1991  2:16:14   FJM
 * Added description files.
 *
 *    Rev 1.25   18 Jan 1991 16:50:22   FJM
 *    Rev 1.17   06 Jan 1991 12:44:44   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.16   27 Dec 1990 20:15:56   FJM
 * Changes for porting.
 *
 *    Rev 1.13   22 Dec 1990 13:37:42   FJM
 *    Rev 1.6   07 Dec 1990 23:10:18   FJM
 *    Rev 1.4   23 Nov 1990 13:24:58   FJM
 * Header change
 *
 *    Rev 1.2   17 Nov 1990 16:11:30   FJM
 * Added version control log header
 * -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  HISTORY:                                                            */
/*                                                                      */
/*  05/07/89    (PAT)   Module created (rewrite of some infofile.c      */
/*                      functions for speed)                            */
/*  03/15/90    {zm}    Use FILESIZE (20) instead of NAMESIZE (30).     */
/*  03/30/90    FJM     Added fflush() calls where needed               */
/*                                                                      */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  External data                                                       */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Static Data                                                         */
/* -------------------------------------------------------------------- */
static void updateFile(char *dir, char *file, char *user, char *comment);
static void newFile(char *dir, struct fInfo * ours);
static BOOL getInfo(char *file, struct fInfo * ours);

/* -------------------------------------------------------------------- */
/*  updateFile()    Update or add file to fileinfo.cit                  */
/* -------------------------------------------------------------------- */
static void updateFile(char *dir, char *file, char *user, char *comment)
{
    struct fInfo info, ours;
    FILE *fl;
    char path[80];
    BOOL found = FALSE;
	unsigned int pos = 0;

    /* setup the buffer for write */
	ours.fn[12]=ours.uploader[sizeof(label)-1]=ours.comment[64]='\0';
    strncpy(ours.fn, file, 13);
    strncpy(ours.uploader, user, sizeof(label));
    strncpy(ours.comment, comment, 64);

    sprintf(path, "%s\\fileinfo.cit", dir);

    if ((fl = fopen(path, "r+b")) == NULL) {
        newFile(path, &ours);
        return;
    }
    while (fread(&info, sizeof(struct fInfo), 1, fl) == 1 && !found) {
		++pos;
		if (*info.fn && !isprint(*info.fn)) {
			amPrintf(" \n %s\\fileinfo.cit corrupted at record %u.",
				roomBuf.rbdirname, pos);
			amPrintf(" \n Erasing record %d.\n\n", pos);
			SaveAideMess();
            fseek(fl, (long) (-(sizeof(struct fInfo))), SEEK_CUR);
			break;
		}
        if (strcmpi(file, info.fn) == SAMESTRING) {
        /* seek back and overwrite it */
            fflush(fl);
            fseek(fl, (long) (-(sizeof(struct fInfo))), SEEK_CUR);
            fwrite(&ours, sizeof(struct fInfo), 1, fl);
			found = TRUE;
        }
    }

    fclose(fl);

    if (!found)
        newFile(path, &ours);
}

/* -------------------------------------------------------------------- */
/*  newFile()       Add a new file to the fileinfo.cit                  */
/* -------------------------------------------------------------------- */
static void newFile(char *file, struct fInfo * ours)
{
    FILE *fl;
	long size;
	long dif;

	fl = fopen(file, "ab");
    if (!fl) {
		amPrintf(" \n Error opening %s for write: %s\n\n",
			file, strerror(errno));
        SaveAideMess();
    } else {
		size = filelength(fileno(fl));
		if (size % (long) sizeof(struct fInfo)) {
			amPrintf(" \n %s\\fileinfo.cit corrupted.",roomBuf.rbdirname);
			amPrintf(" \n Adjusting file length\n");
			SaveAideMess();
			/* try to patch up the fileinfo */
			for (dif=size%(long)sizeof(struct fInfo);
				dif; --dif) {
				fputc(0,fl);
			}
		}
		fwrite(ours, sizeof(struct fInfo), 1, fl);
	
		fclose(fl);
	}
}


/* -------------------------------------------------------------------- */
/*  entercomment()  Update/add comment, high level (assumes cur room)   */
/* -------------------------------------------------------------------- */
void entercomment(char *filename, char *uploader, char *comment)
{
    updateFile(roomBuf.rbdirname, filename, uploader, comment);
}

/* -------------------------------------------------------------------- */
/*  setfileinfo()   menu level .as routine sets entry to aide's name    */
/*                  if none present or leaves original uploader         */
/* -------------------------------------------------------------------- */
void setfileinfo(void)
{
    label filename;
    label uploader;
    char comments[64];
    char path[80];
    struct fInfo old;

    getNormStr("filename", filename, FILESIZE, ECHO);

    sprintf(path, "%s\\%s", roomBuf.rbdirname, filename);

    /* no bad file names */
    if (checkfilename(filename, 0) == ERROR) {
        mPrintf("\n No file %s.\n ", filename);
        return;
    }
    /* no file name? */
    if (!filexists(path)) {
        mPrintf("\n No file %s.\n ", filename);
        return;
    }
    if (!getInfo(filename, &old)) {
        strcpy(uploader, logBuf.lbname);
    } else {
        strcpy(uploader, old.uploader);
    }

    getString("comments", comments, 64, FALSE, TRUE, "");
    entercomment(filename, uploader, comments);
	enterdesc(filename,1);

    sprintf(msgBuf->mbtext, "File info changed for file %s by %s",
		filename, logBuf.lbname);

    trap(msgBuf->mbtext, T_AIDE);
}

/* -------------------------------------------------------------------- */
/*  getInfo()       get infofile slot for this file (current room)      */
/* -------------------------------------------------------------------- */
static BOOL getInfo(char *file, struct fInfo * ours)
{
    struct fInfo info;
    FILE *fl;
    char path[80];
    BOOL found = FALSE;

    sprintf(path, "%s\\fileinfo.cit", roomBuf.rbdirname);

    if ((fl = fopen(path, "rb")) == NULL) {
        return FALSE;
    }
    while (fread(&info, sizeof(struct fInfo), 1, fl) == 1 && !found) {
        if (strcmpi(file, info.fn) == SAMESTRING) {
            *ours = info;

            found = TRUE;
        }
    }

    return found;
}

/* EOF */
