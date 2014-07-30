/* -------------------------------------------------------------------- */
/*  All the global variables in all of citadel. Put here to             */
/*  make moving functions around a hell of a lot easier.                */
/* -------------------------------------------------------------------- */


/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/GLOBAL.H_V  $
 *
 *    Rev 1.42   01 Nov 1991 11:21:12   FJM
 * Added gl_ structures
 *
 *    Rev 1.41   21 Sep 1991 10:19:38   FJM
 * FredCit release
 *
 *    Rev 1.40   06 Jun 1991  9:20:00   FJM
 *
 *   Rev 1.25   18 Jan 1991 16:52:54   FJM
 *   Rev 1.17   06 Jan 1991 12:46:18   FJM
 *Porting, formating, fixes and misc changes.
 *
 *   Rev 1.16   27 Dec 1990 20:17:18   FJM
 *Changes for porting.
 *
 *     Rev 1.4   23 Nov 1990 12:32:24   FJM
 *  Changed my full name to just initials.
 *
 *     Rev 1.2   18 Nov 1990 19:28:26   FJM
 *  Deleted locally used variable infosize.
 *
 *     Rev 1.1   17 Nov 1990 16:12:26   FJM
 *  Added version control log header
 * -------------------------------------------------------------------- */

/* --------------------------------------------------------------------
 *  History since 3.10.05d:
 *
 *  03/07/90    {zm}    Add 'softname' from VERSION.C
 *  03/12/90    {zm}    Add #DEFAULT_GROUP to external.cit
 *  03/12/90    FJM     Added IBM Graphics character translation
 *  03/19/90    FJM     Linted & partial cleanup
 *                      Changed my comptime/compdate thingy.  Now always
 *                      recompile.
 *  06/16/90    FJM     Made IBM Graphics characters a seperate option.
 *  07/22/90    FJM     Added 0 marker at end of baud rate list
 * -------------------------------------------------------------------- */

#define FILT_LEN  256

/* -------------------------------------------------------------------------
 *
 *	Definitions for MISC file.
 *
 * ------------------------------------------------------------------------- */
#ifdef MISC

/*==== FILES ====*/
FILE *msgfl;            		/* msg file descriptor          */
FILE *printfile;        		/* printer.out                  */
FILE *trapfl;           		/* trap file descriptor         */
FILE *upfd;
FILE *aideFl;		        	/* a temp file for aide messages */
FILE *hallfl;           		/* Hall file descriptor         */
FILE *grpfl;            		/* Group file descriptor        */
FILE *roomfl;           		/* Room file descriptor         */
FILE *logfl;            		/* log file descriptor          */

/*=== STRUCTURES ====*/
/* struct logBuffer lBuf2;*/    /* Temp buffer...........       */
label editcmd;          		/* External editor commands     */
struct aRoom roomBuf;       	/* Room buffer                  */
struct accountBuffer *accountBuf;/* Accounting buffer            */
struct accounting thisAccount;  /* Groupdata used for current user  */
struct apldata userdat;     	/* Output to application        */
struct config cfg;      		/* A buncha variables           */
struct groupBuffer grpBuf;  	/* Group data goes here         */
struct logBuffer lBuf;      	/* Temp buffer...........       */
struct logBuffer logBuf;    	/* Log buffer of a person       */
struct mfilter mf;      		/* Message filter               */
struct msginfo mesginfo;    	/* User/room for message from aplic */
struct nodest node;     		/* node buffer                  */
struct parms parm;      		/* command line parms            */
struct talleyBuffer talleyBuf;  /* message talley for rooms     */

struct ext_editor *edit;    	/* External editor commands     */
struct fDir *filedir;       	/* pointer for filedir struct   */
struct fInfo *fileinfo;     	/* pointer for file-info struct */
struct hallBuffer *hallBuf; 	/* Hall stuff right here        */
struct lTable *logTab;      	/* RAM index of pippuls         */
struct messagetable huge *msgTab;   /* THE message table            */
struct msgB *msgBuf2;       	/* The Old-message buffer       */
struct msgB *msgBuf;        	/* The -sole- message buffer    */

/* -------------------------------------------------------------------------
 *
 *	Declarations for all but MISC file.
 *
 * ------------------------------------------------------------------------- */
#else
	
/*==== FILES ====*/
extern FILE *msgfl;     		/* msg file descriptor          */
extern FILE *printfile;     	/* printer.out                  */
extern FILE *trapfl;        	/* trap file descriptor         */
extern FILE *upfd;
extern FILE *aideFl;        	/* a temp file for aide messages */
extern FILE *hallfl;        	/* Hall file descriptor         */
extern FILE *grpfl;     		/* Group file descriptor        */
extern FILE *roomfl;        	/* Room file descriptor         */
extern FILE *logfl;     		/* log file descriptor          */

/*==== STRUCTURES ====*/
/* extern struct logBuffer lBuf2; */ /* Temp buffer...........       */
extern label editcmd;       	/* External editor commands     */
extern struct aRoom roomBuf;    /* Room buffer                  */
extern struct accountBuffer *accountBuf; /* Accounting buffer            */
extern struct accounting thisAccount;   /* Groupdata used for current user  */
extern struct apldata userdat;  /* Output to application        */
extern struct config cfg;   	/* A buncha variables           */
extern struct groupBuffer grpBuf;   /* Group data goes here         */
extern struct logBuffer lBuf;   /* Temp buffer...........       */
extern struct logBuffer logBuf; /* Log buffer of a person       */
extern struct mfilter mf;   	/* Message filter               */
extern struct msginfo mesginfo; /* User/room for message from aplic */
extern struct nodest node;  	/* node buffer                  */
extern struct parms parm;
extern struct talleyBuffer talleyBuf;   /* message talley for rooms     */

extern struct ext_editor *edit; /* External editor commands     */
extern struct fDir *filedir;    /* pointer for filedir struct  */
extern struct fInfo *fileinfo;  /* pointer for file-info struct */
extern struct hallBuffer *hallBuf;  /* Hall stuff right here        */
extern struct lTable *logTab;   /* RAM index of pippuls         */
extern struct messagetable huge *msgTab;    /* THE message table            */
extern struct msgB *msgBuf2;    /* The Old-message buffer       */
extern struct msgB *msgBuf; 	/* The -sole- message buffer    */

#endif

/* -------------------------------------------------------------------------
 *
 *	Definitions for MISC2 file.
 *
 * ------------------------------------------------------------------------- */

#ifdef MISC2
char extrncmd[MAXEXTERN];   	/* External command letters     */
struct ext_prot *extrn;     	/* External protocols buffer    */
struct ext_command *extCmd; 	/* External commands buffer     */
struct rTable *roomTab;     	/* RAM index of rooms           */
struct sgl_term gl_term;		/* Global terminal flags/values */
struct sgl_user gl_user;		/* Global user flags/values		*/

struct sgl_str gl_str = {		/* Global strings (saves RAM)	*/
	"Yes",						/* char yes[]; 					*/
	"No",						/* char no[]; 					*/
	"On",						/* char on[]; 					*/
	"Off"						/* char off[]; 					*/
};

//struct sgl_attr gl_attr;	/* Global screen attributes		*/

/*==== Function pointers ====*/
void (*charattr) (char, uchar);
void (*stringattr) (uint, char *, uchar);

/*==== OTHER ====*/
label default_group;        	/* Default for net messages */
char specialTime;         		/* Is it a "special" hour           */
float startbalance;     		/* Number of credits at log-in      */
long lasttime;          		/* timestamp of last update         */
char readMessage = TRUE;
char ExitToMsdos = FALSE;   	/* bring system down         */
char sysReq = FALSE;        	/* SysOp wants system        */
char *confirm = "Confirm";		/* a string for a prompt		*/
char mailFlag;          		/* Sending private mail?        */
char oldFlag;           		/* Entering old message?        */
char limitFlag;         		/* Entering Limited-Access      */
char chatReq = FALSE;       	/* Did they want to chat?       */
char dowhat = DUNO;     		/* What is calling iChar        */
int xpd, duplic;
char duplicate, expired;    	/* count duplicates and expired */
char netError = FALSE;      	/* save net error message?      */
unsigned char thisHall;     	/* hall we're in                */
int thisLog;            		/* entry currently in logBuf    */
int thisRoom = LOBBY;       	/* Current room                 */
char etcpath[64];       		/* Path etc.dat and ctdl.exe in */
int thisSlot;           		/* logTab slot logBuf found via */
char loggedIn = FALSE;      	/* Global have-caller flag      */
char prevChar;          		/* for EOLN/EOParagraph stuff   */
int people;         			/* this is the old slot         */
char sysopNew = FALSE;
long uptimestamp;
long logtimestamp;
long conntimestamp;
ulong transmitted;      		/* how many characters transmitted */
ulong received;         		/* how many characters transmitted */
char backout = FALSE;       	/* Words out backward?       */
char justLostCarrier = FALSE;   /* Just lost it        */
char newCarrier = FALSE;    	/* Just got carrier         */
char onConsole;         		/* Who's in control?!?           */
unsigned char modStat;      	/* Had carrier LAST time you     */
								/* you checked?                  */
char whichIO = CONSOLE;     	/* CONSOLE or MODEM            */
char haveCarrier;       		/* set if DCD == TRUE            */
char echo;          			/* Either NEITHER, CALLER, or BOTH */
char echoChar;          		/* What to echo with if echo == NEITHER */
char anyEcho = TRUE;
char sysopkey = FALSE;
char chatkey = FALSE;
char eventkey = FALSE;
char disabled;          		/* is modem disabled?   */
int getkey;
char speed;
char detectflag = FALSE;    	/* got carrier while on console */

#ifdef OLD_ASCII_TBL
char filter[FILT_LEN];      	/* full ASCII filtering */
#else
#include "translat.h"
#endif

char modem = TRUE;
char printing = FALSE;
char console = TRUE;        	/* output flag for console */
char ConLock = FALSE;
char debug = FALSE;
char callout = FALSE;
unsigned char crtColumn;    	/* current position on screen  */
char outFlag = OUTOK;       	/* will be one of the above    */
char dotoMessage = NO_SPECIAL;
ulong markedMId = 0l;       	/* id of message to be marked  */

char heldMessage;
char replyFlag;         		/* Sending a reply?            */
char linkMess;          		/* Sending a file-linked message? */
int mread, entered;     		/* #messages read and entered  */

ulong originalId = 0l;      	/* this is the ID of origin      */
unsigned char originalattr; 	/* Attribute byte of origin      */
char copyflag = FALSE;      	/* true when viewing a duplicate */
char MessageRoom[MAXROOMS]; 	/* number of message/room this call */
char seen;
int bauds[] = {300, 1200, 2400, 4800, 9600, 19200, 38400U, 0};
char showdir, showhidden, showgroup;
char skiproom = TRUE;
int oldroom, ug_lvisit, ug_new;
uchar scrollpos = 23;       	/* adjusts for help menu         */
uchar cursorch, cursorcl;
uchar ansiattr;
int sizeLTentry;

char grpZap = FALSE, hallZap = FALSE, msgZap = FALSE, logZap = FALSE, roomZap = FALSE;

char roomBuild = FALSE,     	/* Rebuild room-file?           */
hallBuild = FALSE;      		/* Rebuild hall-file?           */

char gprompt[80];				/* save area for last prompt	*/


/* -------------------------------------------------------------------------
 *
 *	Definitions for all but MISC2 module.
 *
 * ------------------------------------------------------------------------- */

#else
	
extern char extrncmd[MAXEXTERN];/* External command letters     */
extern struct ext_prot *extrn;  /* External protocols buffer    */
extern struct ext_command *extCmd;  /* External commands buffer     */
extern struct rTable *roomTab;  /* RAM index of rooms           */
extern struct sgl_term gl_term;	/* Global terminal flags		*/
extern struct sgl_user gl_user;	/* Global user flags/values		*/
extern struct sgl_str gl_str;	/* Global strings (saves RAM)	*/
//extern struct sgl_attr gl_attr;	/* Global screen attributes		*/

/*==== Function pointers ====*/
extern void (*charattr) (char, uchar);
extern void (*stringattr) (uint, char *, uchar);

/*==== OTHER ====*/
extern label default_group; 	/* Default for net messages */
extern char specialTime;    	/* Is it a "special" hour           */
extern float startbalance;  	/* Number of credits at log-in      */
extern long lasttime;       	/* timestamp of last update         */
extern char readMessage;
extern char ExitToMsdos;    	/* bring system down         */
extern char sysReq;     		/* SysOp wants system        */
extern char *confirm;
extern char mailFlag;       	/* Sending private mail?        */
extern char oldFlag;        	/* Entering old message?        */
extern char limitFlag;      	/* Entering Limited-Access      */
extern char chatReq;        	/* Did they want to chat?       */
extern char dowhat;     		/* What is calling iChar        */
extern int xpd, duplic;
extern char duplicate, expired; /* count duplicates and expired */
extern char netError;       	/* save net error message?      */
extern unsigned char thisHall;  /* hall we're in                */
extern int thisLog;     		/* entry currently in logBuf    */
extern int thisRoom;        	/* Current room                 */
extern char etcpath[64];    	/* Path etc.dat and ctdl.exe in */
extern int thisSlot;        	/* logTab slot logBuf found via */
extern char loggedIn;       	/* Global have-caller flag      */
extern char prevChar;       	/* for EOLN/EOParagraph stuff   */
extern int people;      		/* this is the old slot         */
extern char sysopNew;
extern long uptimestamp;
extern long logtimestamp;
extern long conntimestamp;
extern ulong transmitted;   	/* how many characters transmitted */
extern ulong received;      	/* how many characters transmitted */
extern char backout;        	/* Words out backward?       */
extern char justLostCarrier;    /* Just lost it        */
extern char newCarrier;     	/* Just got carrier         */
extern char onConsole;      	/* Who's in control?!?           */
extern unsigned char modStat;   /* Had carrier LAST time you     */
								/* you checked?                  */
								
extern char whichIO;        	/* CONSOLE or MODEM            */
extern char haveCarrier;    	/* set if DCD == TRUE            */
extern char echo;       		/* Either NEITHER, CALLER, or BOTH */
extern char echoChar;       	/* What to echo with if echo == NEITHER */
extern char anyEcho;
extern char sysopkey;
extern char chatkey;
extern char eventkey;
extern char disabled;       	/* is modem disabled?   */
extern int getkey;
extern char speed;
extern char detectflag;     	/* got carrier while on console */
extern unsigned char filt_in[]; /* translate input      */
extern unsigned char filt_out[];/* translate output     */
extern char modem;
extern char printing;
extern char console;        	/* output flag for console */
extern char ConLock;
extern char debug;
extern char callout;
extern unsigned char crtColumn; /* current position on screen  */
extern char outFlag;        	/* will be one of the above    */
extern char dotoMessage;
extern ulong markedMId;     	/* id of message to be marked  */

extern ulong originalId;    	/* this is the ID of origin      */
extern char copyflag;       	/* true when viewing a duplicate */
extern char MessageRoom[MAXROOMS];  /* number of message/room this call */
extern char seen;
extern int bauds[];
extern char showdir, showhidden, showgroup;
extern char skiproom;
extern int oldroom, ug_lvisit, ug_new;
extern uchar scrollpos;     	/* adjusts for help menu         */
extern uchar ansiattr;

extern char heldMessage;
extern char replyFlag;      	/* Sending a reply?            */
extern char linkMess;       	/* Sending a file-linked message? */
extern int mread, entered;  	/* #messages read and entered  */

extern unsigned char originalattr;  /* Attribute byte of origin      */

extern char grpZap, hallZap, msgZap, logZap, roomZap;

extern char roomBuild,      	/* Rebuild room-file?           */
hallBuild;          			/* Rebuild hall-file?           */

extern char gprompt[80];		/* save area for last prompt	*/


#endif
/* -------------------------------------------------------------------------
 *
 *	External declarations for all files.
 *
 * ------------------------------------------------------------------------- */

extern char *softname;
extern char *version;
extern const char compdate[];
extern const char comptime[];
extern char *welcome[];
extern char *copyright[];
extern unsigned int copycrcs[];
extern unsigned int welcomecrcs[];

/* End of Global.h */
