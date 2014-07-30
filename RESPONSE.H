/* Hayes command set modem responses */

static char *hayes_string[] = {
	"OK",						/* 0 */
	"CONNECT",					/* 1 */
	"RING",						/* 2 */
	"NO CARRIER",				/* 3 */
	"ERROR",					/* 4 */
	"CONNECT 1200",				/* 5 */
	"NO DIALTONE",				/* 6 */
	"BUSY",						/* 7 */
	"NO ANSWER",				/* 8 */
	"UNKNOWN",					/* response 9 not used */
	"CONNECT 2400",				/* 10 */
	NULL
};

#define HAYES_OK		0
#define HAYES_CONNECT	1
#define HAYES_RING		2
#define HAYES_NOCARRIER	3
#define HAYES_ERROR		4
#define HAYES_CONNECT12	5
#define HAYES_NODIAL	6
#define HAYES_BUSY		7
#define HAYES_NOANSWER	8
#define HAYES_UNKNOWN	9
#define HAYES_CONNECT24	10
#define HAYES_NUMCMDS	11	/* one more then max command */
