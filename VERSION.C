/* ------------------------------------------------------------------------- */
/*  Makes us able to change the version at a glance       VERSION.C          */
/* ------------------------------------------------------------------------- */


/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/VERSION.C_V  $
 * 
 *    Rev 1.38   21 Sep 1991 10:19:30   FJM
 * FredCit release
 *
 *    Rev 1.15   06 Jan 1991 12:46:08   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.14   27 Dec 1990 20:16:26   FJM
 * Changes for porting.
 *
 *    Rev 1.4   23 Nov 1990 13:25:32   FJM
 * Header change
 *
 *    Rev 1.2   17 Nov 1990 16:12:16   FJM
 * Added version control log header
 * -------------------------------------------------------------------- */

/* --------------------------------------------------------------------
 *  History:
 *
 *  03/03/90    GREN    Display the 'family tree' of some Citadels.
 *  03/07/90    {zm}    Add 'softname' as the name of the software!
 *  05/20/90    FJM     Put compdate[] and comptime here (force reconfig)
 *                                              Some cleanup.
 * -------------------------------------------------------------------- */
#include "ctdl.h"

char *softname = "FredCit";

#ifndef ATARI_ST
char *version = REL;

#else
char *version = "1.0";

#endif

const char compdate[] = __DATE__;
const char comptime[] = __TIME__;

char *welcome[] = {     /* 10 LINES MAX LENGTH!!!!!! */
    "                GremCit (Gremlin)               ",
    "                        ³                       ",
    "     ÚÄÄÄÄÄÄÄÄÄDragCit (Dragon/Ray)ÄÄÄÄÄÄ¿      ",
    "     ³                  ³                ³      ",
    "(Joe Broxson)   (Fred, Zen, others)    (Ray)    ",
    "DragCit Rev. B. FredCit and Kremlin    TurboCit ",
    "                                                ",
    "         Thanks to all that came before.        ",
    0,
    0,
    0
};

char *copyright[] = {       /* 2 LINES ONLY!!!! */
    "Fred's Citadel",
    "Public Domain Software",
    0
};

 /* CRC's for the array's above */

unsigned int welcomecrcs[] = {
    0x0b66, 0x9f6f, 0xc28c, 0x5a24, 0xdd2b, 0xa0d3, 0x2ed4, 0xa757, 0x2ed4, 0xa757
};

unsigned int copycrcs[] = {0xd5eb, 0x6b01};
