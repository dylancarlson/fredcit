/* $Header:   D:/VCS/FCIT/MACHINE.H_V   1.29   29 May 1991 11:21:10   FJM  $ */

/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/MACHINE.H_V  $
 * 
 *    Rev 1.29   29 May 1991 11:21:10   FJM
 * Removed redundent includes.
 *
 *    Rev 1.28   16 May 1991  8:44:24   FJM
 * Added NEAR
 *
 *   Rev 1.5   06 Jan 1991 12:46:40   FJM
 *Porting, formating, fixes and misc changes.
 *
 *    Rev 1.11   27 Dec 1990 20:17:24   FJM
 * Changes for porting.
 *
 *   Rev 1.1   22 Dec 1990 13:50:46   FJM
 *
 *   Rev 1.0   22 Dec 1990  0:27:30   FJM
 *Initial revision.
 *
 * -------------------------------------------------------------------- */

#include "ibmcom.h"				/* for macros to work - don't use directly! */

#define NEAR	near
#ifndef FAR
#define FAR     far
#endif
#define HUGE    huge
