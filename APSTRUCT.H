/* -------------------------------------------------------------------- */
/*  APSTRUCT.H              Dragon Citadel                              */
/* -------------------------------------------------------------------- */
/*  This file contains the table to generate OUTPUT.APL and INPUT.APL   */
/* -------------------------------------------------------------------- */

/*  ------------------------------------------------------------------  */
/*  #defines                                                            */
/*  ------------------------------------------------------------------  */
#define TYP_CHAR    0
#define TYP_INT     1
#define TYP_STR     2
#define TYP_BOOL    3
#define TYP_FLOAT   4
#define TYP_LONG    5
#define TYP_OTHER   6
#define TYP_GRP     7
#define TYP_END     8

/*  ------------------------------------------------------------------  */
/*  Local variables                                                     */
/*  ------------------------------------------------------------------  */
struct ApplicTable {
    int item;
    void *variable;
    int type;
    int length;
    int keep;
};

struct ApplicTable AplTab[] =
{
    {APL_NAME, logBuf.lbname, TYP_STR, NAMESIZE, 0},
    {APL_SURNAME, logBuf.surname, TYP_STR, NAMESIZE, 1},
    {APL_TITLE, logBuf.title, TYP_STR, NAMESIZE, 1},
    {APL_SYSOP, &gl_user.sysop, TYP_BOOL, 0, 1},
    {APL_AIDE, &gl_user.aide, TYP_BOOL, 0, 1},
    {APL_TWIT, &gl_user.twit, TYP_BOOL, 0, 1},
    {APL_EXPERT, &gl_user.expert, TYP_BOOL, 0, 1},
    {APL_CREDITS, &logBuf.credits, TYP_FLOAT, 0, 1},
    {APL_PERMANENT, NULL, TYP_OTHER, 0, 1},
    {APL_NETUSER, NULL, TYP_OTHER, 0, 1},
    {APL_NOMAIL, NULL, TYP_OTHER, 0, 1},
    {APL_VERIFIED, NULL, TYP_OTHER, 0, 1},
    {APL_NULLS, &gl_term.termNulls, TYP_CHAR, 0, 1},
    {APL_LINEFEED, &gl_term.termLF, TYP_CHAR, 0, 1},
    {APL_UPPERCASE, &gl_term.termUpper, TYP_BOOL, 0, 1},
    {APL_ANSION, &gl_term.ansiOn, TYP_BOOL, 0, 1},
    {APL_IBM_GRAPHIC, &gl_term.IBMOn, TYP_BOOL, 0, 1},
    {APL_COLUMNS, &gl_term.termWidth, TYP_CHAR, 0, 1},
    {APL_LINES, &logBuf.linesScreen, TYP_CHAR, 0, 1},
    {APL_ULISTED, &gl_user.unlisted, TYP_BOOL, 0, 1},
    {APL_MDATA, &cfg.mdata, TYP_OTHER, 0, 0},
    {APL_BAUD, &speed, TYP_CHAR, 0, 0},
    {APL_NODE, cfg.nodeTitle, TYP_STR, NAMESIZE, 0},
    {APL_REGION, cfg.nodeRegion, TYP_STR, NAMESIZE, 0},
/* {APL_NODECOUNTRY, cfg.nodeCountry, TYP_STR, NAMESIZE, 0}, */
/* {APL_VERSION, version, TYP_STR, NAMESIZE, 0}, */
    {APL_HALL, NULL, TYP_OTHER, 0, 1},
    {APL_ROOM, NULL, TYP_OTHER, 0, 1},
    {APL_TEMPPATH, cfg.temppath, TYP_STR, 64, 0},
    {APL_TRANSPATH, cfg.transpath, TYP_STR, 64, 0},
    {APL_APPLPATH, cfg.aplpath, TYP_STR, 64, 0},
    {APL_HELPPATH, cfg.helppath, TYP_STR, 64, 0},
    {APL_HOMEPATH, cfg.homepath, TYP_STR, 64, 0},
    {APL_ROOMPATH, cfg.roompath, TYP_STR, 64, 0},
    {APL_MSGPATH, cfg.msgpath, TYP_STR, 64, 0},
    {APL_PRINTPATH, cfg.printer, TYP_STR, 64, 0},
    {APL_TRAPFILE, cfg.trapfile, TYP_STR, 64, 0},
    {APL_ACCOUNTING, NULL, TYP_OTHER, 0, 0},
    {APL_ATTR, &cfg.attr, TYP_CHAR, 0, 0},
    {APL_WATTR, &cfg.wattr, TYP_CHAR, 0, 0},
    {APL_CATTR, &cfg.cattr, TYP_CHAR, 0, 0},
    {APL_UTTR, &cfg.uttr, TYP_CHAR, 0, 0},
    {APL_BATTR, &cfg.battr, TYP_CHAR, 0, 0},
    {APL_TRANSMIT, &transmitted, TYP_LONG, 0, 1},
    {APL_RECEIVE, &received, TYP_LONG, 0, 1},
    {APL_SYSREQ, &sysReq, TYP_BOOL, 0, 1},
    {APL_CONLOCK, &cfg.f6pass, TYP_STR, NAMESIZE, 0},
    {APL_ECHO, &anyEcho, TYP_BOOL, 0, 1},
    {APL_CHAT, NULL, TYP_OTHER, 0, 1},
    {APL_BELLS, NULL, TYP_OTHER, 0, 1},
    {APL_PRINTING, &printing, TYP_BOOL, 0, 1},
    {GRP_ADD, NULL, TYP_GRP, 0, 1},
    {GRP_DEL, NULL, TYP_GRP, 0, 1},
    {APL_END, NULL, TYP_END, 0, 1}
};

/* EOF */
