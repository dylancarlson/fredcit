/************************************************************************/
/*                              account2.c                              */
/*        time accounting code for Citadel bulletin board system        */
/************************************************************************/

#include <time.h>
#include "ctdl.h"
#include "proto.h"
#include "global.h"

/************************************************************************
 *                              Contents                                *
 *                                                                      *
 *      clearthisAccount()      initializes all current user group data *
 *      logincrement()          increments balance according to data    *
 *      logincheck()            logs-out a user if can't log in now     *
 *      negotiate()             determines lowest cost, highest time    *
 *      newaccount()            sets up accounting balance for new user *
 *      unlogthisAccount()      NULL group accounting to thisAccount    *
 *      calc_trans()            adjust time after a transfer            *
 ************************************************************************
 *
 *      03/19/90    FJM     Linted & partial cleanup
 *                      Changed float (unsresolved data type) to double
 *      05/19/90    FJM     Cleaned negotiate(), and nade it negotiate upload
 *                                              and download multipliers.
 *
 ************************************************************************/

/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/ACCOUNT.C_V  $
 * 
 *    Rev 1.43   01 Nov 1991 11:19:40   FJM
 * Added gl_ structures
 *
 *    Rev 1.42   27 May 1991 11:40:32   FJM
 *
 *    Rev 1.38   04 Apr 1991 14:11:40   FJM
 * Made accountBuf dynamicly allocated.
 *
 *    Rev 1.19   06 Jan 1991 12:44:38   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.18   27 Dec 1990 20:15:52   FJM
 * Changes for porting.
 *
 *    Rev 1.15   22 Dec 1990 13:37:36   FJM
 *    Rev 1.8   07 Dec 1990 23:10:56   FJM
 *    Rev 1.6   24 Nov 1990  3:07:58   FJM
 * Changes for shell/door mode.
 *
 *    Rev 1.5   23 Nov 1990 13:25:46   FJM
 * Header change
 *
 *    Rev 1.3   18 Nov 1990 19:31:20   FJM
 *
 *    Rev 1.2   17 Nov 1990 16:11:28   FJM
 * Added version control log header
 * -------------------------------------------------------------------- */


/************************************************************************
 *      clearthisAccount()  initializes all current user group data     *
 ************************************************************************/
void clearthisAccount(void)
{
    int i;

    /* init days */
    for (i = 0; i < 7; i++)
        thisAccount.days[i] = 0;

    /* init hours & special hours */
    for (i = 0; i < 24; i++) {
        thisAccount.hours[i] = 0;
        thisAccount.special[i] = 0;
    }

    thisAccount.have_acc = FALSE;
    thisAccount.dayinc = 0.;
    thisAccount.maxbal = 0.;
    thisAccount.dlmult = -1.;   /* charge full time */
    thisAccount.ulmult = 1.;    /* credit full time (xtra)  */
}

/************************************************************************/
/*      logincrement()          increments balance according to data    */
/*                              give em' more minutes if new day!       */
/************************************************************************/
void logincrement(void)
{
    long diff, timestamp;
    long day = 86400L;
    double numdays, numcredits;

    timestamp = cit_time();

    diff = timestamp - logBuf.calltime;
    /* how many days since last call(1st of day) */
    numdays = (double) ((double) diff / (double) day);

    if (numdays < 0.0) {    /* date was wrong..             */
        mPrintf("[Date correction made.]");
        numdays = 1;        /* give em something for sysops mistake..   */
        lasttime = cit_time();
        logBuf.calltime = lasttime;
    }
    numcredits = numdays * thisAccount.dayinc;

    /* If they have negative minutes this will bring them up MAYBE  */
    logBuf.credits = logBuf.credits + numcredits;

    if (logBuf.credits > thisAccount.maxbal)
        logBuf.credits = thisAccount.maxbal;

    /* Credits/Minutes to start with.   */
    startbalance = logBuf.credits;

    lasttime = cit_time();  /* Now, this is the last time we were on.   */
}


/************************************************************************/
/*      logincheck()            logs-out a user if can't log in now     */
/************************************************************************/
int logincheck(void)
{
    if (thisAccount.special[hour()]) {  /* Is is free time?     */
        specialTime = TRUE;
    } else {
        specialTime = FALSE;
    }

    /* Local and bad calls get no accounting.   */
    if (!gotCarrier() || onConsole)
        return (TRUE);

    if (!thisAccount.days[dayofweek()]
        || !thisAccount.hours[hour()]
    || logBuf.credits <= 0.0) {
        nextblurb("nologin", &(cfg.cnt.nlogcount), 1);

        loggedIn = FALSE;

        terminate(TRUE, FALSE);

        return (FALSE);
    }
    if (thisAccount.special[hour()]) {  /* Is is free time?     */
        specialTime = TRUE;
    }
    return (TRUE);
}

/************************************************************************/
/*      negotiate()  determines lowest cost, highest time for user      */
/************************************************************************/
void negotiate(void)
{
    int groupslot, i;
    int firstime = TRUE;
    double priority = 0.0;
    int topslot = 0;

    clearthisAccount();

    for (groupslot = 0; groupslot < MAXGROUPS; groupslot++) {
        if (ingroup(groupslot)
        && accountBuf->group[groupslot].account.have_acc) {
        /* is in a group with accounting */
            thisAccount.have_acc = TRUE;

            if (accountBuf->group[groupslot].account.priority >= priority) {
                topslot = groupslot;
                if (accountBuf->group[groupslot].account.priority > priority) {  /********************************/
                    priority = accountBuf->group[groupslot].account.priority;
                    firstime = TRUE;
                }       /************************/
            }           /* if  */
            if (accountBuf->group[topslot].account.dlmult > thisAccount.dlmult
                || firstime)
                thisAccount.dlmult =    /* these are */
                accountBuf->group[topslot].account.dlmult;   /* special   */
        /* */
            if (accountBuf->group[topslot].account.ulmult > thisAccount.dlmult
                || firstime)
                thisAccount.ulmult =    /* */
                accountBuf->group[topslot].account.ulmult;   /*-----------*/

            if (accountBuf->group[groupslot].account.dayinc >
                thisAccount.dayinc || firstime)
                thisAccount.dayinc
                = accountBuf->group[groupslot].account.dayinc;

            if (accountBuf->group[groupslot].account.maxbal >
            thisAccount.maxbal || firstime) {
                firstime = FALSE;
                thisAccount.maxbal
                = accountBuf->group[groupslot].account.maxbal;
            }
        }           /* if  */
    }               /* for  */

    for (i = 0; i < 7; ++i)
        if (accountBuf->group[topslot].account.days[i])
            thisAccount.days[i] = 1;

    for (i = 0; i < 24; ++i) {
        if (accountBuf->group[topslot].account.hours[i])
            thisAccount.hours[i] = 1;

        if (accountBuf->group[topslot].account.special[i])
            thisAccount.special[i] = 1;
    }

}

/************************************************************************/
/*      newaccount()  sets up accounting balance for new user           */
/*      extra set-up stuff for new user                                 */
/************************************************************************/
void newaccount(void)
{

    logBuf.credits = cfg.newbal;
    lasttime = cit_time();
    logBuf.calltime = lasttime;

}

/************************************************************************/
/*      unlogthisAccount()  sets up NULL group accounting to thisAccount*/
/************************************************************************/
void unlogthisAccount(void)
{
    int i;

    /* set up unlogged balance */
    logBuf.credits = cfg.unlogbal;

    /* reset transmitted & received */
    transmitted = 0L;
    received = 0L;

    lasttime = cit_time();
    logBuf.calltime = lasttime;

    /* init days */
    for (i = 0; i < 7; i++)
        thisAccount.days[i] = accountBuf->group[0].account.days[i];

    /* init hours & special hours */
    for (i = 0; i < 24; i++) {
        thisAccount.hours[i] = accountBuf->group[0].account.hours[i];
        thisAccount.special[i] = accountBuf->group[0].account.special[i];
    }

    thisAccount.dayinc = accountBuf->group[0].account.dayinc;
    thisAccount.maxbal = accountBuf->group[0].account.maxbal;
    thisAccount.dlmult = accountBuf->group[0].account.dlmult;
    thisAccount.ulmult = accountBuf->group[0].account.ulmult;

}

/************************************************************************/
/*      calc_trans()        Calculate and adjust time after a transfer  */
/*                          (trans == 1) ? Upload : Download            */
/************************************************************************/
void calc_trans(long time1, long time2, char trans)
{
    long diff;          /* # secs trans took            */
    double credcost;        /* their final credit/cost      */
    double c;           /* # minutes transfer took      */
    double change;      /* how much we change their bal */
    double mult;        /* the multiplyer for trans     */
    char neg;           /* is credcost negative?        */

    if (trans)
        mult = thisAccount.ulmult;
    else
        mult = thisAccount.dlmult;

    diff = time2 - time1;   /* how many secs did ul take                 */

    c = (double) diff / 60.0;   /* how many minutes did it take  */

    change = c * mult;      /* take care of multiplyer...    */

    logBuf.credits = logBuf.credits + change;

    credcost = change;

    if (credcost < 0.0) {
        neg = TRUE;
    } else {
        credcost = credcost + c;/* Make less confusion, add time used */
    }               /* when telling them about credit     */

    mPrintf(" Your transfer took %.2f %s. You have been %s %.2f %s%s.",
    c,
    (c == 1) ? "minute" : "minutes",
    (neg == 1) ? "charged" : "credited",
    (neg == 1) ? (-credcost) : credcost,
    cfg.credit_name,
    (credcost == 1) ? "" : "s");

    lasttime=cit_time();        /* don't charge twice...    */
}
