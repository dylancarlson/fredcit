/* -------------------------------------------------------------------- */
/*  TIMEDATE.C                Citadel                                   */
/* -------------------------------------------------------------------- */
/*  This file contains functions relating to the time and date          */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Includes                                                            */
/* -------------------------------------------------------------------- */
#include <string.h>
#include <time.h>
#include "ctdl.h"

#ifndef ATARI_ST
#include <dos.h>
#endif

#include "proto.h"
#include "global.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  changedate()    Menu-level change date rutine                       */
/*  dayofweek()     returns current day of week 0 - 6  sun = 0          */
/*  diffstamp()     display the diffrence from timestamp to present     */
/*  getdstamp()     get datestamp (text format)                         */
/*  gettstamp()     get timestamp (text format)                         */
/*  hour()          returns hour of day  0 - 23                         */
/*  set_date()      Gets the date from the user and sets it             */
/*  to_roman()      Roman numeral conversion                            */
/*  cit_strftime()  formats a custom time and date string using formats */
/* -------------------------------------------------------------------- */


/* --------------------------------------------------------------------
 *  Version control log:
 * $Log:   D:/VCS/FCIT/TIMEDATE.C_V  $
 * 
 *    Rev 1.43   01 Nov 1991 11:20:58   FJM
 * Added gl_ structures
 *
 *    Rev 1.42   21 Sep 1991 10:19:28   FJM
 * FredCit release
 *
 *    Rev 1.41   16 May 1991  8:43:32   FJM
 * Moved systimeout() to input.c, it's only used there.
 *
 *    Rev 1.29   19 Jan 1991 14:16:46   FJM
 *    Rev 1.24   13 Jan 1991  0:32:44   FJM
 * Name overflow fixes.
 *
 *    Rev 1.18   06 Jan 1991 12:44:58   FJM
 * Porting, formating, fixes and misc changes.
 *
 *    Rev 1.17   27 Dec 1990 20:15:54   FJM
 * Changes for porting.
 *
 *    Rev 1.14   22 Dec 1990 13:37:38   FJM
 *
 *    Rev 1.6   24 Nov 1990  3:07:48   FJM
 * Changes for shell/door mode.
 *
 *    Rev 1.5   23 Nov 1990 22:14:16   FJM
 * Modifed systimeout() to look for parm.door state.
 *
 *    Rev 1.4   23 Nov 1990 13:25:30   FJM
 * Header change
 *
 *    Rev 1.2   17 Nov 1990 16:12:14   FJM
 * Added version control log header
 * -------------------------------------------------------------------- */

/* --------------------------------------------------------------------
 *  HISTORY:
 *
 *  05/26/89    (PAT)   Created from MISC.C to break that module into
 *                      more managable and logical pieces. Also draws
 *                      off MODEM.C
 *
 *  03/03/90    {zm}    added to_roman() for the year, at least.
 *  06/06/90    FJM     Changed strftime to cit_strftime
 *  11/04/90    FJM     Forced getdstamp() to insert a date (Jan 1, 1980)
 *                      when no real date read.
 *
 * -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  External data                                                       */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Static data/functions                                               */
/* -------------------------------------------------------------------- */
static char *monthTab[12] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
static char *fullmnts[12] = {
	"January", "February", "March", "April",
    "May", "June", "July", "August",
	"September", "October", "November", "December"};
	static char *days[7] = {"Sun", "Mon", "Tue", "Wed",
	"Thur", "Fri", "Sat"
};
static char *fulldays[7] = {
	"Sunday", "Monday", "Tuesday", "Wednesday",
	"Thursday", "Friday", "Saturday"
};

static int pows[4][4] =
{
    {1000, 1000, 1000, 1000},
    {900, 500, 400, 100},
    {90, 50, 40, 10},
    {9, 5, 4, 1}
};

static char *roms[4][4] =
{
    {"M", "M", "M", "M"},
    {"CM", "D", "CD", "C"},
    {"XC", "L", "XL", "X"},
    {"IX", "V", "IV", "I"}
};

/* -------------------------------------------------------------------- */
/*  changedate()    Menu-level change date routine                      */
/* -------------------------------------------------------------------- */
void changeDate(void)
{
    char dtstr[20];

    doCR();

    cit_strftime(dtstr, 19, "%x", 0l);
    mPrintf(" Current date is now: %s", dtstr);

    doCR();

    cit_strftime(dtstr, 19, "%X", 0l);
    mPrintf(" Current time is now: %s", dtstr);

    doCR();

    if (!getYesNo(" Enter a new date & time", 0))
        return;

    set_date();
}

/* -------------------------------------------------------------------- */
/*  dayofweek()     returns current day of week 0 - 6  sun = 0          */
/* -------------------------------------------------------------------- */
int dayofweek(void)
{
    long stamp;
    struct tm *timestruct;

    stamp = cit_time();

    timestruct = localtime(&stamp);

    return (timestruct->tm_wday);
}

/* -------------------------------------------------------------------- */
/*  diffstamp()     display the diffrence from timestamp to present     */
/* -------------------------------------------------------------------- */
void diffstamp(long oldtime)
{
    long diff;
    char tstr[10], dstr[10];
    int days;

    diff = cit_time() - oldtime;

    diff += 315561600L;

    cit_strftime(dstr, 9, "%j", diff);
    cit_strftime(tstr, 9, "%X", diff);

    /* convert it to integer */
    days = atoi(dstr) - 1;

    if (days)
        mPrintf(" %d %s ", days, days == 1 ? "day" : "days");

    mPrintf(" %s", tstr);
}

/* -------------------------------------------------------------------- */
/*  getdstamp()     get datestamp (text format)                         */
/* -------------------------------------------------------------------- */
void getdstamp(char *buffer, unsigned stamp)
{
    int day, year, mth;

    char month[4];

    year = ((stamp >> 9) & 0x7f) + 80;
    mth = (stamp >> 5) & 0x0f;
    day = stamp & 0x1f;

    if (mth < 1 || mth > 12 || day < 1 || day > 31) {
        strcpy(buffer, "80Jan01");
        return;
    }
    strcpy(month, monthTab[mth - 1]);

    sprintf(buffer, "%d%s%02d", year, month, day);
}

/* -------------------------------------------------------------------- */
/*  gettstamp()     get timestamp (text format)                         */
/* -------------------------------------------------------------------- */
void gettstamp(char *buffer, unsigned stamp)
{
    int hours, minutes, seconds;

    hours = (stamp >> 11) & 0x1f;
    minutes = (stamp >> 5) & 0x3f;
    seconds = (stamp & 0x1f) * 2;

    sprintf(buffer, "%02d:%02d:%02d", hours, minutes, seconds);
}

/* -------------------------------------------------------------------- */
/*  hour()          returns hour of day  0 - 23                         */
/* -------------------------------------------------------------------- */
int hour(void)
{
    long stamp;
    struct tm *timestruct;

    stamp = cit_time();

    timestruct = localtime(&stamp);

    return (timestruct->tm_hour);
}

/* -------------------------------------------------------------------- */
/*  set_date()       Gets the date from the user and sets it             */
/* -------------------------------------------------------------------- */
void set_date(void)
{
    char dtstr[20];
    struct time tim;
    struct date dat;

    getdate(&dat);
    dat.da_year = (uint) getNumber("Year", 1985L, 1999L, (long) dat.da_year);
    dat.da_mon = (uchar) getNumber("Month", 1L, 12L, (long) dat.da_mon);
    dat.da_day = (uchar) getNumber("Day", 1L, 31L, (long) dat.da_day);
    setdate(&dat);

    gettime(&tim);
    tim.ti_hour = (uchar) getNumber("Hour", 0L, 23L, tim.ti_hour);
    tim.ti_min = (uchar) getNumber("Minute", 0L, 59L, tim.ti_min);
    tim.ti_sec = (uchar) getNumber("Second", 0L, 59L, tim.ti_sec);
    tim.ti_hund = 0;
    settime(&tim);

    doCR();

    cit_strftime(dtstr, 19, "%x", 0l);
    mPrintf(" Current date is now: %s", dtstr);

    doCR();

    cit_strftime(dtstr, 19, "%X", 0l);
    mPrintf(" Current time is now: %s", dtstr);

    doCR();
}

/* -------------------------------------------------------------------- */
/*  to_roman()      Roman numeral conversion                            */
/*  (by anonymous, 9-27-87 6:01pm, via C' Hackers Forum, Seattle.)      */
/* -------------------------------------------------------------------- */
void to_roman(int decimal, char *roman)
{
    int power;          /* Current power of 10  */
    int indx;           /* Indexes through values to subtract  */

    roman[0] = '\0';
    for (power = 0; power < 4; power++) {
        for (indx = 0; indx < 4; indx++) {
            while (decimal >= pows[power][indx]) {
                strcat(roman, roms[power][indx]);
                decimal -= pows[power][indx];
            }
        }
    }
}

extern char *tzname[2];
extern int daylight;

/* -------------------------------------------------------------------- */
/*  cit_strftime()      formats a custom time and date string using formats */
/* -------------------------------------------------------------------- */
void cit_strftime(char *outstr, int maxsize, char *formatstr, long tnow)
{
    int i, k;
    char temp[30];
    struct tm *tmnow;

    if (tnow == 0l)
        tnow = cit_time();

    tmnow = localtime(&tnow);

    outstr[0] = '\0';

    for (i = 0; formatstr[i]; i++) {
        if (formatstr[i] != '%')
            sprintf(temp, "%c", formatstr[i]);
        else {
            i++;
            temp[0] = '\0';
            if (formatstr[i])
            switch (formatstr[i]) {
                case 'a':   /* %a  abbreviated weekday name                      */
                    sprintf(temp, "%s", days[tmnow->tm_wday]);
                    break;
                case 'A':   /* %A  full weekday name                            */
                    sprintf(temp, "%s", fulldays[tmnow->tm_wday]);
                    break;
                case 'b':   /* %b  abbreviated month name                       */
                    sprintf(temp, "%s", monthTab[tmnow->tm_mon]);
                    break;
                case 'B':   /* %B  full month name                              */
                    sprintf(temp, "%s", fullmnts[tmnow->tm_mon]);
                    break;
                case 'c':   /* %c  standard date and time string                */
                    sprintf(temp, "%s", ctime(&tnow));
                    temp[strlen(temp) - 1] = '\0';
                    break;
                case 'd':   /* %d  day-of-month as decimal (1-31)               */
                    sprintf(temp, "%d", tmnow->tm_mday);
                    break;
                case 'D':   /* %D  day-of-month as decimal (01-31)              */
                    sprintf(temp, "%02d", tmnow->tm_mday);
                    break;
                case 'H':   /* %H  hour, range (0-23)                           */
                    sprintf(temp, "%d", tmnow->tm_hour);
                    break;
                case 'I':   /* %I  hour, range (1-12)                           */
                    if (tmnow->tm_hour)
                        sprintf(temp, "%d", tmnow->tm_hour > 12 ?
                        tmnow->tm_hour - 12 :
                        tmnow->tm_hour);
                    else
                        sprintf(temp, "%d", 12);
                    break;
                case 'j':   /* %j  day-of-year as a decimal (1-366)             */
                    sprintf(temp, "%d", tmnow->tm_yday + 1);
                    break;
                case 'm':   /* %m  month as decimal (1-12)                      */
                    sprintf(temp, "%d", tmnow->tm_mon + 1);
                    break;
                case 'M':   /* %M  minute as decimal (0-59)                     */
                    sprintf(temp, "%02d", tmnow->tm_min);
                    break;
                case 'p':   /* %p  locale's equivaent af AM or PM               */
                    sprintf(temp, "%s", tmnow->tm_hour > 11 ? "PM" : "AM");
                    break;
                case 'R':   /* %R  year in Roman numerals with century          */
                    to_roman(tmnow->tm_year + 1900, temp);
                    break;
                case 'S':   /* %S  second as decimal (0-59)                     */
                    sprintf(temp, "%02d", tmnow->tm_sec);
                    break;
                case 'U':   /* %U  week-of-year, Sunday being first day (0-52)  */
                    k = tmnow->tm_wday - (tmnow->tm_yday % 7);
                    if (k < 0)
                        k += 7;
                    if (k != 0) {
                        k = tmnow->tm_yday - (7 - k);
                        if (k < 0)
                            k = 0;
                    } else
                        k = tmnow->tm_yday;
                    sprintf(temp, "%d", k / 7);
                    break;
                case 'W':   /* %W  week-of-year, Monday being first day (0-52)  */
                    k = tmnow->tm_wday - (tmnow->tm_yday % 7);
                    if (k < 0)
                        k += 7;
                    if (k != 1) {
                        if (k == 0)
                            k = 7;
                        k = tmnow->tm_yday - (8 - k);
                        if (k < 0)
                            k = 0;
                    } else
                        k = tmnow->tm_yday;
                    sprintf(temp, "%d", k / 7);
                    break;
                case 'w':   /* %w  weekday as a decimal (0-6, sunday being 0)   */
                    sprintf(temp, "%d", tmnow->tm_wday);
                    break;
                case 'x':   /* %x  standard date string                         */
                    sprintf(temp, "%02d/%02d/%d", tmnow->tm_mon + 1,
                    tmnow->tm_mday,
                    tmnow->tm_year);
                    break;
                case 'X':   /* %X  standard time string                         */
                    sprintf(temp, "%02d:%02d:%02d", tmnow->tm_hour,
                    tmnow->tm_min,
                    tmnow->tm_sec);
                    break;
                case 'y':   /* %y  year in decimal without century (00-99)      */
                    sprintf(temp, "%02d", tmnow->tm_year);
                    break;
                case 'Y':   /* %Y  year including century as decimal            */
                    if (tmnow->tm_year > 99) {
                        tmnow->tm_year -= 100;
                        sprintf(temp, "20%02d", tmnow->tm_year);
                    } else
                        sprintf(temp, "19%02d", tmnow->tm_year);
                    break;
                case 'Z':   /* %Z  timezone name                                */
                    tzset();
                    sprintf(temp, "%s", tzname[daylight]);
                    break;
                case '%':   /* %%  the percent sign                             */
                    sprintf(temp, "%%");
                    break;
                default:
                    temp[0] = '\0';
                    break;
            }       /* end of switch */

        }           /* end of if */

        if ((strlen(temp) + strlen(outstr)) > maxsize)
            break;
        else if (strlen(temp))
            strcat(outstr, temp);

    }               /* end of for loop */
}

/* EOF */
