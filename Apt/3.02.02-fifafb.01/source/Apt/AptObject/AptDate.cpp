/*H************************************************************************************************/
/*!

\File
AptDate.cpp

\Description
Simple description of main purpose of file.

\Notes
None.

\Copyright
(c) 2005 Electronic Arts Inc.

\Version    1.0        01/04/05 (MCB) First Version

*/
/************************************************************************************************H*/

#include "_Apt.h"
#include "MainInline.h"

#if defined (APT_USE_DATE_OBJECT)
#include "AptObject/AptDate.h"
#include "AptDateMembers.h"


static const int AptDate_getDate = 1;
static const int AptDate_getDay = 2;
static const int AptDate_getFullYear = 3;
static const int AptDate_getHours = 4;
static const int AptDate_getMilliseconds = 5;
static const int AptDate_getMinutes = 6;
static const int AptDate_getMonth = 7;
static const int AptDate_getSeconds = 8;
static const int AptDate_getTime = 9;
static const int AptDate_getTimezoneOffset = 10;
static const int AptDate_getUTCDate = 11;
static const int AptDate_getUTCDay = 12;
static const int AptDate_getUTCFullYear = 13;
static const int AptDate_getUTCHours = 14;
static const int AptDate_getUTCMilliseconds = 15;
static const int AptDate_getUTCMinutes = 16;
static const int AptDate_getUTCMonth = 17;
static const int AptDate_getUTCSeconds = 18;
static const int AptDate_getYear = 19;
static const int AptDate_setDate = 20;
static const int AptDate_setFullYear = 21;
static const int AptDate_setHours = 22;
static const int AptDate_setMilliseconds = 23;
static const int AptDate_setMinutes = 24;
static const int AptDate_setMonth = 25;
static const int AptDate_setSeconds = 26;
static const int AptDate_setTime = 27;
static const int AptDate_setUTCDate = 28;
static const int AptDate_setUTCFullYear = 29;
static const int AptDate_setUTCHours = 30;
static const int AptDate_setUTCMilliseconds = 31;
static const int AptDate_setUTCMinutes = 32;
static const int AptDate_setUTCMonth = 33;
static const int AptDate_setUTCSeconds = 34;
static const int AptDate_setYear = 35;
static const int AptDate_toString = 36;
static const int AptDate_UTC = 37;


#define SYS_SECONDS_PER_DAY     (24*60*60)

enum DateMonthE
{
    DATE_MONTH_INVALID   = -1,
    DATE_MONTH_FIRST     =  0,
    DATE_MONTH_JANUARY   =  0,
    DATE_MONTH_FEBRUARY  =  1,
    DATE_MONTH_MARCH     =  2,
    DATE_MONTH_APRIL     =  3,
    DATE_MONTH_MAY       =  4,
    DATE_MONTH_JUNE      =  5,
    DATE_MONTH_JULY      =  6,
    DATE_MONTH_AUGUST    =  7,
    DATE_MONTH_SEPTEMBER =  8,
    DATE_MONTH_OCTOBER   =  9,
    DATE_MONTH_NOVEMBER  = 10,
    DATE_MONTH_DECEMBER  = 11,
    DATE_MONTH_MAX       = 11,
};

bool AptDate::dateIsYearLeap(int nYear)
{
    bool bIsLeap = false;        // Default is that it is not a leap year.

    if( (nYear % 4) == 0)
    {
        // Every fourth year is a leap year.
        if( (nYear % 100) != 0)
        {
            bIsLeap = true;
        }
        else
        {
            // Exception: every 100 years is not a leap year
            if( (nYear % 400) != 0)
            {
                bIsLeap = false;
            }
            else
            {
                // Exception: every 400 years is a leap year
                bIsLeap = true;
            }
        }
    }
    return bIsLeap;
}

int AptDate::dateGetNumDaysInYear(int nYear)
{
    int nResult;

    if (dateIsYearLeap(nYear))
    {
        nResult = 366;
    }
    else
    {
        nResult = 365;
    }

    return nResult;
}

int AptDate::dateGetNumDaysInMonth(int nMonth, int nYear)
{
    int nNumDays;

    nNumDays = 31;

    switch(nMonth)
    {
        case DATE_MONTH_JANUARY:
        case DATE_MONTH_MARCH:
        case DATE_MONTH_MAY:
        case DATE_MONTH_JULY:
        case DATE_MONTH_AUGUST:
        case DATE_MONTH_OCTOBER:
        case DATE_MONTH_DECEMBER:
            nNumDays = 31;
            break;
        case DATE_MONTH_FEBRUARY:
            if(dateIsYearLeap(nYear))
            {
                nNumDays = 29;
            }
            else
            {
                nNumDays = 28;
            }
            break;
        case DATE_MONTH_APRIL:
        case DATE_MONTH_JUNE:
        case DATE_MONTH_SEPTEMBER:
        case DATE_MONTH_NOVEMBER:
            nNumDays = 30;
            break;
        default:
            APT_ASSERT(0);;   // Invalid month
            break;
    }
    return nNumDays;
}

void AptDate::setDates(AptSysClock *mTM1, AptSysClock *mTM2, int hourDiff)
{
    mTM2->Year = mTM1->Year;
    mTM2->Month = mTM1->Month;
    mTM2->Date = mTM1->Date;
    mTM2->Hour = mTM1->Hour - hourDiff;
    if((mTM1->Hour - hourDiff) > 23)
    {
        mTM2->Hour = mTM2->Hour % 24;
        int tmp = dateGetNumDaysInMonth(mTM2->Month, mTM2->Year);
        if(++(mTM2->Date) > tmp)
        {
            mTM2->Date = 1;
            if(++(mTM2->Month) > 11)
            {
                mTM2->Month = 0;
                mTM2->Year++;
            }
        }
    }
    else if((mTM1->Hour - hourDiff) < 0)
    {
        mTM2->Hour = 24 - hourDiff;
        if(--(mTM2->Date) < 1)
        {
            if(--(mTM2->Month) < 0)
            {
                mTM2->Month = 11;
                mTM2->Year--;
            }
            mTM2->Date = dateGetNumDaysInMonth(mTM2->Month, mTM2->Year);
        }
    }
    mTM2->Minute = mTM1->Minute;
    mTM2->Second = mTM1->Second;
    mTM2->Hundredths = mTM1->Hundredths;
}

int AptDate::getDayOfWeek(int year, int month, int day)
{
    int nTmp1 = 0, nTmp2 = 0;
    int nCentury = -1;
    int nWeekday;
    int nDoomsMonth = month + 1;
    int aCentury[] = {3, 2, 0, 5};          // Array of base DoomsDay dates for centuries
    int nYear = year / 100;

    nCentury = aCentury[nYear < 19 ? 4 - (abs(nYear - 19) % 4) : abs(nYear - 19) % 4];
    nYear = year - (year % 100);
    if(month == 1)
        nDoomsMonth = dateIsYearLeap(year) ? 29 : 28;
    else if(month % 2 == 0)
    {
        if(month == 8) nDoomsMonth = 5;
        else if(month == 4) nDoomsMonth = 9;
        else if(month == 6) nDoomsMonth = 11;
        else if(month == 10) nDoomsMonth = 7;
        else if(month == 2) nDoomsMonth = 7;
        else if(month == 0) nDoomsMonth = dateIsYearLeap(year) ? 32 : 31;
    }

    if( nCentury < 0 || nDoomsMonth < 0) return -1;
    if( nDoomsMonth > day )
        nWeekday = (7 - ((nDoomsMonth - day) % 7 ) + nDoomsMonth);
    else
        nWeekday = day;
    nTmp1 = (nWeekday - nDoomsMonth);
    nTmp1 %= 7;
    nTmp2 = nCentury + (year - nYear) + ((int)floorf(((float)(year - nYear)) / 4));
    nTmp2 %= 7;
    return (nTmp1 + nTmp2) % 7;
}

void AptDate::toString(AptNativeString &sDate)
{
    char days[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    char mon[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jly", "Aug", "Sep", "Oct", "Nov", "Dec"};

    char szTemp[4];

    sDate = days[getDayOfWeek(mTM.Year, mTM.Month, mTM.Date)];
    sDate += " ";
    sDate += mon[mTM.Month];
    sDate += " ";
    sprintf(szTemp, "%d", mTM.Date);
    sDate += szTemp;
    sDate += " ";
    sprintf(szTemp, "%02d", mTM.Hour);
    sDate += szTemp;
    sDate += ":";
    sprintf(szTemp, "%02d", mTM.Minute);
    sDate += szTemp;
    sDate += ":";
    sprintf(szTemp, "%02d", mTM.Second);
    sDate += szTemp;
    sDate += " GMT";
    sDate += hourDiff < 0 ? "-" : "+";
    sprintf(szTemp, "%02d", abs(hourDiff));
    sDate += szTemp;
    sDate += "00 ";
    sprintf(szTemp, "%d", mTM.Year);
    sDate += szTemp;
}

NATIVE_MEMBER_FUNCTION(AptDate, getDate) { return AptInteger::Create(pThis->c_date()->mTM.Date); }
NATIVE_MEMBER_FUNCTION(AptDate, getDay) { return AptInteger::Create(pThis->c_date()->getDayOfWeek(pThis->c_date()->mTM.Year, pThis->c_date()->mTM.Month, pThis->c_date()->mTM.Date)); }
NATIVE_MEMBER_FUNCTION(AptDate, getFullYear) { return AptInteger::Create(pThis->c_date()->mTM.Year); }
NATIVE_MEMBER_FUNCTION(AptDate, getHours) { return AptInteger::Create(pThis->c_date()->mTM.Hour); }
NATIVE_MEMBER_FUNCTION(AptDate, getMilliseconds) { return AptInteger::Create(pThis->c_date()->mTM.Hundredths); }
NATIVE_MEMBER_FUNCTION(AptDate, getMinutes) { return AptInteger::Create(pThis->c_date()->mTM.Minute); }
NATIVE_MEMBER_FUNCTION(AptDate, getMonth) { return AptInteger::Create(pThis->c_date()->mTM.Month); }
NATIVE_MEMBER_FUNCTION(AptDate, getSeconds) { return AptInteger::Create(pThis->c_date()->mTM.Second); }

//#if defined(EA_PLATFORM_PLAYSTATION2)
//typedef signed long   int64_t;
//#endif
//#if defined(EA_PLATFORM_GAMECUBE)
//typedef signed long long int64_t;
//#endif
//#if defined(EA_PLATFORM_XBOX)
//typedef __int64   int64_t;
//#endif
//#if defined(EA_PLATFORM_WINDOWS)
//typedef __int64   int64_t;
//#endif
//
//int64_t atoi64(const char *nptr)
//{
//  int64_t total = 0;
//  int64_t minus = 0;
//
//  while (isspace(*nptr))
//    {
//        nptr++;
//    }
//
//  if (*nptr == '+')
//    {
//        nptr++;
//    }
//  else if (*nptr == '-')
//    {
//      minus = 1;
//      nptr++;
//  }
//
//  while (isdigit(*nptr))
//    {
//      total *= 10;
//      total += (*nptr++ - '0');
//  }
//
//  return minus ? -total : total;
//}
//
//char *_WriteInt(int64_t ival, char* buf, char* eob, char pad, int width, int bsigned)
//{
//    char revStr[265]; // big enough to hold any int;
//    int i, n = 0, sign=0;
//    int64_t val, nextVal;
//
//    val = (bsigned && (sign = (ival < 0))) ? -ival : (int64_t)ival;
//
//    do {
//        nextVal = val/10;
//        revStr[n++] = '0' + (int)(val - (nextVal*10));
//        val = nextVal;
//    } while(val);
//
//    if (width)
//        for (i = width - sign - n; ((i > 0) && (buf < eob)); i--)
//            *buf++ = pad;
//    while ((--n >= 0) && (buf < eob))
//        *buf++ = revStr[n];
//  *buf++ = '\0';
//    return buf;
//}

// For now we can't use this function, but it might be useful later
//void getTimeInMilliSeconds(AptValue *pThis, char *buff)
//{
//  // Since the time is converted to milliseconds, the returning float value might be unexpected do to loss of precision
//  int iYear, iMonth;
//  int64_t nDataInMilSec = 0;
//  AptSysClock *currDate = &pThis->c_date()->mTMU;
//
//  for(iYear = 1970; iYear < currDate->Year; ++iYear)
//  {
//      int64_t pp = pThis->c_date()->dateGetNumDaysInYear(iYear);
//      pp *= SYS_SECONDS_PER_DAY;
//      nDataInMilSec += pp;
//  }
//
//  // compute seconds to last full month
//  for(iMonth = 0; iMonth < currDate->Month; ++iMonth)
//  {
//      nDataInMilSec += pThis->c_date()->dateGetNumDaysInMonth(iMonth, currDate->Year) * SYS_SECONDS_PER_DAY;
//  }
//
//  // get seconds to last full day
//  nDataInMilSec += (currDate->Date - 1) * SYS_SECONDS_PER_DAY;
//
//  nDataInMilSec += (currDate->Hour) * (60*60);
//  nDataInMilSec += (currDate->Minute) * 60;
//  nDataInMilSec += currDate->Second;
//
//  nDataInMilSec *= 1000;
//
//  char *bb = buff, *eob = bb+(255);
//  bb =_WriteInt(nDataInMilSec, bb, eob, '-', 0, 0);
//  //return 0.0;//nDataInMilSec;
//}

NATIVE_MEMBER_FUNCTION(AptDate, getTime)
{
    //AptString *tmp = new AptString();
    //char tt[256];
    //getTimeInMilliSeconds(pThis, &tt[0]);
    //tmp->cpy(tt);
    //return tmp;
    return AptInteger::Create(0);
}

NATIVE_MEMBER_FUNCTION(AptDate, getTimezoneOffset)
{
    return AptInteger::Create(abs(pThis->c_date()->hourDiff) * 60);
}

NATIVE_MEMBER_FUNCTION(AptDate, getUTCDate) { return AptInteger::Create(pThis->c_date()->mTMU.Date); }
NATIVE_MEMBER_FUNCTION(AptDate, getUTCDay) { return AptInteger::Create(pThis->c_date()->getDayOfWeek(pThis->c_date()->mTMU.Year, pThis->c_date()->mTMU.Month, pThis->c_date()->mTMU.Date)); }
NATIVE_MEMBER_FUNCTION(AptDate, getUTCFullYear) { return AptInteger::Create(pThis->c_date()->mTMU.Year); }
NATIVE_MEMBER_FUNCTION(AptDate, getUTCHours) { return AptInteger::Create(pThis->c_date()->mTMU.Hour); }
NATIVE_MEMBER_FUNCTION(AptDate, getUTCMilliseconds) { return AptInteger::Create(pThis->c_date()->mTMU.Hundredths); }
NATIVE_MEMBER_FUNCTION(AptDate, getUTCMinutes) { return AptInteger::Create(pThis->c_date()->mTMU.Minute); }
NATIVE_MEMBER_FUNCTION(AptDate, getUTCMonth) { return AptInteger::Create(pThis->c_date()->mTMU.Month); }
NATIVE_MEMBER_FUNCTION(AptDate, getUTCSeconds) { return AptInteger::Create(pThis->c_date()->mTMU.Second); }

NATIVE_MEMBER_FUNCTION(AptDate, getYear) { return AptInteger::Create(pThis->c_date()->mTM.Year - 1900); }

NATIVE_MEMBER_FUNCTION(AptDate, setDate) {
    if (nParams < 1) return gpUndefinedValue;
    pThis->c_date()->mTM.Date = gAptActionInterpreter.stackAt(0)->toInteger();
    pThis->c_date()->setDates(&pThis->c_date()->mTM, &pThis->c_date()->mTMU, pThis->c_date()->hourDiff);
    return AptInteger::Create(0);
    //SET_TM(Date);
}
NATIVE_MEMBER_FUNCTION(AptDate, setFullYear)
{
    if (nParams < 1) return gpUndefinedValue;
    pThis->c_date()->mTM.Year = gAptActionInterpreter.stackAt(0)->toInteger();
    if(nParams > 1)
        pThis->c_date()->mTM.Month = gAptActionInterpreter.stackAt(1)->toInteger();
    if(nParams > 2)
        pThis->c_date()->mTM.Date = gAptActionInterpreter.stackAt(2)->toInteger();

    pThis->c_date()->setDates(&pThis->c_date()->mTM, &pThis->c_date()->mTMU, pThis->c_date()->hourDiff);
    return AptInteger::Create(0);
}
NATIVE_MEMBER_FUNCTION(AptDate, setHours) {
    if (nParams < 1) return gpUndefinedValue;
    pThis->c_date()->mTM.Hour = gAptActionInterpreter.stackAt(0)->toInteger();
    pThis->c_date()->setDates(&pThis->c_date()->mTM, &pThis->c_date()->mTMU, pThis->c_date()->hourDiff);
    return AptInteger::Create(0);
    //SET_TM(Hour);
}
NATIVE_MEMBER_FUNCTION(AptDate, setMilliseconds) {
    if (nParams < 1) return gpUndefinedValue;
    pThis->c_date()->mTM.Hundredths = gAptActionInterpreter.stackAt(0)->toInteger();
    pThis->c_date()->setDates(&pThis->c_date()->mTM, &pThis->c_date()->mTMU, pThis->c_date()->hourDiff);
    return AptInteger::Create(0);
    //SET_TM(Hundredths);
}
NATIVE_MEMBER_FUNCTION(AptDate, setMinutes) {
    if (nParams < 1) return gpUndefinedValue;
    pThis->c_date()->mTM.Minute = gAptActionInterpreter.stackAt(0)->toInteger();
    pThis->c_date()->setDates(&pThis->c_date()->mTM, &pThis->c_date()->mTMU, pThis->c_date()->hourDiff);
    return AptInteger::Create(0);
    //SET_TM(Minute);
}
NATIVE_MEMBER_FUNCTION(AptDate, setMonth) {
    if (nParams < 1) return gpUndefinedValue;
    pThis->c_date()->mTM.Month = gAptActionInterpreter.stackAt(0)->toInteger();
    pThis->c_date()->setDates(&pThis->c_date()->mTM, &pThis->c_date()->mTMU, pThis->c_date()->hourDiff);
    return AptInteger::Create(0);
    //SET_TM(Month);
}
NATIVE_MEMBER_FUNCTION(AptDate, setSeconds) {
    if (nParams < 1) return gpUndefinedValue;
    pThis->c_date()->mTM.Second = gAptActionInterpreter.stackAt(0)->toInteger();
    pThis->c_date()->setDates(&pThis->c_date()->mTM, &pThis->c_date()->mTMU, pThis->c_date()->hourDiff);
    return AptInteger::Create(0);
    //SET_TM(Second);
}

NATIVE_MEMBER_FUNCTION(AptDate, setTime)
{
//  // The time value can be very large value hence the floats, but even float values lose precision,
//  //  so the calculated date might not be the same as expected.
//  int _SysMonthDays[13] = { 0, 31, 28, 31, 30 , 31, 30, 31, 31, 30 , 31, 30 ,31 };
//  char t2[256];
//  gAptActionInterpreter.stackAt(0)->toString(t2);
//  int64_t t1 = atoi64(t2);
//
//  t1 /= 1000;
//  pThis->c_date()->mTMU.Second = (int)(t1 % 60);
//
//  int64_t Minutes = (t1/60);
//    pThis->c_date()->mTMU.Minute = (int)(Minutes % 60);
//
//  int64_t Hours = (t1 / 3600);
//    pThis->c_date()->mTMU.Hour = (int)(Hours % 24);
//
//    int64_t Days = (t1/86400);
//    int iCount = 366;
//    pThis->c_date()->mTMU.Year = 0;
//  while ( Days >= iCount )
//    {
//      Days -= iCount;
//        pThis->c_date()->mTMU.Year++;
//        if (pThis->c_date()->dateIsYearLeap(pThis->c_date()->mTMU.Year) )
//        {
//          iCount = 366;
//        }
//        else
//        {
//          iCount = 365;
//        }
//    }
//    //base is year 2000
//  pThis->c_date()->mTMU.Year += 1970;
//
//    //calculate number of days in february
//    if (pThis->c_date()->dateIsYearLeap(pThis->c_date()->mTMU.Year) )
//    {
//      _SysMonthDays[2] = 29;
//    }
//    else
//    {
//      _SysMonthDays[2] = 28;
//    }
//
//    //calculate the month and day of month bassed on year and day of year
//    iCount = 1;
//    while (Days >= _SysMonthDays[iCount])
//    {
//      Days -= _SysMonthDays[iCount++];
//        APT_ASSERT(iCount < 13);
//    }
//  pThis->c_date()->mTMU.Month = iCount - 1;
//    pThis->c_date()->mTMU.Date = (int)(Days + 1); // Days are 1-based
//
//    pThis->c_date()->mTMU.Hundredths = (int)t1;
//  pThis->c_date()->setDates(&pThis->c_date()->mTMU, &pThis->c_date()->mTM, -pThis->c_date()->hourDiff);
    return AptInteger::Create(0);
}

NATIVE_MEMBER_FUNCTION(AptDate, setUTCDate) {
    if (nParams < 1) return gpUndefinedValue;
    pThis->c_date()->mTMU.Date = gAptActionInterpreter.stackAt(0)->toInteger();
    pThis->c_date()->setDates(&pThis->c_date()->mTMU, &pThis->c_date()->mTM, -pThis->c_date()->hourDiff);
    return AptInteger::Create(0);
    //SET_TM(Date);
}
NATIVE_MEMBER_FUNCTION(AptDate, setUTCFullYear)
{
    if (nParams < 1) return gpUndefinedValue;
    pThis->c_date()->mTMU.Year = gAptActionInterpreter.stackAt(0)->toInteger();
    if(nParams > 1)
        pThis->c_date()->mTMU.Month = gAptActionInterpreter.stackAt(1)->toInteger();
    if(nParams > 2)
        pThis->c_date()->mTMU.Date = gAptActionInterpreter.stackAt(2)->toInteger();

    pThis->c_date()->setDates(&pThis->c_date()->mTMU, &pThis->c_date()->mTM, -pThis->c_date()->hourDiff);
    return AptInteger::Create(0);
}
NATIVE_MEMBER_FUNCTION(AptDate, setUTCHours) {
    if (nParams < 1) return gpUndefinedValue;
    pThis->c_date()->mTMU.Hour = gAptActionInterpreter.stackAt(0)->toInteger();
    pThis->c_date()->setDates(&pThis->c_date()->mTMU, &pThis->c_date()->mTM, -pThis->c_date()->hourDiff);
    return AptInteger::Create(0);
    //SET_TMU(Hour);
}
NATIVE_MEMBER_FUNCTION(AptDate, setUTCMilliseconds) {
    if (nParams < 1) return gpUndefinedValue;
    pThis->c_date()->mTMU.Hundredths = gAptActionInterpreter.stackAt(0)->toInteger();
    pThis->c_date()->setDates(&pThis->c_date()->mTMU, &pThis->c_date()->mTM, -pThis->c_date()->hourDiff);
    return AptInteger::Create(0);
    //SET_TMU(Hundredths);
}
NATIVE_MEMBER_FUNCTION(AptDate, setUTCMinutes) {
    if (nParams < 1) return gpUndefinedValue;
    pThis->c_date()->mTMU.Minute = gAptActionInterpreter.stackAt(0)->toInteger();
    pThis->c_date()->setDates(&pThis->c_date()->mTMU, &pThis->c_date()->mTM, -pThis->c_date()->hourDiff);
    return AptInteger::Create(0);
    //SET_TMU(Minute);
}
NATIVE_MEMBER_FUNCTION(AptDate, setUTCMonth) {
    if (nParams < 1) return gpUndefinedValue;
    pThis->c_date()->mTMU.Month = gAptActionInterpreter.stackAt(0)->toInteger();
    pThis->c_date()->setDates(&pThis->c_date()->mTMU, &pThis->c_date()->mTM, -pThis->c_date()->hourDiff);
    return AptInteger::Create(0);
    //SET_TMU(Month);
}
NATIVE_MEMBER_FUNCTION(AptDate, setUTCSeconds) {
    if (nParams < 1) return gpUndefinedValue;
    pThis->c_date()->mTMU.Second = gAptActionInterpreter.stackAt(0)->toInteger();
    pThis->c_date()->setDates(&pThis->c_date()->mTMU, &pThis->c_date()->mTM, -pThis->c_date()->hourDiff);
    return AptInteger::Create(0);
    //SET_TMU(Second);
}
NATIVE_MEMBER_FUNCTION(AptDate, setYear)
{
    if (nParams < 1) return gpUndefinedValue;
    pThis->c_date()->mTM.Year = gAptActionInterpreter.stackAt(0)->toInteger();
    pThis->c_date()->setDates(&pThis->c_date()->mTM, &pThis->c_date()->mTMU, pThis->c_date()->hourDiff);
    return AptInteger::Create(0);
    //SET_TM(Year);
}

NATIVE_MEMBER_FUNCTION(AptDate, toString)
{
    AptString *newDate = AptString::Create();
    AptNativeString tmp;
    pThis->c_date()->toString(tmp);
    newDate->cpy(tmp.ConstRawPtr());
    return newDate;
}

NATIVE_MEMBER_FUNCTION(AptDate, UTC)
{
    // This function is not yet supported.

    //if (nParams < 2) return gpUndefinedValue;
    //pThis->c_date()->mTMU.Year = gAptActionInterpreter.stackAt(0)->toInteger();
    //if(nParams > 1)
    //  pThis->c_date()->mTMU.Month = gAptActionInterpreter.stackAt(1)->toInteger();
    //if(nParams > 2)
    //  pThis->c_date()->mTMU.Date = gAptActionInterpreter.stackAt(2)->toInteger();
    //if(nParams > 3)
    //  pThis->c_date()->mTMU.Hour = gAptActionInterpreter.stackAt(2)->toInteger();
    //if(nParams > 4)
    //  pThis->c_date()->mTMU.Minute = gAptActionInterpreter.stackAt(2)->toInteger();
    //if(nParams > 5)
    //  pThis->c_date()->mTMU.Second = gAptActionInterpreter.stackAt(2)->toInteger();
    //if(nParams > 6)
    //  pThis->c_date()->mTMU.Hundredths = gAptActionInterpreter.stackAt(2)->toInteger();
    //pThis->c_date()->setDates(&pThis->c_date()->mTMU, &pThis->c_date()->mTM, -pThis->c_date()->hourDiff);
    //return new AptFloat(getTimeInMilliSeconds(pThis));
    return gpUndefinedValue;
}


/*F************************************************************************************************/
/*!
    Returns the AptValue for the context and the member name.

    \Input      pContext          - The context passed in.
    \Input      pName             - The member name.

    \Output     AptValue *        - The AptValue of the member.
                                    gpUndefinedValue is the member is unknown.

    \Notes
        None.

    \Version
        1.0        03/26/04 (ONA) First Version

*/
/************************************************************************************************F*/
AptValue *    AptDate::objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const
{
    DateMembers *pProp = pContext ? DateMembersIndex::in_word_set(pName->c_str(), pName->Size()) : NULL;
    if (pProp)
    {
        switch(pProp->nIndex)
        {
            case(AptDate_getDate):
            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(getDate);
            }
            break;
            case AptDate_getDay:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(getDay);
            }
            break;
            case AptDate_getFullYear:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(getFullYear);
            }
            break;
            case AptDate_getHours:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(getHours);
            }
            break;
            case AptDate_getMilliseconds:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(getMilliseconds);
            }
            break;
            case AptDate_getMinutes:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(getMinutes);
            }
            break;
            case AptDate_getMonth:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(getMonth);
            }
            break;
            case AptDate_getSeconds:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(getSeconds);
            }
            break;
            case AptDate_getTime:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(getTime);
            }
            break;
            case AptDate_getTimezoneOffset:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(getTimezoneOffset);
            }
            break;
            case AptDate_getUTCDate:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(getUTCDate);
            }
            break;
            case AptDate_getUTCDay:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(getUTCDay);
            }
            break;
            case AptDate_getUTCFullYear:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(getUTCFullYear);
            }
            break;
            case AptDate_getUTCHours:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(getUTCHours);
            }
            break;
            case AptDate_getUTCMilliseconds:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(getUTCMilliseconds);
            }
            break;
            case AptDate_getUTCMinutes:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(getUTCMinutes);
            }
            break;
            case AptDate_getUTCMonth:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(getUTCMonth);
            }
            break;
            case AptDate_getUTCSeconds:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(getUTCSeconds);
            }
            break;
            case AptDate_getYear:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(getYear);
            }
            break;
            case AptDate_setDate:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(setDate);
            }
            break;
            case AptDate_setFullYear:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(setFullYear);
            }
            break;
            case AptDate_setHours:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(setHours);
            }
            break;
            case AptDate_setMilliseconds:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(setMilliseconds);
            }
            break;
            case AptDate_setMinutes:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(setMinutes);
            }
            break;
            case AptDate_setMonth:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(setMonth);
            }
            break;
            case AptDate_setSeconds:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(setSeconds);
            }
            break;
            case AptDate_setTime:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(setTime);
            }
            break;
            case AptDate_setUTCDate:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(setUTCDate);
            }
            break;
            case AptDate_setUTCFullYear:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(setUTCFullYear);
            }
            break;
            case AptDate_setUTCHours:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(setUTCHours);
            }
            break;
            case AptDate_setUTCMilliseconds:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(setUTCMilliseconds);
            }
            break;
            case AptDate_setUTCMinutes:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(setUTCMinutes);
            }
            break;
            case AptDate_setUTCMonth:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(setUTCMonth);
            }
            break;
            case AptDate_setUTCSeconds:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(setUTCSeconds);
            }
            break;
            case AptDate_setYear:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(setYear);
            }
            break;
            case AptDate_toString:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(toString);
            }
            break;
            case AptDate_UTC:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(UTC);
            }
        }
    }

#if defined(EA_DEBUG)
    if (
        (pName->EqualNoCase("getDate")) ||
        (pName->EqualNoCase("getDay")) ||
        (pName->EqualNoCase("getFullYear")) ||
        (pName->EqualNoCase("getHours")) ||
        (pName->EqualNoCase("getMilliseconds")) ||
        (pName->EqualNoCase("getMinutes")) ||
        (pName->EqualNoCase("getMonth")) ||
        (pName->EqualNoCase("getSeconds")) ||
        (pName->EqualNoCase("getTime")) ||
        (pName->EqualNoCase("getTimezoneOffset")) ||
        (pName->EqualNoCase("getUTCDate")) ||
        (pName->EqualNoCase("getUTCDay")) ||
        (pName->EqualNoCase("getUTCFullYear")) ||
        (pName->EqualNoCase("getUTCHours")) ||
        (pName->EqualNoCase("getUTCMilliseconds")) ||
        (pName->EqualNoCase("getUTCMinutes")) ||
        (pName->EqualNoCase("getUTCMonth")) ||
        (pName->EqualNoCase("getUTCSeconds")) ||
        (pName->EqualNoCase("getYear")) ||
        (pName->EqualNoCase("setDate")) ||
        (pName->EqualNoCase("setFullYear")) ||
        (pName->EqualNoCase("setHours")) ||
        (pName->EqualNoCase("setMilliseconds")) ||
        (pName->EqualNoCase("setMinutes")) ||
        (pName->EqualNoCase("setMonth")) ||
        (pName->EqualNoCase("setSeconds")) ||
        (pName->EqualNoCase("setTime")) ||
        (pName->EqualNoCase("setUTCDate")) ||
        (pName->EqualNoCase("setUTCFullYear")) ||
        (pName->EqualNoCase("setUTCHours")) ||
        (pName->EqualNoCase("setUTCMilliseconds")) ||
        (pName->EqualNoCase("setUTCMinutes")) ||
        (pName->EqualNoCase("setUTCMonth")) ||
        (pName->EqualNoCase("setUTCSeconds")) ||
        (pName->EqualNoCase("setYear")) ||
        (pName->EqualNoCase("toString")) ||
        (pName->EqualNoCase("UTC"))
    )
    {
        APT_DEBUGPRINT(APT_DEBUG_MSG_WARNING_LVL, "AptDate: Incorrect case for '%s'.\n", pName->c_str());
        APT_ASSERT(0);
    }
#endif

    return 0;
}

/*F************************************************************************************************/
/*!
    Releases the AptNativeScript functions.

    \Input      None.

    \Output     None.

    \Notes
        This function needs to be called at least during AptShutdown.

    \Version
        1.0        04/05/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    AptDate::CleanNativeFunctions()
{
#if defined (APT_USE_DATE_OBJECT)
    NATIVE_MEMBER_FUNCTION_DESTROY(getDate);
    NATIVE_MEMBER_FUNCTION_DESTROY(getDay);
    NATIVE_MEMBER_FUNCTION_DESTROY(getFullYear);
    NATIVE_MEMBER_FUNCTION_DESTROY(getHours);
    NATIVE_MEMBER_FUNCTION_DESTROY(getMilliseconds);
    NATIVE_MEMBER_FUNCTION_DESTROY(getMinutes);
    NATIVE_MEMBER_FUNCTION_DESTROY(getMonth);
    NATIVE_MEMBER_FUNCTION_DESTROY(getSeconds);
    NATIVE_MEMBER_FUNCTION_DESTROY(getTime);
    NATIVE_MEMBER_FUNCTION_DESTROY(getTimezoneOffset);
    NATIVE_MEMBER_FUNCTION_DESTROY(getUTCDate);
    NATIVE_MEMBER_FUNCTION_DESTROY(getUTCDay);
    NATIVE_MEMBER_FUNCTION_DESTROY(getUTCFullYear);
    NATIVE_MEMBER_FUNCTION_DESTROY(getUTCHours);
    NATIVE_MEMBER_FUNCTION_DESTROY(getUTCMilliseconds);
    NATIVE_MEMBER_FUNCTION_DESTROY(getUTCMinutes);
    NATIVE_MEMBER_FUNCTION_DESTROY(getUTCMonth);
    NATIVE_MEMBER_FUNCTION_DESTROY(getUTCSeconds);
    NATIVE_MEMBER_FUNCTION_DESTROY(getYear);
    NATIVE_MEMBER_FUNCTION_DESTROY(setDate);
    NATIVE_MEMBER_FUNCTION_DESTROY(setFullYear);
    NATIVE_MEMBER_FUNCTION_DESTROY(setHours);
    NATIVE_MEMBER_FUNCTION_DESTROY(setMilliseconds);
    NATIVE_MEMBER_FUNCTION_DESTROY(setMinutes);
    NATIVE_MEMBER_FUNCTION_DESTROY(setMonth);
    NATIVE_MEMBER_FUNCTION_DESTROY(setSeconds);
    NATIVE_MEMBER_FUNCTION_DESTROY(setTime);
    NATIVE_MEMBER_FUNCTION_DESTROY(setUTCDate);
    NATIVE_MEMBER_FUNCTION_DESTROY(setUTCFullYear);
    NATIVE_MEMBER_FUNCTION_DESTROY(setUTCHours);
    NATIVE_MEMBER_FUNCTION_DESTROY(setUTCMilliseconds);
    NATIVE_MEMBER_FUNCTION_DESTROY(setUTCMinutes);
    NATIVE_MEMBER_FUNCTION_DESTROY(setUTCMonth);
    NATIVE_MEMBER_FUNCTION_DESTROY(setUTCSeconds);
    NATIVE_MEMBER_FUNCTION_DESTROY(setYear);
    NATIVE_MEMBER_FUNCTION_DESTROY(toString);
    NATIVE_MEMBER_FUNCTION_DESTROY(UTC);
#endif
}

#endif      // end of #if defined (APT_USE_DATE_OBJECT) from very beginning of file.
