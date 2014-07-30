/* -------------------------------------------------------------------- */
/*  PROTO.H  -  Function Prototypes for Citadel.                        */
/* -------------------------------------------------------------------- */

/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/PROTO.H_V  $
 * 
 *    Rev 1.49   01 Nov 1991 11:21:16   FJM
 * Added gl_ structures
 *
 *    Rev 1.48   21 Sep 1991 10:19:42   FJM
 * FredCit release
 *
 *    Rev 1.47   15 Jun 1991  8:45:34   FJM
 *
 *    Rev 1.46   06 Jun 1991  9:20:06   FJM
 *
 *    Rev 1.45   29 May 1991 11:21:18   FJM
 * new port.c macros
 *
 *    Rev 1.44   27 May 1991 11:43:38   FJM
 *    Rev 1.43   22 May 1991  2:18:06   FJM
 *    Rev 1.42   18 May 1991  9:32:52   FJM
 *    Rev 1.41   16 May 1991  8:44:32   FJM
 *    Rev 1.39   19 Apr 1991 23:28:16   FJM
 *    Rev 1.31   05 Feb 1991 14:33:28   FJM
 *    Rev 1.30   28 Jan 1991 13:13:44   FJM
 *    Rev 1.28   19 Jan 1991 14:18:00   FJM
 *    Rev 1.25   18 Jan 1991 16:53:12   FJM
 *    Rev 1.21   11 Jan 1991 12:44:10   FJM
 *    Rev 1.20   06 Jan 1991 16:52:12   FJM
 *    Rev 1.17   06 Jan 1991 12:46:40   FJM
 *  Porting, formating, fixes and misc changes.
 *
 *   Rev 1.16   27 Dec 1990 20:17:22   FJM
 *  Changes for porting.
 *
 *   Rev 1.13   22 Dec 1990 13:50:56   FJM
 *   Rev 1.7   16 Dec 1990 18:13:58   FJM
 *   Rev 1.5   07 Dec 1990 23:10:50   FJM
 *   Rev 1.3   23 Nov 1990 13:25:40   FJM
 *  Header change
 *
 *     Rev 1.1   17 Nov 1990 16:12:32   FJM
 *  Added version control log header
 * --------------------------------------------------------------------
 *  Early History:
 *
 *  03/03/90    GREN    Add DRAGON.C, lets the user abuse routine work.
 *  03/07/90    {zm}    Add ismostlyupper() to EDIT.C
 *  03/13/90    FJM     Made oChar() take unsigned arg for filtering
 *                      Converted to use prototypes (was extern decl's)
 *  03/19/90    FJM     Linted & partial cleanup
 *  06/06/90    FJM     Changed strftime to cit_strftime
 *
 * -------------------------------------------------------------------- */

/* CTDL.C */
void main(int argc, char **argv);

/* COMMAND.C */
void doKnown(char moreYet, char first);
void doLogin(char moreYet);
void doLogout(char expand, char first);
void doNext(void);
void doNextHall(void);
void doPrevious(void);
void doPreviousHall(void);
char doRegular(char x, char c);
void doXpert(void);
void do_SysopGroup(void);
void do_SysopHall(void);
void exclude(void);
char getCommand(char *c);
void greeting(void);
unsigned char mAbort(void);
void doAd(int force);
void updatebalance(void);

/* DOAIDE.C */
void doAide(char moreYet, char first);

/* DOCHAT.C */
void chat(void);
void ringSysop(void);
void doChat(char moreYet, char first);

/* DOUPDOWN.C */
void doDownload(char ex);
void doUpload(char ex);

/* DOENTER.C */
void doEnter(char moreYet, char first);
void exclude(void);

/* DOGOTO.C */
void doGoto(char expand, int skip);

/* DOHELP.C */
void doHelp(char expand);

/* DOINTO.C */
void doIntro(void);

/* DOREAD.C */
void doRead(char moreYet, char first);

/* DOSYSOP.C */
char doSysop(void);

/* ACCOUNT.C */
void clearthisAccount(void);
void logincrement(void);
int logincheck(void);
void negotiate(void);
void newaccount(void);
void unlogthisAccount(void);
void calc_trans(long time1, long time2, char trans);

/* APLIC.C */
void readDoorFiles(int door);
void roomFmtRun(char *cmd);
void extFmtRun(char *cmd, char *file);
void ExeAplic(void);
void apsystem(char *stuff);
void shellescape(char super);

/* CONFIG.C */
int logSort(const void *s1, const void *s2);
int zapLogFile(void);
int zapMsgFile(void);
int zapRoomFile(void);
void RoomTabBld(void);
void buildcopies(void);
void buildhalls(void);
void buildroom(void);
void clearaccount(void);
void configcit(void);
void illegal(char *errorstring);
void initfiles(void);
void logInit(void);
void msgInit(void);
void readaccount(void);
void readconfig(int firstime);
void readprotocols(void);
void showtypemsg(unsigned long here);
void slidemsgTab(int howmany);
void zapGrpFile(void);
void zapHallFile(void);

/* CRON.C */
int cronNextEvent(int evnt);
int cando_event(int evnt);
void do_cron_loop(void);
int do_cron(int why_called);
void cron_commands(void);
void did_net(char *callnode);
void list_event(void);
void readCrontab(void);
void readcron(void);
void writeCrontab(void);

/* DOAIDE.C */
void doAide(char moreYet, char first);

/* EDIT.C */
BOOL fedit(char *filename);
BOOL editBuf (char *buf, int lim);
char *matchString(char *buf, char *pattern, char *bufEnd, char ver);
int editText(char *buf, int lim);
int ismostlyupper(char *s, int n);
unsigned char getText(void);
void fakeFullCase(char *text);
void putheader(void);
void replaceString(char *buf, int lim, char ver);
void wordcount(char *buf);
void xPutStr(FILE * fl, char *str);

/* FILES.C */
double dltime(long size);
int ambig(char *filename);
int ambigUnlink(char *filename, char change);
int checkfilename(char *filename, char xtype);
int checkup(char *filename);
int dump(char *filename);
int dumpf(char *filename);
void attributes(void);
void blocks(char *filename, int bsize);
void dir(char *filename, char verbose);
void download(char c);
void entertextfile(void);
void enterwc(void);
void entrycopy(int element, char verbose);
void entrymake(struct ffblk * file_buf);
void filldirectory(char *filename, char verbose);
void goodbye(void);
void hello(void);
void hide(char *filename);
void readdirectory(char verbose);
void readtextfile(void);
void readwc(void);
void renamefile(void);
void strlower(char *string);
void textdown(char *filename);
void textup(char *filename);
void tutorial(char *filename, int showhelp);
void unlinkfile(void);
void upDownMnu(char cmd);
void upload(char c);
void wcdown(char *filename);
void wcup(char *filename);
void wxrcv(char *path, char *file, char trans);
void wxsnd(char *path, char *file, char trans);
void next_hfile(char *name, int *count, int help, int ext);

/* FORMAT.C */
int parse_it(char **words, char *input);
int qtext(char *buf, char *line, char end);
int strpos(int ch, char *str);
int u_match(char *s, char *p);
void normalizeString(char *s);
void sformat(char *str, char *fmt, char *val,...);

/* GRPHALL.C */
int accesshall(int slot);
int groupexists(char *groupname);
int groupseeshall(int hallslot);
int groupseesroom(int groupslot);
int hallexists(char *hallname);
int ingroup(int groupslot);
int iswindow(int roomslot);
int partialgroup(char *groupname);
int partialhall(char *hallname);
int roominhall(int roomslot);
void cleargroupgen(void);
void enterhall(void);
void getgroup(void);
void gotodefaulthall(void);
void knownhalls(void);
void readhalls(void);
void setgroupgen(void);
void stephall(int direction);

/* INFO.C */
void entercomment(char *filename, char *uploader, char *comment);
void setfileinfo(void);

/* INFOFILE.C */
char *getdescname(char *filename);
void enterdesc(char *filename, int editit);
void showdesc(char *filename);
int batchinfo(int askuser);
void killinfo(char *filename);
void moveFile(void);
void readinfofile(char verbose);
void showinfo(char verbose);
void updateinfo(void);

/* INIT.C */
int readMsgTab(void);
void writeMsgTab(void);
void crashout(char *message);
void exitcitadel(void);
void initCitadel(void);
void asciitable(void);
int readTables(void);
void freeTables(void);
void allocateTables(void);
void writeTables(void);

/* INPUT.C */
int BBSCharReady(void);
int getYesNo(char *prompt, char dfault);
int iChar(void);
long getNumber(char *prompt, long bottom, long top, long dfaultnum);
void cGetString(char *buffer, int length, char *dfault, char *prompt);
void getASCString(char *prompt, char *buf, int lim, char QuestIsSpecial,
	char doEcho, char *dfault, char ASCIIOnly);
void getNormStr(char *prompt, char *s, int size, char doEcho);
void getString(char *prompt, char *buf, int lim, char QuestIsSpecial, char doEcho, char *dfault);
void setio(char whichio, char echo, char outflag);

/* LIBRARY.C */
void getGroup(void);
void getHall(void);
void getLog(struct logBuffer * lBuf, int n);
void getRoom(int rm);
void putGroup(void);
void putHall(void);
void putLog(struct logBuffer * lBuf, int n);
void putRoom(int rm);

/* LOG.C */
int findPerson(char *name, struct logBuffer * lbuf);
int personexists(char *name);
int pwexists(char *pw);
void displaypw(char *name, char *in, char *pw);
void normalizepw(char *InitPw, char *Initials, char *passWord, char *semicolon);
void setdefaultconfig(void);
void setlogconfig(void);
void setsysconfig(void);
void showconfig(struct logBuffer * lbuf);
void slideLTab(int slot);
void storeLog(void);

/* LOG2.C */
int pwslot(char *in, char *pw);
void Readlog(char verbose);
void forwardaddr(void);
void killuser(void);
void login(char *initials, char *password);
void minibin(void);
void newPW(void);
void setalloldrooms(void);
void setlbvisit(void);
void setroomgen(void);
void showuser(void);
void terminate(char discon, char verbose);

/* LOG3.C */
void userEdit2(label who);
int newlog(char *fullnm, char *in, char *pw);
int newslot(void);
void configure(unsigned char new);
void initroomgen(void);
void newUser(char *initials, char *password);
void newUserFile(void);
void userEdit(void);

/* MACHINE.C */
int cit_herror_handler(int errval,int ax,int bp,int si);
int special_pressed(void);
long bytesfree(void);
unsigned char getattr(char far * filename);
unsigned long cit_time(void);
unsigned long cit_timer(void);
void cit_init_timer(void);
void setattr(char far * filename, unsigned char attr);

/* MISC.C */
int changedir(char *path);
unsigned char filexists(char *filename);
unsigned int hash(char *str);
void SaveAideMess(void);
void amPrintf(char *fmt,...);
void amZap(void);
void ctrl_c(void);
void h2memcpy(char huge * to, char huge * from, long size);
void hmemcpy(void huge * xto, void huge * xfrom, long size);
void openFile(char *filename, FILE * *fd);
void trap(char *string, int what);
void GetStr(FILE * fl, char *str, int mlen);
void PutStr(FILE * fl, char *str);
void stripANSI(char *s);
/*void changedisk(char disk);*/

/* MISC2.C */
void dial_out(void);
void logo(void);
void ringSystemREQ(void);
void systat(void);

/* MODEM.C */
int carrier(void);
int findbaud(void);
int getModStr(char *str);
unsigned char KBReady(void);
unsigned char checkCR(void);
void carrdetect(void);
void carrloss(void);
void doccr(void);
void domcr(void);
void fkey(void);
void offhook(void);
void outCon(char c);
void outstring(char *string);
void ringdetectbaud(void);
void verbosebaud(void);

/* MSG.C */
unsigned char putMessage(void);
void aideMessage(void);
void clearmsgbuf(void);
void getMessage(void);
void specialMessage(void);

/* MSG2.C */
int getMsgChar(void);
int indexslot(unsigned long msgno);
unsigned char dGetWord(char *dest, int lim);
unsigned char mayseeindexmsg(int slot);
unsigned char mayseemsg(void);
unsigned int sizetable(void);
void changeheader(unsigned long id, unsigned char roomno, unsigned char attr);
void copyindex(int dest, int source);
void crunchmsgTab(int howmany);
void dPrintf(char *fmt,...);
void getMsgStr(char *dest, int lim);
void indexmessage(unsigned long here);
void noteMessage(void);
void notelogmessage(char *name);
void overwrite(int bytes);
void putMsgChar(char c);
void putMsgStr(char *string);

/* MSG3.C */
unsigned char makeMessage(void);
unsigned char markIt(void);
unsigned char pullIt(void);
unsigned char stepMessage(unsigned long *at, int dir);
void copymessage(unsigned long id, unsigned char roomno);
void deleteMessage(void);
void insert(void);
void markmsg(void);
void printMessage(unsigned long id, char verbose);
void printheader(unsigned long id, char verbose, int slot);
void showMessages(char whichMess, char revOrder, char verbose);
void saveMessageText(char *filename);
int strblank(char *string);

/* NET.C */
int NfindRoom(char *str);
int ReadMsgFl(int room, char *filename, char *here, char *there);
unsigned char GetMessage(FILE * fl);
unsigned char alias(char *str);
unsigned char alias_route(char *str, char *srch);
unsigned char get_first_room(char *here, char *there);
unsigned char get_next_room(char *here, char *there);
unsigned char getnode(char *nodename);
unsigned char master(void);
unsigned char n_dial(void);
unsigned char n_login(void);
unsigned char net_callout(char *node);
unsigned char net_master(void);
unsigned char net_slave(void);
unsigned char netcanseeroom(int roomslot);
/* unsigned char readnode(void); */
unsigned char route(char *str);
unsigned char save_mail(void);
unsigned char slave(void);
unsigned char wait_for(char *str);
void NewRoom(int room, char *filename);
void fPutMessage(FILE * fl);
void cleanup(void);
void saveMessage(unsigned long id, FILE * fl);

/* OUTPUT.C */
int getWord(char *dest, char *source, int offset, int lim);
void cCPrintf(char *fmt,...);
void cPrintf(char *fmt,...);
void doBS(void);
void doCR(void);
void doTAB(void);
void dospCR(void);
void echocharacter(char c);
void mFormat(char *string);
void mPrintf(char *fmt,...);
void mpPrintf(char *fmt,...);
void mtPrintf(char attr, char *fmt,...);
void oChar(unsigned char c);
void putWord(char *st);
void termCap(char c);
void updcrtpos(char c);

/* PORT.C */
void portDump(void);
int MIReady(void);
int MOReady(void);
int getMod(void);
int gotCarrier(void);
int ringdetect(void);
void Hangup(void);
void Initport(void);
void baud(int baudrate);
void drop_dtr(void);
void outMod(unsigned char ch);
void portExit(void);
void portFlush(void);
void portInit(void);

#define MIReady()       (!com_rx_empty())
#define MOReady()   	(com_tx_ready())
#define getMod()		(received++,com_rx())
#define gotCarrier()    (com_carrier())
#define ringdetect()	(com_ring())

/* ROOMA.C */
int canseeroom(int roomslot);
int gotoRoom(char *roomname);
int partialExist(char *roomname);
int roomExists(char *room);
int substr(char *str1, char *str2);
void RoomStatus(void);
void dumpRoom(void);
void givePrompt(void);
void indexRooms(void);
void listRooms(unsigned int what, char verbose, char numMess);
void noteRoom(void);
void printrm(int room);
void printroomVer(int room, int verbose, char numMess);
void roomdescription(void);
void roomtalley(void);
void stepRoom(int direction);
void unGotoRoom(void);

/* ROOMB.C */
int findRoom(void);
int renameRoom(void);
void formatSummary(char *buffer);
void killempties(void);
void killroom(void);
void makeRoom(void);
void massdelete(void);
void readbymsgno(void);

/* IDLE.C */
void do_idle(int idleflag);

/* SYSOP1.C */
int xhallfunc(int roomslot, int xyn, int fl);
void defaulthall(void);
void force(void);
void globalgroup(void);
void globalhall(void);
void globaluser(void);
void globalverify(void);
void groupfunc(void);
void hallfunc(void);
void killgroup(void);
void killhall(void);
void listgroup(void);
void listhalls(void);
void newgroup(void);
void newhall(void);
void renamegroup(void);
void renamehall(void);
void sysopunlink(void);
void windowfunc(void);

/* TIMEDATE.C */
int dayofweek(void);
int hour(void);
void changeDate(void);
void cit_strftime(char *outstr, int maxsize, char *formatstr, long tnow);
void diffstamp(long oldtime);
void getdstamp(char *buffer, unsigned int stamp);
void gettstamp(char *buffer, unsigned int stamp);
void pause(int ptime);
void set_date(void);

/* VERSION.C */

/* VIDEO.C */
int gmode(void);
unsigned char screenheight(void);
unsigned char screenwidth(void);
void bioschar(char ch, unsigned char attr);
void biosstring(unsigned int row, char *str, unsigned char attr);
void clearline(unsigned int row, unsigned char attr);
void cls(void);
void cursoff(void);
void curson(void);
void directchar(char ch, unsigned char attr);
void directstring(unsigned int row, char *str, unsigned char attr);
void help(void);
void position(unsigned char row, unsigned char column);
void readpos(unsigned char *row, unsigned char *column);
void restore_screen(void);
void save_screen(void);
void scroll(unsigned char row, unsigned char howmany, unsigned char attr);
void setscreen(void);
void update25(void);
void updatehelp(void);
void wscroll(int left, int top, int right, int bottom, int lines, unsigned char attr);

/* WINDOW.C */
char ansi(char c);
void connectcls(void);

/* XMODEM.C */
unsigned int stringcrc(char *buff);
void rxfile(char *name);
void trans(char *in_name);

/* ZIPFILE.C */
char zipview(char *filename, char verbose);
void readzip(char verbose);

/* EOF */
