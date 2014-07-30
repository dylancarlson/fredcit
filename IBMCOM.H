/*---------------------------------------------------------------------------*
 *                                 ibmcom.h                                  *
 *---------------------------------------------------------------------------*
 * DESCRIPTION: ANSI C function prototypes and other definitions for the     *
 *              routines in ibmcom.c                                         *
 *  03/19/90    FJM     Linted & partial cleanup                             *
 *---------------------------------------------------------------------------*/

/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/IBMCOM.H_V  $
 * 
 *    Rev 1.38   06 Jun 1991  9:20:02   FJM
 *
 *   Rev 1.15   06 Jan 1991 12:46:38   FJM
 *Porting, formating, fixes and misc changes.
 *
 *   Rev 1.14   27 Dec 1990 20:17:18   FJM
 *Changes for porting.
 *
 *     Rev 1.3   23 Nov 1990 13:25:40   FJM
 *  Header change
 *
 *     Rev 1.1   17 Nov 1990 16:12:34   FJM
 *  Added version control log header
 * -------------------------------------------------------------------- */


enum par_code   { COM_NONE, COM_EVEN, COM_ODD, COM_ZERO, COM_ONE };

int             com_carrier(void);
void            com_deinstall(void);
void            com_flush_rx(void);
void            com_flush_tx(void);
int             com_install(int portnum);
void interrupt  com_interrupt_driver();
void            com_lower_dtr(void);
void            com_raise_dtr(void);
int             com_ring(void);
unsigned char            com_rx(void);
int             com_rx_empty(void);
void            com_set_parity(enum par_code parity, int stop_bits);
void            com_set_speed(long speed);
void            com_tx(char c);
int             com_tx_empty(void);
int             com_tx_ready(void);
void            com_tx_string(char *s);

