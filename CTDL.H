/* -------------------------------------------------------------------- */
/*                              Ctdl.h                                  */
/*               #include file for all Citadel C files.                 */
/*              Now includes only #defines and structs.                 */
/* -------------------------------------------------------------------- */

/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/CTDL.H_V  $
 *
 *    Rev 1.42   01 Nov 1991 11:21:08   FJM
 * Added gl_ structures
 *
 *    Rev 1.41   29 May 1991 11:21:00   FJM
 * Removed redundent includes.
 *
 *    Rev 1.40   27 May 1991 11:43:30   FJM
 *
 *    Rev 1.39   22 May 1991  2:18:00   FJM
 *    Rev 1.35   04 Apr 1991 14:14:28   FJM
 *   Rev 1.29   05 Feb 1991 14:32:50   FJM
 *New cfg.screen_save.
 *
 *   Rev 1.28   28 Jan 1991 13:13:32   FJM
 *Added SUCCESS define.
 *
 *   Rev 1.23   18 Jan 1991 16:52:44   FJM
 *   Rev 1.21   13 Jan 1991  0:33:04   FJM
 *Enlarged e_str to prevent overflow.
 *
 *   Rev 1.19   11 Jan 1991 12:44:04   FJM
 *   Rev 1.18   06 Jan 1991 16:51:48   FJM
 *Added login to parm struct.
 *
 *   Rev 1.15   06 Jan 1991 12:46:36   FJM
 *Porting, formating, fixes and misc changes.
 *
 *   Rev 1.14   27 Dec 1990 20:17:16   FJM
 *Changes for porting.
 *
 *     Rev 1.5   09 Dec 1990 15:24:48   FJM
 *  Added color codes for screen & remote.
 *
 *     Rev 1.3   23 Nov 1990 13:25:36   FJM
 *  Header change
 *
 *     Rev 1.1   17 Nov 1990 16:12:30   FJM
 *  Added version control log header
 *
 * --------------------------------------------------------------------
 *
 *  EARLY HISTORY:
 *
 *  03/01/90  {zm}   Add anonymous rooms.
 *  03/15/90  {zm}   Add [title] name [surname] everywhere @ 30 chars.
 *  03/16/90  {zm}   Add route path and other new DragCit msg fields.
 *  03/19/90  FJM    Linted & partial cleanup
 *  04/01/90  FJM    Removed unused variables
 *  05/10/90  FJM    Fixed FileInfo.  Requires convert.
 *  09/21/90  FJM    Added #SYSOP, #CREDIT_NAME
 *  10/15/90  FJM    Added console/net only protocol flag to ext_prot
 *  12/20/90  JDW    Added #define ATARI_ST for porting...
 *
 * -------------------------------------------------------------------- */

#include "machine.h"

/*
** For reference the ANSI header files are:
**  assert.h
**  ctype.h
**  errno.h
**  float.h
**  limits.h
**  locale.h
**  math.h
**  setjmp.h
**  signal.h
**  stdarg.h
**  stddef.h
**  stdio.h
**  stdlib.h
**  string.h
**  time.h
**
** On the other hand, things like dos.h, bios.h, alloc.h etc.
** are not ANSI headers and reduce the portability of each module
** they appear in a great deal...
**
**      (jdw/gan)
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#ifndef ATARI_ST
#include <dir.h>
#endif

#define EOS     ((char)0)           /* End-Of-String define */
#undef toupper
#undef tolower

/* macro to translate setmem calls to the equivalent memset call */
#define setmem(p, n, c) memset(p, c, n)

typedef unsigned long  ulong ;   /* Makes it easier to format            */
typedef unsigned short ushort;   /* Makes it easier to format            */
typedef unsigned int   uint  ;   /* Makes it easier to format            */
typedef unsigned char  uchar ;   /* Makes it easier to format            */
typedef unsigned char  BOOL  ;   /* Makes it easier to format            */

#define FILESIZE       20        /* length of file names                 */
#define NAMESIZE       30        /* length of room names                 */

typedef char label[31];          /* must be NAMESIZE + 1                 */

/* -------------------------------------------------------------------- */
/*                      Stuff to size system with:                      */
/* -------------------------------------------------------------------- */

#define MAXROOMS       250      /* number of rooms allowed in system    */
#define MAXHALLS       64       /* number of halls allowed in system    */
#define MAXCRON        30       /* number of events in cron.cit         */
#define MAXEXTERN      30       /* number of externs in protocol.cit    */

#define MAXGROUPS      64       /* number of groups allowed in system   */
#define MAXTEXT        8192     /* cheating  (message buffer size)      */

#define MAXDIRS        6        /* number of directions                 */


/* --------------------------------------------------------------------
 *
 * Global attribute values
 *
 * -------------------------------------------------------------------- */

/* Need to change the way these vars work for this;
	cfg.attr
	cfg.wattr
	cfg.cattr
	cfg.battr
	cfg.uttr
*/
//struct sgl_attr {
//	uchar attr;
//	uchar wattr;
//	uchar cattr;
//	uchar battr;
//	uchar uttr;
//};

/* --------------------------------------------------------------------
 *
 * Global strings
 *
 * -------------------------------------------------------------------- */

struct sgl_str {
	char *yes;
	char *no;
	char *on;
	char *off;
};

/* --------------------------------------------------------------------
 *
 * Global user flags
 *
 * -------------------------------------------------------------------- */

struct sgl_user {
	char expert;           		/* true to suppress hints etc.  */
	char aide;         			/* aide privileges flag         */
	char sysop;        			/* sysop privileges flag        */
	char twit;         			/* Problem user?                */
	char unlisted;         		/* List in userlog?             */
	char oldToo;           		/* Send last old on new request? */
	char roomtell;         		/* Print room descriptions?     */
};

/* --------------------------------------------------------------------
 *
 * Global terminal flags
 *
 * -------------------------------------------------------------------- */

struct sgl_term {
	char termUpper;        		/* uppercase only flag          */
	char termLF;          		/* LF-after-CR flag             */
	char termTab;           	/* Tabs                         */
	uchar termWidth;    		/* width to format output to    */
	uchar termNulls;    		/* # nulls to send at EOLN      */
	char ansiOn;				/* ANSI terminal flag			*/
	char IBMOn;					/* IBM Graphics characters flag	*/
};								/* Global terminal flags		*/

/* --------------------------------------------------------------------
 *
 * counters definitions
 *
 * -------------------------------------------------------------------- */

#define nextblurb(s,c,h) (next_hfile((s),(c),(h),1))
#define nextmenu(s,c,h) (next_hfile((s),(c),(h),2))
#define nexthelp(s,c,h) (next_hfile((s),(c),(h),3))

struct scounters {
	/* blurbs */
	int acount;
	int bcount;
	int ccount;
	int chatcnt;
	int chatnum;
	int count;
	int ecount;
	int gcount;
	int newroomcount;
	int nlogcount;
	int nosop;
	int pcount;
	int shelpcount;
	int sleepcount;
	int sleepycount;
	int tcount;
	int ucount;
	int vcount;
	int wcdcount;
	int wcucount;
	int downcount;
	int upcount;
	int whichBye;
	int whichHello;
	/* tutorials */
	int aidetut;
	int ansitut;
	int dohelptut;
	int edittut;
	int entopttut;
	int fileNametut;
	int helpopttut;
	int introtut;
	int knowntut;
	int mainopttut;
	int readopttut;
	int roomedittut;
	int sysoptut;
	
};
				                /* This is part of the contents of Etc.dat              */
struct config
{
    char compdate[20];
    char comptime[20];
    unsigned messagek;          /* how many K message table is..        */
    int      maxtext;           /* # of characters per message          */

    int maxfiles;               /* Max number of files per directory    */

    ulong callno;               /* how many calls the system has gotten */

    ulong oldest;               /* 32-bit ID# of first message in system*/
    ulong newest;               /* 32-bit ID# of last  message in system*/

    ulong mtoldest;             /* oldest message ID # in message table */

    label nodeTitle;            /* name of the system                   */

    label nodeRegion;           /* name of the system's reagon          */

    label f6pass;

    char  datestamp[64];         /* time date format string              */
    char  vdatestamp[64];        /* time date format string (verbose)    */

    char  modsetup[64];          /* Normal init string                   */
    label offhooksetup[20];
    char  dialsetup[64];         /* Dial out string                      */
    label dialpref;

    int   idle;                  /* how long we wait before we net       */
	int   screen_save;			 /* delay before the screen saver starts */
    int   mdata;                 /* 3f8 com 1,  2f8 com 2                */

    char  dumbmodem;
    char  initbaud;

    char homepath[64];          /* which path system files will be in   */
    char msgpath[64];           /* which path message file will be in   */
    char helppath[64];          /* path which help files live in        */
    char temppath[64];          /* temporary files (networking) go here */
    char roompath[64];          /* where to look for room descriptions  */
    char trapfile[64];          /* where trap file goes                 */
    char aplpath[64];           /* which path applications will be in   */
    char transpath[64];         /* path transient files will be in      */
    char printer[64];           /* where to put output at alt-p         */
    char sysop[64];             /* sysop's name                         */
    char credit_name[64];       /* what to call credit's (e.g. gold pieces) */
	char swapfile[64];			/* name of swap file					*/

    char newuserapp[13];        /* application to run for new users     */

    uchar width;                /* Default unlogged screen width        */
    uchar nulls;                /* Default unlogged # nulls             */

    char unlogtimeout;          /* how many idle mins unlogged user gets*/
    char timeout;               /* how many idle mins logged user gets  */

    int  oldcount;              /* how many message new to new users    */

    uchar attr;                 /* color of text displayed on screen    */
    uchar wattr;                /* color of text displayed on window    */
    uchar cattr;                /* color of text displayed on window    */
    uchar battr;                /* color of text screen border lines    */
    uchar uttr;                 /* color of text displayed in underline */

    long catLoc;                /* where to begin writing next message  */

    int MAXLOGTAB;              /* number of log entries supported      */
    int nmessages;              /* # of messages to be stored in table  */

    float cost1200;             /* Amount to multiply costs by at 1200  */
    float cost2400;             /* Amount to multiply costs by at 2400  */

    char maxwarn;               /* # times to warn user before hangup   */

    float unlogbal  ;           /* # credits unlogged users get         */
    float newbal    ;           /* # credits new users get              */
    float maxbalance;           /* maximum amount of credits user gets  */

    char trapit[16];            /* which events are logged              */
    char user[5];               /* #user default status                 */

    uchar MessageRoom;          /* max messages per room per call       */

    uchar nopwecho;             /* what to echo initials & pw at login  */

    char  extcmd[20];           /* command character list               */

    unsigned ad_chance;                    /* chance 0-100% of an ad appearing             */

    unsigned ad_time;                      /* time before random ad appears                */

    unsigned subhubs       : 2;   /* provision for special g)oto looping  */

    unsigned kill          : 1;   /* TRUE if user can kill own messages   */

    unsigned offhook       : 1;   /* TRUE to go off-hook when 'L' is hit  */

    unsigned bios          : 1;   /* TRUE to use BIOS calls for screen I/O*/

    unsigned noChat        : 1;   /* TRUE to suppress chat attempts       */
    unsigned noBells       : 1;   /* TRUE to supress all bells            */

    unsigned roomtell      : 1;   /* display room descriptions?           */

    unsigned loginstats    : 1;   /* display log-in statistics?           */

    unsigned moderate      : 1;   /* can aides see moderated messages?    */

    unsigned unlogEnterOk  : 1;   /* TRUE if OK to enter messages anon    */
    unsigned unlogReadOk   : 1;   /* TRUE if unlogged folks can read mess */
    unsigned nonAideRoomOk : 1;   /* TRUE general folks can make rooms    */

    unsigned uppercase     : 1;   /* Default is upper case?               */
    unsigned linefeeds     : 1;   /* Default linefeeds given?             */
    unsigned tabs          : 1;   /* Default do we have tabs?             */
    unsigned forcelogin    : 1;   /* Automatically log someone in?        */

    unsigned accounting    : 1;   /* is accounting enabled on the system? */

    unsigned surnames      : 1;   /* are the surenames on?                */

    unsigned aidehall      : 1;   /* Aides mess with halls.               */

    unsigned readluser     : 1;   /* Read Limited Access Userlog?         */
    unsigned readuser      : 1;   /* Read Userlog?                        */
    unsigned aidereaduser  : 1;   /* Aides Read Userlog?                  */
    unsigned entersur      : 1;   /* Let users enter their own surname?   */
    unsigned netsurname    : 1;   /* Display networked surnames?          */

    unsigned private       : 4;   /* Private type                         */

    unsigned dummy         : 2;    /* unused fields                       */

    unsigned pace;                 /* pacing rate                         */
	struct scounters cnt;
    char lastExtern;
};


/* -------------------------------------------------------------------- */
/*                           Misc. #defines                             */
/* -------------------------------------------------------------------- */
/* values for functions to return: */

#define TRUE            1
#define FALSE           0
#define ERROR          -1
#define SUCCESS			0
#define SAMESTRING      0       /* value for strcmp() & friend          */


/* -------------------------------------------------------------------- */
/*                              Room data                               */
/* -------------------------------------------------------------------- */
#define LOBBY           0       /* Lobby> is >always< room 0.           */
#define MAILROOM        1       /* Mail>  is >always< room 1.           */
#define AIDEROOM        2       /* Aide)  is >always< room 2.           */
#define DUMP            3       /* Dump>  is >always< room 3.           */
#define DUNGEON         4       /* ....>  is >always< room 4.           */

#define MAXGEN        128       /* six bits of generation => 64 of them */

#define OLDNEW          0       /* List both unread and read rooms      */
#define NEWRMS          1       /* Only list unread rooms.              */
#define WINDWS          2       /* List rooms which are windows         */
#define DIRRMS          4       /* Directory rooms..                    */
#define MAILRM          5       /* Mail rooms......                     */
#define APLRMS          6       /* Aplication rooms                     */
#define STATRM          7       /* Status of Room                       */
#define LIMRMS          8       /* Limited Access Rooms                 */
#define OLDRMS          9       /* Old Rooms                            */
#define XCLRMS          10      /* Xcluded Rooms                        */
#define SHRDRM          11      /* Shared rooms                         */
#define ANONRM                 12              /* Anonymous rooms                                              */


struct parms   /* all defaults are 0   */
{
	char *login;				/* user login string					*/
	char *user;					/* user login name						*/
    unsigned long baud;         /* initial baud rate                    */
    unsigned pace;              /* pacing delay on modem transmit       */
    unsigned dv          : 1;   /* TRUE to use BIOS calls for screen I/O*/
    unsigned door        : 1;   /* run as door from another BBS         */
    unsigned ems         : 1;   /* use EMS for overlays                 */
    unsigned ext         : 1;   /* use extended memory for overlays     */
    unsigned events      : 1;   /* just run cron events                 */
	unsigned memcheck    : 1;   /* do farheapcheck() calls				*/
    unsigned unused      : 10;  /* extras                               */
};

struct rflags                   /* Room flags                           */
{
    unsigned INUSE    : 1;      /* Room in use?                         */
    unsigned PUBLIC   : 1;      /* Room public?                         */
    unsigned MSDOSDIR : 1;      /* Room directory?                      */
    unsigned PERMROOM : 1;      /* Room permanent?                      */
    unsigned GROUPONLY: 1;      /* Room Owned by a group?               */
    unsigned READONLY : 1;      /* Room Read only?                      */
    unsigned DOWNONLY : 1;      /* Room download only?                  */
    unsigned SHARED   : 1;      /* Room Shared?                         */
    unsigned MODERATED: 1;      /* Room Moderated?                      */
    unsigned APLIC    : 1;      /* Room have application?               */

/*
 * new as of v3.10.00 DragCit
 */
    unsigned BIO      : 1;      /* By invintation only..                */
    unsigned UPONLY   : 1;      /* Upload only                          */
    unsigned WINDOW   : 1;      /* Window into other nav system         */
    unsigned ANONYMOUS: 1;      /* Room Anonymous?                      */
    unsigned UNUSED   : 2;      /* extra space, formerly 3 bits..       */
} ;

struct rTable                   /* The summation of a room              */
{
    uchar         rtgen;        /* generation # of room                 */
    uchar         grpno;        /* group # which owns it                */
    uchar         grpgen;       /* generation # of group which owns it  */
    struct rflags rtflags;      /* public/private flag etc              */
    label         rtname;       /* name of room                         */
} ;                             /* And see ROOMA.C for declaration      */

struct aRoom                        /* The appearance of a room:            */
{
    uchar           rbgen;          /* generation # of room                 */
    struct rflags   rbflags;        /* same bits as flags above             */
    label           rbname;         /* name of room                         */
    char            rbdirname[64];  /* user directory for this room's files */
    char            rbroomtell[13]; /* filename for room description        */
    char            rbaplic[13];    /* filename for room description        */
    uchar           rbgrpgen;       /* group gen# for Limited Access rooms  */
    uchar           rbgrpno;        /* group # which owns it                */
    int             rbmodhash;      /* moderator's hash, not used yet       */
    uchar           reserved[6];    /* six whole chars for anything!!!!     */

    /*
     * new as of v3.10.00 DragCit
     */
    char            descript[80];   /* for .kv[...]                         */
    char            netID[20];      /* Network ID for room, not used yet.   */
};

/* -------------------------------------------------------------------- */
/*                      group & accounting stuff                        */
/* -------------------------------------------------------------------- */
#define MAXGROUPGEN 128

struct accounting
{
    char days[7];        /* Which Days this group can log-in        */
    char hours[24];      /* Which hours of the day group can log-in */
    char special[24];    /* Which hours are special                 */
    float dayinc;        /* Maximum time add per day                */
    float priority;      /* What is the priority for this group     */
    float maxbal;        /* This groups maxtime on the system       */
    float dlmult;        /* Multiplyer for Downloads                */
    float ulmult;        /* Multiplyer for Uploads                  */

    unsigned have_acc: 1;/* Does group/user have accounting?        */
};

struct accountBuffer	/* 4800 bytes */
{
    struct
    {
        struct accounting account;
    } group[MAXGROUPS];
};

/* -------------------------------------------------------------------- */
/*                          group stuff                                 */
/* -------------------------------------------------------------------- */
struct groupBuffer					/* 7296 bytes */
{
    struct group_buffer				/* 114 bytes	*/
    {
        label    groupname  ;      /* Name of the group                 */
        uchar    groupgen   ;      /* Group generation #.               */
        char     desc[80]   ;      /* Description of group              */
        unsigned g_inuse : 1;      /* Is group inuse?                   */
        unsigned lockout : 1;      /* Sysop access needed for changes   */
        unsigned hidden  : 1;      /* hidden to all, unless specified
                                      by name or in group               */
        unsigned UNUSED  :13;
    } group[MAXGROUPS];
} ;

/* -------------------------------------------------------------------- */
/*                             hall stuff                               */
/* -------------------------------------------------------------------- */
struct hallBuffer					/* 35072 bytes */
{
    struct hall_buffer
    {
        label    hallname;           /* Name of our hall                */
        uchar    grpno;              /* owning group's #                */
        unsigned h_inuse    : 1;     /* Is hall inuse?                  */
        unsigned owned      : 1;     /* Is this hall owned?             */
        unsigned described  : 1;     /* described?                      */
        unsigned dmaint     : 1;     /* default mainenacne              */
        unsigned UNUSED     :12;
        struct
        {
            unsigned inhall : 1;     /* is room in this hall?           */
            unsigned window : 1;     /* is room a window into hallway?  */
            unsigned UNUSED :14;
        } hroomflags[MAXROOMS];
        uchar    mdefault;           /* hall default mainencance hall   */
        char     htell[13];          /* Hall description                */
    } hall[MAXHALLS];
} ;

/* -------------------------------------------------------------------- */
/*                            userlog stuff                             */
/* -------------------------------------------------------------------- */
struct lflags                   /* Flags for person in log              */
{
    unsigned L_INUSE   : 1;     /* Is this slot in use?                 */
    unsigned UCMASK    : 1;     /* Uppercase?                           */
    unsigned LFMASK    : 1;     /* Linefeeds?                           */
    unsigned EXPERT    : 1;     /* Expert?                              */
    unsigned AIDE      : 1;     /* Aide?                                */
    unsigned TABS      : 1;     /* Tabs?                                */
    unsigned OLDTOO    : 1;     /* Print out last oldmessage on  N>ew?  */
    unsigned PROBLEM   : 1;     /* Twit bit                             */
    unsigned UNLISTED  : 1;     /* List in Userlog?                     */
    unsigned PERMANENT : 1;     /* Permanent log entry?                 */
    unsigned SYSOP     : 1;     /* Sysop special access.                */
    unsigned NODE      : 1;     /* Are they a node?                     */
    unsigned NETUSER   : 1;     /* Can they send network E-mail         */
    unsigned NOACCOUNT : 1;     /* Accounting disabled for this person? */
    unsigned NOMAIL    : 1;     /* Can user send exclusive mail?        */
    unsigned ROOMTELL  : 1;     /* Room-descriptions?                   */
} ;

#define MAXVISIT       16       /* #visits we remember old newestLo for */

struct lbroomdata
{
    uchar    lbgen;             /* which rooms are visible?             */
    unsigned xclude : 1;        /* is room excluded?                    */
    unsigned mail   : 1;        /* private mail in this room?           */
    unsigned lvisit : 5;        /* lastvisit pointer 1 - 16             */
    unsigned UNUSED : 8;
};

struct logBuffer                /* The appearance of a user:            */
{
    label         lbname;       /* caller's name                        */
    label         lbin;         /* caller's initials                    */
    label         lbpw;         /* caller's password                    */
    label         forward;      /* forwarding address                   */
    /* the following were reversed for the sake of compatibility.       */
    label         surname;      /* user's surname                       */
    label         title;        /* user's title                         */
    int           hallhash;     /* default hallway hash                 */
    uchar         lbnulls;      /* #Nulls, lCase, lFeeds                */
    uchar         lbwidth;      /* terminal width                       */
    long          calltime;     /* Time/date stamp of last call         */
    ulong         callno;       /* Which caller # they were             */
    float         credits;      /* Credits for accounting               */
    uchar     groups[MAXGROUPS];/* Group generation # 16 of them     */
    struct lbroomdata lbroom[MAXROOMS]; /* Data for each room in LogBuf */
    ulong     lbvisit[MAXVISIT];/* newestLo for this and 15 prev.visits */

/*
 * new as of v3.10.00 DragCit
 */
    uchar    lastRoom;          /* room they were in last call          */
    uchar    lastHall;          /* hall they were in last call          */

    uchar    linesScreen;       /* the number of lines a caller has per */
                                /* screen */
    char     tty[10];           /* type of TTY                          */

    struct lflags lbflags;      /* UCMASK, LFMASK, EXPERT, AIDE, INUSE  */
    unsigned DUNGEONED : 1;     /* dungeoned user?                      */
    unsigned MSGAIDE   : 1;     /* message only aide?                   */
    unsigned FORtOnODE : 1;     /* forward to node?                     */
    unsigned NEXTHALL  : 1;     /* Auto Next Hallway?                   */
    unsigned OTHERNAV  : 2;     /* other nav type                       */
    unsigned VERIFIED  : 1;     /* has the user been verified?          */
    unsigned SURNAMLOK : 1;     /* surname locked?                      */
        /* new for version 13 FredCit  */
    unsigned IBMGRAPH  : 1;     /* IBM Graphics characters ?            */
    unsigned UNUSED    : 15;    /* unused remaining bits                */
    unsigned UNUSED2   : 8;     /* unused byte                          */
};

struct lTable                   /* Summation of a person:               */
{
    int   ltpwhash;             /* hash of password                     */
    int   ltinhash;             /* hash of initials                     */
    int   ltnmhash;             /* hash of name                         */
    int   ltlogSlot;            /* location in userlog.buf              */
    ulong ltcallno;             /* Which caller # they were             */
    char  permanent;            /* permanent log entry.                 */
};

/* -------------------------------------------------------------------- */
/*                      message stuff                                   */
/* -------------------------------------------------------------------- */
#define HELD           3

#define ATTR_RECEIVED  1
#define ATTR_REPLY     2
#define ATTR_MADEVIS   4

struct msgflags                 /* Message attribute flags              */
{                               /* included in msg buffer * msg table   */
    unsigned MAIL     : 1;
    unsigned RECEIVED : 1;      /* Was it received?                     */
    unsigned REPLY    : 1;      /* Reply sent?                          */
    unsigned PROBLEM  : 1;      /* Problem User message?                */
    unsigned MADEVIS  : 1;      /* Made Visible?                        */
    unsigned LIMITED  : 1;      /* Limited-access message?              */
    unsigned MODERATED: 1;      /* Moderated message?                   */
    unsigned RELEASED : 1;      /* Released by moderator?               */
    unsigned COPY     : 1;      /* Is message a copy?                   */
    unsigned NET      : 1;      /* Networked mail en-route?             */
    unsigned UNUSED   : 6;
};

struct msgB                     /* This is what a msg looks like        */
{
    char mbtext[MAXTEXT];       /* buffer text is edited in             */

    long  mbheadLoc;            /* Start of message                     */
    uchar mbroomno;             /* room # of message                    */
    uchar mbattr;               /* 3 attribute bits                     */

    label mbauth ;              /* name of author                       */
    label mbcopy ;              /* message ID of copy this msg is of    */
    label mbfwd  ;              /* forwarding address                   */
    label mbgroup;              /* group name for limited access msg    */
    label mbtime ;              /* creation time                        */
    label mbId   ;              /* local number of message              */
    label mboname;              /* name of origin system                */
    label mboreg ;              /*  "        "    region                */
    label mbocont;              /*  "        "    country               */
    label mbreply;              /* message ID of message replied to     */
    label mbroom ;              /* creation room                        */
    label mbsrcId;              /* message ID on system of origin       */
    label mbto   ;              /* private message to                   */
    label mbx    ;              /* twit/moderated message header        */
    label mbtitle;              /* title                                */
    label mbsur  ;              /* surname                              */
    char  mblink[64];           /* file linked message                  */
    label mbzip  ;              /* name of destination system           */
    label mbrzip ;              /*   "          "      region           */
    label mbczip ;              /*   "          "      country          */
    char  mbfpath[129];         /* where did the message come from?     */
    char  mbtpath[129];         /* forced routing via path              */
} ;                             /* declaration in MSG.C                 */

/* The following is the message filter table.. */

struct mfilter
{
    unsigned mfMai   : 1;
    unsigned mfLim   : 1;
    unsigned mfPub   : 1;
    label    mfUser;
    label    mfGroup;
} ;

/* values for showMess routine */
#define NEWoNLY         0
#define OLDaNDnEW       1
#define OLDoNLY         2

/* -------------------------------------------------------------------- */
/*                      message table stuff                             */
/* -------------------------------------------------------------------- */
struct messagetable
{
    struct msgflags mtmsgflags; /* every message gets flags             */
    long     mtmsgLoc;          /* offset where message starts          */
    ushort   mtoffset;          /* msg# - mtoffset == actual msg no     */
                                /* to determine if copy has scrolled    */
    uchar    mtroomno;          /* room # of message                    */
    int      mttohash;          /* hash of recipient or group           */
    int      mtauthhash;        /* hash of author of message            */
    int      mtfwdhash;         /* hash of forwardee                    */
    long     mtomesg;           /* this and the next are to check for   */
                                /* duplicate messages                   */
    int      mtorigin;          /* hash of the origin system's name     */
};

/* -------------------------------------------------------------------- */
/*          TalleyBuffer Stuff (message counts for each room)           */
/* -------------------------------------------------------------------- */
struct talleyBuffer {
    struct {
        ushort total;           /* total # message in room (sysops & aides) */
        ushort messages;        /* # message user can read */
        ushort new;             /* # new message user can read */
    } room[MAXROOMS];
};

/* -------------------------------------------------------------------- */
/*                       node stuff                                     */
/* -------------------------------------------------------------------- */
struct nodest
{
    int   ndbaud;
    int   nddialto;
    int   ndredial;
    int   ndwaitto;
    int   ndexpire;

    label ndname;
    label ndregion;
    label ndprotocol;
    label ndphone;
    label ndmailtmp;

    struct {
        label here;
        label there;
    }ndgroups[MAXGROUPS];

    ulong roomoff;

    char  ndlogin[90];
};


/* -------------------------------------------------------------------- */
/*                      modem stuff                                     */
/* -------------------------------------------------------------------- */
/* fiddle factor to timeout on no input: */
#define HITIMEOUT       10000   /* processor dependent, see MODEM.C     */

#define NEWCARRIER   0x01       /* returned to main prog on login       */

#define MODEM           0       /* current user of system is            */
#define CONSOLE         1       /* one of these                         */

/*  output XON/XOFF etc flag... */
#define OUTOK           0       /* normal output                        */
#define OUTPAUSE        1       /* a pause has been requested           */
#define OUTNEXT         2       /* quit this message, get the next      */
#define OUTSKIP         3       /* stop current process                 */
#define OUTPARAGRAPH    4       /* skip to next paragraph               */
#define IMPERVIOUS      5       /* make current output unstoppable      */

#define NEITHER         0       /* don't echo input at all              */
#define CALLER          1       /* echo to caller only --passwords etc  */
#define BOTH            2       /* echo to caller and console both      */

#define NO_ECHO         0       /* Echo input as .'s                    */
#define ECHO            1       /* Echo input   (getString())           */


/* -------------------------------------------------------------------- */
/*                           Trap File Stuff                            */
/* -------------------------------------------------------------------- */
#define T_ALL        0             /* All events are logged             */
#define T_CARRIER    1             /* carrier detect & loss are logged  */
#define T_LOGIN      2             /* login, logout, new users          */
#define T_NEWROOM    3             /* new room creations are logged     */
#define T_PROBLEM    4             /* problem user messages are logged  */
#define T_CHAT       5             /* chat requests are logged          */
#define T_PASSWORD   6             /* password changes are logged       */
#define T_AIDE       7             /* aide functions are logged         */
#define T_SYSOP      8             /* sysop functions are logged        */
#define T_UPLOAD     9             /* uploads are logged                */
#define T_DOWNLOAD   10            /* all downloads are logged          */
#define T_ERROR      11            /* internal errors are logged        */
#define T_NETWORK    12            /* network events are logged         */
#define T_ACCOUNT    13            /* accounting information is logged  */
#define T_APPLIC     14            /* application executions are logged */

/* -------------------------------------------------------------------- */
/*                        Default User Stuff                            */
/* -------------------------------------------------------------------- */
#define D_PROBLEM     0            /* Everyone defaults to problem user */
#define D_PERMANENT   1            /* Everyone gets perm log entry      */
#define D_NOACCOUNT   2            /* Everyone has accounting disabled  */
#define D_NETWORK     3            /* Everyone's a network user         */
#define D_NOMAIL      4            /* Everyone sends no mail            */

/* -------------------------------------------------------------------- */
/*  External stuff                                                      */
/* -------------------------------------------------------------------- */
/*  External protical structure  */
struct ext_prot
{
    label ex_name;              /* protocol name, first char is command key */
    char  ex_rcv[40];           /* recieve command                  */
    char  ex_snd[40];           /* send command                     */
    char  ex_batch;             /* batch protocol flag              */
    int   ex_block;             /* block size, 0 for unknown/variable */
    int   ex_console;           /* console/net only protocol flag   */
};

/*  External editor structure  */
struct ext_editor
{
    label ed_name;
    char  ed_cmd[30];
    char  ed_local;
};

/* external commands structure - FJM */
struct ext_command
{
    label name;
    char command[40];
    char local;
};

/*  External other command structure  */
struct ext_other
{
    label    eo_name1;
    label    eo_name2;
    char     eo_cmd1;
    char     eo_cmd2;
    char     eo_cmd[40];
    unsigned DIR    :1;
    unsigned SYSOP  :1;
    unsigned AIDE   :1;
    unsigned CON    :1;
    unsigned UNUSED :12;
};

/* -------------------------------------------------------------------- */
/*                       cron stuff                                     */
/* -------------------------------------------------------------------- */
struct event
{
    char   e_type;       /* Event type (network, shell, ext)              */
    label  e_str;        /* nodename, shell command, ext                  */
    char   e_hours[24];  /* valid hours for event                         */
    char   e_days[7];    /* valid days for event                          */
    int    e_redo;       /* minutes before it will redo this event        */
    int    e_retry;      /* minutes before it will retry                  */

    long l_sucess;       /* last time it has sucsess                      */
    long l_try;          /* last time it tryed                            */
};

#define CRON_TIMEOUT   0   /* why do_cron() is called                   */
#define CRON_LOGOUT    1
#define CRON_PROMPT    2

/* -------------------------------------------------------------------- */
/* Infofile nonsence                                                    */
/* -------------------------------------------------------------------- */
struct fInfo                          /*  fileinfo data structure */
{
    char fn[13];
    label uploader;
    char comment[65];
};

/* -------------------------------------------------------------------- */
/*  File directory structure.                                           */
/* -------------------------------------------------------------------- */
struct fDir
{
    char entry[90];
};

/* -------------------------------------------------------------------- */
/*  What we are doing..                                                 */
/* -------------------------------------------------------------------- */
#define DUNO        0
#define MAINMENU    1
#define SYSOPMENU   2
#define PROMPT      3
#define MESSAGETEXT 4
#define DIALOUT     5
#define NETWORKING  6

/* -------------------------------------------------------------------- */
/*  TERMCAP definitions                                                 */
/* -------------------------------------------------------------------- */
#define TERM_NORMAL     '0'
#define TERM_BLINK      '1'
#define TERM_REVERSE    '2'
#define TERM_BOLD       '3'
#define TERM_UNDERLINE  '4'

/* foreground colors */
#define TERM_BLACK_FG  'a'
#define TERM_RED_FG            'b'
#define TERM_GREEN_FG  'c'
#define TERM_YELLOW_FG 'd'
#define TERM_BLUE_FG   'e'
#define TERM_MAGENTA_FG        'f'
#define TERM_CYAN_FG   'g'
#define TERM_WHITE_FG  'h'

/* light foreground colors - not used, reserved */
#define TERM_LBLACK_FG 'i'
#define TERM_LRED_FG   'j'
#define TERM_LGREEN_FG 'k'
#define TERM_LYELLOW_FG        'l'
#define TERM_LBLUE_FG  'm'
#define TERM_LMAGENTA_FG       'n'
#define TERM_LCYAN_FG  'o'
#define TERM_LWHITE_FG 'p'

/* background colors */
#define TERM_BLACK_BG  'A'
#define TERM_RED_BG            'B'
#define TERM_GREEN_BG  'C'
#define TERM_YELLOW_BG 'D'
#define TERM_BLUE_BG   'E'
#define TERM_MAGENTA_BG        'F'
#define TERM_CYAN_BG   'G'
#define TERM_WHITE_BG  'H'
/* -------------------------------------------------------------------- */
/*  These flags are for special action during message retrvial          */
/* -------------------------------------------------------------------- */
#define              NO_SPECIAL     0   /* dont do anything with message */
#define              PULL_IT        1   /* kill the message */
#define              MARK_IT        2   /* mark the message to be moved */
#define              REVERSE_READ   3   /* change read direction */
#define              COPY_IT        4   /* copy message to file */

/* -------------------------------------------------------------------- */
/*  Other includes                                                      */
/* -------------------------------------------------------------------- */
#include "applic.h"

/* EOF */
