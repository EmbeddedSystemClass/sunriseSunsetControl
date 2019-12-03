#include "datetime_utils.h"

#if UNIT_TEST
#define unit_static
#else
#define unit_static static
#endif

//hodi se i pro vypocet poctu dni od 1.1.2000
static const uint8_t DayMonthShift[12] = {0, 3, 3, 6, 8, 11, 13, 16, 19, 21, 24, 26};

//number of days in each month
static const uint8_t Months[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// Winter -> Summer
#define PART_WI_SU  0
// Summer -> Winter
#define PART_SU_WI  2

//v11_0
static uint8_t const ZoneOffsets[] = {
    53, //	0
    181, //	1
    185, //	2
    189, //	3
    189, //	4
    61, //	5
    205, //	6
    53, //	7
    65, //	8
    49, //	9
    53, //	10
    69, //	11
    71, //	12
    21, //	13
    73, //	14
    75, //	15
    77, //	16
    81, //	17
    85, //	18
    89, //	19
    17, //	20
    219, //	21
    91, //	22
    93, //	23
    221, //	24
    53, //	25
    53, //	26
    97, //	27
    229, //	28
    101, //	29
    177, //	30
    45, //	31
    169, //	32
    41, //	33
    167, //	34
    165, //	35
    37, //	36
    33, //	37
    161, //	38
    157, //	39
    29, //	40
    25, //	41
    153, //	42
    149, //	43
    145, //	44
    13, //	45
    9, //	46
    157, //	47
    141, //	48
    53, //	49
    53, //	50
    53, //	51
    53, //	52
    53, //	53
    177, //	54
    169, //	55
    53, //	56
    85, //	57
    35, //	58
    57, //	59
    53, //	60
    53, //	61
    197, //	62
    53, //	63
    53, //	64
    53//	65
};

static uint16_t const ZoneDSTParam[] = {
    0, 0, 0, 0, //	0
    64, 1875, 128, 1882, //	1
    128, 1875, 192, 1882, //	2
    192, 1875, 256, 1882, //	3
    192, 1875, 256, 1882, //	4
    0, 0, 0, 0, //	5
    1467, 1107, 64, 1370, //	6
    0, 0, 0, 0, //	7
    0, 0, 0, 0, //	8
    0, 0, 0, 0, //	9
    0, 0, 0, 0, //	10
    0, 0, 0, 0, //	11
    0, 0, 0, 0, //	12
    0, 0, 0, 0, //	13
    0, 0, 0, 0, //	14
    0, 0, 0, 0, //	15
    0, 0, 0, 0, //	16
    0, 0, 0, 0, //	17
    0, 0, 0, 0, //	18
    0, 0, 0, 0, //	19
    0, 0, 0, 0, //	20
    128, 1818, 192, 1812, //	21
    0, 0, 0, 0, //	22
    0, 0, 0, 0, //	23
    128, 1818, 192, 1812, //	24
    0, 0, 0, 0, //	25
    0, 0, 0, 0, //	26
    0, 0, 0, 0, //	27
    128, 1881, 192, 1812, //	28
    0, 0, 0, 0, //	29
    0, 1875, 64, 1882, //	30
    0, 0, 0, 0, //	31
    0, 1850, 0, 1842, //	32
    0, 0, 0, 0, //	33
    128, 1827, 128, 1819, //	34
    128, 1827, 128, 1819, //	35
    0, 0, 0, 0, //	36
    0, 0, 0, 0, //	37
    128, 1827, 128, 1819, //	38
    128, 1827, 128, 1819, //	39
    0, 0, 0, 0, //	40
    0, 0, 0, 0, //	41
    128, 1827, 128, 1819, //	42
    128, 1827, 128, 1819, //	43
    128, 1827, 128, 1819, //	44
    0, 0, 0, 0, //	45
    0, 0, 0, 0, //	46
    128, 1812, 128, 1882, //	47
    128, 1827, 128, 1819, //	48
    0, 0, 0, 0, //	49
    0, 0, 0, 0, //	50
    0, 0, 0, 0, //	51
    0, 0, 0, 0, //	52
    0, 0, 0, 0, //	53
    64, 1875, 64, 1882, //	54
    1408, 1619, 1472, 1626, //	55
    0, 0, 0, 0, //	56
    0, 0, 0, 0, //	57
    0, 0, 0, 0, //	58
    0, 0, 0, 0, //	59
    0, 0, 0, 0, //	60
    0, 0, 0, 0, //	61
    256, 1875, 320, 1882, //	62
    0, 0, 0, 0, //	63
    0, 0, 0, 0, //	64
    0, 0, 0, 0 //	65	- Auto
};

#if 0
#define CACHESIZE  5
struct ZoneCache_s ZoneCache[CACHESIZE];
#endif

/* ----------------- AbsTime_s arithmetic ------------------------------------*/

/**
 * Copies one AbsTime_s structure into another
 * @param Dest
 * @param Source
 */
void AbsTime_Copy(struct AbsTime_s *Dest, const struct AbsTime_s *Source) {
    Dest->Sec = Source->Sec;
    Dest->Millisec = Source->Millisec;
}

/**
 * Set values in AbsTime_s  struct
 * @param Acc
 * @param Sec
 * @param Millisec
 */
void AbsTime_Set(struct AbsTime_s *Acc, uint32_t Sec, uint16_t Millisec) {
    Acc->Sec = Sec;
    Acc->Millisec = Millisec;
    AbsTime_NormTime(Acc);
}

/**
 * Makes ceiling function with the seconds in time struct
 * @param Acc
 * @param Divisor
 */
void AbsTime_Ceil(struct AbsTime_s *Acc, uint32_t Divisor) {
    uint32_t Rest = (Acc->Sec) % Divisor;
    if (Rest == 0 && Acc->Millisec == 0) {
        Rest = Divisor;
    }
    Acc->Sec += Divisor - Rest;
    Acc->Millisec = 0;
}

/**
 * Makes floor function with the seconds in time struct
 * @param Acc
 * @param Divisor
 */
void AbsTime_Floor(struct AbsTime_s *Acc, uint32_t Divisor) {
    uint32_t Rest = (Acc->Sec) % Divisor;
    Acc->Sec -= Rest;
    Acc->Millisec = 0;
}

/**
 *
 * @param Acc
 */
void AbsTime_Increment(struct AbsTime_s *Acc, int32_t Millisec) {
    Millisec += Acc->Millisec;

    Acc->Sec += Millisec / 1000;
    Millisec = Millisec % 1000;
    if (Millisec < 0) {
        Millisec += 1000;
        Acc->Sec--;
    }
    Acc->Millisec = Millisec;
}

/**
 * Normalize the time to have the Millisec part always positive and in range 0..999
 * @param Acc
 */
void AbsTime_NormTime(struct AbsTime_s *Acc) {
    if (Acc->Millisec >= 1000) {
        Acc->Sec += Acc->Millisec / 1000;
        Acc->Millisec = Acc->Millisec % 1000;
    }
}

/**
 * Adds the Diff to the Acc
 * @param Acc
 * @param Diff
 */
void AbsTime_Add(struct AbsTime_s *Acc, const struct AbsTime_s *Diff) {
    Acc->Millisec += Diff->Millisec;
    Acc->Sec += Diff->Sec;
    AbsTime_NormTime(Acc);
}

/**
 * Subtracts the Diff from Acc
 * @param Acc
 * @param Diff
 */
void AbsTime_Sub(struct AbsTime_s *Acc, const struct AbsTime_s *Diff) {
    uint32_t diffSec = Diff->Sec;
    if (Acc->Millisec < Diff->Millisec) {
        Acc->Millisec += 1000 - Diff->Millisec;
        diffSec++;
    } else {
        Acc->Millisec -= Diff->Millisec;
    }
    if (Acc->Sec < diffSec) {
        AbsTime_Set(Acc, 0, 0);
    } else {
        Acc->Sec -= diffSec;
    }
    AbsTime_NormTime(Acc);
}

/**
 * Divides time /2 - only for positive values
 * @param Acc
 */
void AbsTime_Div2(struct AbsTime_s *Acc) {
    if (Acc->Sec % 2) {
        Acc->Millisec += 1000;
    }
    Acc->Sec /= 2;
    Acc->Millisec /= 2;
}

/**
 * returns number of milliseconds
 * @param Acc
 * @return
 */
uint32_t AbsTime_TotalMillisec(struct AbsTime_s *Acc) {
    return ((Acc->Sec * 1000) + Acc->Millisec);
}

/**
 * Compares, if one time equals to the other
 * @param Time
 * @param Comp
 * @return
 */
CompareTime_e AbsTime_Compare(const struct AbsTime_s *Time, const struct AbsTime_s *Comp) {
    if (Time->Sec > Comp->Sec) return MORE;
    if (Time->Sec < Comp->Sec) return LESS;
    if (Time->Millisec > Comp->Millisec) return MORE;
    if (Time->Millisec < Comp->Millisec) return LESS;
    return EQUALS;
}

/**
 * Compares, if CompTime equals to the (AbsTime +- Margin)
 * @param Time
 * @param Margin
 * @param Comp
 * @return 1 if Time is in Margin from Comp otherwise 0
 */
int AbsTime_CompareWithinMargin(const struct AbsTime_s *Time, const struct AbsTime_s *Margin, const struct AbsTime_s *Comp) {
    struct AbsTime_s TempTime;

    AbsTime_Copy(&TempTime, Time);
    AbsTime_Add(&TempTime, Margin);
    if (AbsTime_Compare(&TempTime, Comp) != MORE) return (0); //Comp is not in margin

    AbsTime_Copy(&TempTime, Time);
    AbsTime_Sub(&TempTime, Margin);
    if (AbsTime_Compare(&TempTime, Comp) != LESS) return (0); //Comp is not in margin
    return (1); //Comp is in margin
}

/*------------------- date arithmetic ----------------------------------------*/

/**
 * Get number of leap days during elapsed years from 1.1.0000 to 1.1.Year
 * @param Year
 * @return
 */
uint32_t Date_LeapDays(uint32_t Year) {
    return GET_LEAP_DAYS(Year);
}

/**
 * Get number of days from 1.1. 0000 to 1.1.Year
 * @param Year
 * @return
 */
uint32_t Date_ElapsedDays(uint32_t Year) {
    return GET_ELAPSED_DAYS(Year);
}

/**
 * Returns 1 if the year is leap
 * @param Year
 * @return 1 when leap, otherwise 0
 */
int Date_IsLeapYear(uint32_t Year) {
    return (!(Year % 4) && (Year % 100)) || !(Year % 400);
}

/**
 * Returns day of week 1 to 7 (1.1.2000 was sunday 6), from DayCount
 * @param DayCountVal
 * @return day of week 1 to 7
 */
int Date_WeekDayFromDayCount(uint32_t DayCountVal) {
    int result = ((DayCountVal + ABSTIME_DAY_CORR) % 7) + ABSTIME_START_DAY;
    if (result > 7) result -= 7;
    return result;
}

/**
 * Returns day of week 1 to 7 (1.1.2000 was sunday 6), from date
 * @param day
 * @param month
 * @param year
 * @return day of week 1 to 7
 */
int Date_WeekDay(uint32_t day, uint32_t month, uint32_t year) {
    return Date_WeekDayFromDayCount(Date_DayCount(day, month, year));
}

/**
 * Returns number of days from 1.1.2000 to preset day
 * @param Day
 * @param Month
 * @param Year
 * @return
 */
uint32_t Date_DayCount(uint32_t Day, uint32_t Month, uint32_t Year) {
    uint32_t result;
    if (Month == 0) Month = 1;
    if (Month > 12) Month = 12;
    result = Date_ElapsedDays(Year) - ABSTIME_DAY_CORR + (Month - 1)*28 + DayMonthShift[Month - 1] + Day;
    if ((Date_IsLeapYear(Year))&&(Month < 3)) result--; //prestupny rok pred 29.02.
    return (result);
}

/**
 * Returns day number
 * @param Day
 * @param Month
 * @param Year
 * @return day of year (uint32_t) - 1.1. returns 1
 */
uint32_t Date_DayOfYear(uint32_t Day, uint32_t Month, uint32_t Year) {
    return Date_DayCount(Day, Month, Year) - Date_DayCount(1, 1, Year) + 1;
}

/**
 * Returns seconds since 00:00:00
 * @param Hour
 * @param Min
 * @param Sec
 * @return
 */
uint32_t Time_SecCount(uint32_t Hour, uint32_t Min, uint32_t Sec) {
    return (Hour * 60 + Min)*60 + Sec;
}

/**
 * Returns seconds since 1.1.2000 00:00:00
 * @param Day
 * @param Month
 * @param Year
 * @param Hour
 * @param Min
 * @param Sec
 * @return
 */
uint32_t DateTime_TotalSec(uint16_t Day, uint16_t Month, uint16_t Year, uint16_t Hour, uint16_t Min, uint16_t Sec) {
    uint32_t result;
    result = Date_DayCount(Day, Month, Year);
    result = result * 86400 + Time_SecCount(Hour, Min, Sec);
    return result;
}

/**
 * Returns seconds since 1.1.2000 00:00:00
 * @param DayCountVal
 * @param SecCountVal
 * @return
 */
uint32_t DateTime_TotalSecFromCount(uint32_t DayCountVal, uint32_t SecCountVal) {
    return (DayCountVal * 86400 + SecCountVal);
}

/**
 * Gets ComponentDate day.month. from DayOfYear value
 * @param Year
 * @param DayOfYear
 * @param DateObject output
 */
void Date_DayOfYearToDate(uint32_t Year, uint32_t DayOfYear, RTC_DATE *DateObject) {
    //DayOfYear - kolikaty den v roce - pocita se od 1
    int Month = 0;
    int Day = DayOfYear;
    while (Day > 0) {
        Month++;
        Day -= Months[Month - 1];
        if ((Month == 2)&&((Date_IsLeapYear(Year)))) Day--;
    }
    DateObject->month = Month;

    if ((Month == 2)&&((Date_IsLeapYear(Year)))) Day++;
    Day += Months[Month - 1];
    DateObject->day = Day;
    DateObject->year = Year % 100;
    DateObject->century = Year / 100;
    DateObject->weekday = Date_WeekDay(Day, Month, Year);
}

/**
 * Get time in Hour:Min.Sec.... components from absolute value
 * @param AbsVal
 * @param MilliSec
 * @param TimeObject
 */
void AbsTime_ToTime(uint32_t AbsVal, uint16_t Millisec, RTC_TIME *TimeObject) {
    AbsVal %= 86400; //cut-off the daycount information
    TimeObject->hours = AbsVal / 3600;
    AbsVal %= 3600;
    TimeObject->minutes = AbsVal / 60;
    TimeObject->seconds = AbsVal % 60;
    TimeObject->milliseconds = Millisec;
}

/**
 * Get Year and Day of Year from absolute value
 * @param AbsVal
 * @param Year
 * @param DayOfYear
 */
void AbsTime_DayOfYear(uint32_t AbsVal, uint32_t *Year, uint32_t *DayOfYear) {
    AbsVal /= 86400; //cut-off the seccount information
    AbsVal += ABSTIME_DAY_CORR;
    *Year = AbsVal / 365;
    AbsVal = AbsVal % 365;

    if ((*Year) != 0) {
        AbsVal -= Date_LeapDays(*Year - 1) + 1;

        while ((int32_t) AbsVal < 0) {
            (*Year)--;
            if (!Date_IsLeapYear(*Year)) AbsVal += 365;
            else AbsVal += 366;
        }
    }
    *DayOfYear = AbsVal + 1;
}

/**
 * Get week number from absolute value according to ISO 8601
 * @param AbsVal number of seconds from 1.1.2000
 * @return
 */
int AbsTime_ISOWeek(uint32_t AbsVal) {
    int ISOWeek;
    uint32_t Year;
    uint32_t DayOfYear;
    int WDay;

    AbsTime_DayOfYear(AbsVal, &Year, &DayOfYear);
    WDay = Date_WeekDay(1, 1, Year);

    ISOWeek = (DayOfYear + WDay + 5) / 7;

    if (WDay > 4) {
        ISOWeek--;
        if (ISOWeek == 0) { //ISOWeek is 52 or 53
            WDay = Date_WeekDay(1, 1, Year - 1);
            if ((WDay == 4) || ((WDay == 3)&&(Date_IsLeapYear(Year)))) ISOWeek = 53;
            else ISOWeek = 52;
            return (ISOWeek);
        }
    }


    if (ISOWeek == 53) {
        WDay = Date_WeekDay(1, 1, Year + 1);
        if (WDay <= 4) ISOWeek = 1;
    }

    return (ISOWeek);
}

/**
 * Get date in day.month.year.weekday.century .... components from absolute value
 * @param AbsVal
 * @param DateObject
 */
void AbsTime_ToDate(uint32_t AbsVal, RTC_DATE *DateObject) {
    uint32_t DayOfYear;
    uint32_t Year;

    AbsTime_DayOfYear(AbsVal, &Year, &DayOfYear);

    Date_DayOfYearToDate(Year, DayOfYear, DateObject);
    DateObject->weekday = Date_WeekDayFromDayCount(AbsVal / 86400);
}

/**
 * If it is a leap month
 * @param Year
 * @param Month
 * @return 0=it isn't, 1=it is
 */
int Date_IsLeapMonth(uint32_t Year, uint32_t Month) {
    return ((Date_IsLeapYear(Year))&&(Month == 2)) ? 1 : 0;
}

CompareTime_e Date_Compare(const RTC_DATE * DateA, const RTC_DATE * DateB) {
    if (DateA->century > DateB->century) return MORE;
    if (DateA->century < DateB->century) return LESS;
    if (DateA->year > DateB->year) return MORE;
    if (DateA->year < DateB->year) return LESS;
    if (DateA->month > DateB->month) return MORE;
    if (DateA->month < DateB->month) return LESS;
    if (DateA->day > DateB->day) return MORE;
    if (DateA->day < DateB->day) return LESS;
    return EQUALS;
}

CompareTime_e Time_Compare(const RTC_TIME * TimeA, const RTC_TIME * TimeB, CompareTimeResolution_e resolution) {
    if (TimeA->hours > TimeB->hours) return MORE;
    if (TimeA->hours < TimeB->hours) return LESS;
    if (resolution == HOURS) return EQUALS;
    if (TimeA->minutes > TimeB->minutes) return MORE;
    if (TimeA->minutes < TimeB->minutes) return LESS;
    if (resolution == MINUTES) return EQUALS;
    if (TimeA->seconds > TimeB->seconds) return MORE;
    if (TimeA->seconds < TimeB->seconds) return LESS;
    if (resolution == SECONDS) return EQUALS;
    if (TimeA->milliseconds > TimeB->milliseconds) return MORE;
    if (TimeA->milliseconds < TimeB->milliseconds) return LESS;
    return EQUALS;
}

/**
 * get SecCount from absolute value
 * @param AbsVal
 * @return
 */
uint32_t AbsTime_SecCount(uint32_t AbsVal) {
    return (AbsVal % 86400); //cut-off the daycount information
}

/**
 * Get DayCount from absolute value
 * @param AbsVal
 * @return
 */
uint32_t AbsTime_DayCount(uint32_t AbsVal) {
    return (AbsVal / 86400); //cut-off the seccount information
}

/**
 * Decodes Wi/Su or Su/Wi parameters from ZoneDSTParam array
 * @param TimeZone
 * @param Part
 * @param DSTParam
 */
unit_static void DST_DecodeZoneTable(int TimeZone, int Part, struct DSTParam_s *DSTParam) {
    uint16_t Value1, Value2;

    Value1 = ZoneDSTParam[(TimeZone << 2) + 0 + Part];
    Value2 = ZoneDSTParam[(TimeZone << 2) + 1 + Part];

    DSTParam->Hour = (Value1 >> 6) & 0x1f; //hodina
    DSTParam->Min = Value1 & 0x3f; //minuta

    DSTParam->Day = (Value2 >> 8) & 0x1f; //den v tydnu nebo den mesice (fix)
    DSTParam->Which = (Value2 >> 4) & 0xf; //kolikaty
    DSTParam->Month = Value2 & 0xf; //mesic
}

/**
 * Calculates fixed DST date from DSTParam struct
 * @param DSTParam
 * @param Year
 * @param DSTFix
 */
unit_static void DST_ParamToFix(const struct DSTParam_s *DSTParam, uint32_t Year, struct DSTFix_s *DSTFix) {
    int Day = 0;
    uint8_t Which = DSTParam->Which;

    if (DSTParam->Day > 0) {
        if (Which == CALCDSTWHICH_FIXED) { //fix
            Day = DSTParam->Day;
        } else if ((Which == CALCDSTWHICH_LAST) || (Which == CALCDSTWHICH_PRELAST) || (Which == CALCDSTWHICH_PREPRELAST)) { //last or last but one or last but two
            Day = Date_DaysInMonth(Year, DSTParam->Month);
            while (Date_WeekDay(Day, DSTParam->Month, Year) != DSTParam->Day) {
                Day--;
            }
            if (Which == CALCDSTWHICH_PRELAST) {
                Day -= 7; //last but one
            } else if (Which == CALCDSTWHICH_PREPRELAST) {
                Day -= 14; //last but two
            }
        } else {
            if (Which == CALCDSTWHICH_AFTER15) { //first after 15th of current month DST transition date
                Day = 15;
                Which = 1;
            } else {
                Day = 1;
            }
            while (Date_WeekDay(Day, DSTParam->Month, Year) != DSTParam->Day) {
                Day++;
            }
            Day += (Which - 1)*7;
        }
    }

    DSTFix->Min = DSTParam->Min; //minutes
    DSTFix->Hour = DSTParam->Hour; //hours
    DSTFix->Month = DSTParam->Month; //month
    DSTFix->Day = Day; //day
}

/**
 * Calculates Absolute value of fixed DST date from DSTFix struct
 * @param DSTFix
 * @param Year
 * @return
 */
unit_static uint32_t DST_FixToAbsTime(const struct DSTFix_s *DSTFix, uint32_t Year) {
    if (DSTFix) {
        return DateTime_TotalSec(DSTFix->Day, DSTFix->Month, Year, DSTFix->Hour, DSTFix->Min, 0);
    } else {
        return 0;
    }
}

/**
 * Calculates fixed DST date from absolute time
 * @param DSTFix
 * @param AbsTime
 */
void DST_AbsTimeToFix(struct DSTFix_s *DSTFix, uint32_t AbsTime) {
    RTC_TIME TimeDSTFix;
    RTC_DATE DateDSTFix;

    AbsTime_ToTime(AbsTime, 0, &TimeDSTFix);
    AbsTime_ToDate(AbsTime, &DateDSTFix);

    DSTFix->Min = TimeDSTFix.minutes;
    DSTFix->Hour = TimeDSTFix.hours;
    DSTFix->Month = DateDSTFix.month;
    DSTFix->Day = DateDSTFix.day;
}

/**
 * Calculates Local time from UTC time using offset
 * @param UTCTime
 * @param TimeOffset
 * @param DSTActive
 * @return
 */
uint32_t DST_LocalFromUTC_DSTNone(uint32_t UTCTime, int32_t TimeOffset, uint8_t *DSTActive) {
    if (DSTActive) {
        *DSTActive = 0;
    }
    return UTCTime + TimeOffset;
}

/**
 * Calculates Local time from UTC time using fixed DST parameters
 * @param UTCTime
 * @param UseDST
 * @param TimeOffset offset in seconds
 * @param DSTFixWiSu
 * @param DSTFixSuWi
 * @param DSTActive
 * @return
 */
uint32_t DST_LocalFromUTC_DSTFix(uint32_t UTCTime, uint8_t UseDST, int32_t TimeOffset, const struct DSTFix_s *DSTFixWiSu, const struct DSTFix_s *DSTFixSuWi, uint8_t *DSTActive) {
    int dst;

    uint32_t SuWiAbs;
    uint32_t WiSuAbs;

    uint32_t Year;
    uint32_t Dummy;

    if (UseDST) { // DST scheme is used
        AbsTime_DayOfYear(UTCTime, &Year, &Dummy); //use only Year value

        //calculate local Wi/Su Absolute time
        WiSuAbs = DST_FixToAbsTime(DSTFixWiSu, Year);

        //calculate local Su/Wi Absolute time
        SuWiAbs = DST_FixToAbsTime(DSTFixSuWi, Year);

        // transform to fixed UTC changeover date and time
        WiSuAbs -= TimeOffset; // Wi/Su
        SuWiAbs -= TimeOffset + 3600; // Su/Wi including DST
        if (SuWiAbs > WiSuAbs) { //northern hemisphere
            dst = (UTCTime >= WiSuAbs) && (UTCTime < SuWiAbs);
        } else { //southern hemisphere
            dst = (UTCTime >= WiSuAbs) || (UTCTime < SuWiAbs);
        }

        // calculate DST
        if (dst) TimeOffset += 3600;
    } else {
        dst = 0; //no DST
    }
    if (DSTActive) {
        *DSTActive = dst;
    }

    return UTCTime + TimeOffset;
}

/**
 * Calculates Local time from UTC time using calculated DST parameters
 * @param UTCTime
 * @param UseDST
 * @param TimeOffset offset in seconds
 * @param DSTParamWiSu
 * @param DSTParamSuWi
 * @param DSTActive
 * @return
 */
uint32_t DST_LocalFromUTC_DSTParam(uint32_t UTCTime, uint8_t UseDST, int32_t TimeOffset, const struct DSTParam_s *DSTParamWiSu, const struct DSTParam_s *DSTParamSuWi, uint8_t *DSTActive) {
    struct DSTFix_s DSTFixWiSu, DSTFixSuWi;

    if (UseDST) {
        uint32_t Year;
        uint32_t Dummy;

        AbsTime_DayOfYear(UTCTime, &Year, &Dummy); //use only Year value

        //calculate fixed DST param
        DST_ParamToFix(DSTParamWiSu, Year, &DSTFixWiSu);

        //calculate fixed DST param
        DST_ParamToFix(DSTParamSuWi, Year, &DSTFixSuWi);
    }

    return DST_LocalFromUTC_DSTFix(UTCTime, UseDST, TimeOffset, &DSTFixWiSu, &DSTFixSuWi, DSTActive);
}

/**
 * Calculates Local time from UTC time using predefined zone table
 * @param UTCTime
 * @param TimeZone
 * @param DSTActive
 * @return
 */
uint32_t DST_LocalFromUTC_ZoneTable(uint32_t UTCTime, uint8_t TimeZone, uint8_t *DSTActive) {
    struct DSTParam_s DSTParamWiSu, DSTParamSuWi;

    int32_t TimeOffset = ((ZoneOffsets[TimeZone] & 0x7F) - 53)*15 * 60;

    if (ZoneOffsets[TimeZone] & 0x80) {
        DST_DecodeZoneTable(TimeZone, PART_WI_SU, &DSTParamWiSu);
        DST_DecodeZoneTable(TimeZone, PART_SU_WI, &DSTParamSuWi);

        return DST_LocalFromUTC_DSTParam(UTCTime, 1, TimeOffset, &DSTParamWiSu, &DSTParamSuWi, DSTActive);
    } else {
        return DST_LocalFromUTC_DSTNone(UTCTime, TimeOffset, DSTActive);
    }
}

/**
 * Calculate UTC from local time without DST
 * @param LocalTime
 * @param TimeOffset offset in seconds
 * @param DSTActive
 * @return
 */
uint32_t DST_UTCFromLocal_DSTNone(uint32_t LocalTime, int32_t TimeOffset, uint8_t *DSTActive) {
    if (DSTActive) {
        *DSTActive = 0;
    }
    return LocalTime - TimeOffset;
}

/**
 * Calculate UTC from local time using fixed DST
 * @param LocalTime
 * @param UseDST
 * @param TimeOffset offset in seconds
 * @param DSTFixWiSu
 * @param DSTFixSuWi
 * @param DSTActive
 * @return
 */
uint32_t DST_UTCFromLocal_DSTFix(uint32_t LocalTime, uint8_t UseDST, int32_t TimeOffset, const struct DSTFix_s *DSTFixWiSu, const struct DSTFix_s *DSTFixSuWi, uint8_t *DSTActive) {
    uint32_t SuWiAbs;
    uint32_t WiSuAbs;

    uint32_t Year;
    uint32_t Dummy;

    uint8_t dst;

    if (UseDST) { // DST scheme is used

        AbsTime_DayOfYear(LocalTime, &Year, &Dummy); //use only Year value

        //calculate local Wi/Su Absolute time
        WiSuAbs = DST_FixToAbsTime(DSTFixWiSu, Year);

        //calculate local Su/Wi Absolute time
        SuWiAbs = DST_FixToAbsTime(DSTFixSuWi, Year);

        if (SuWiAbs > WiSuAbs) { //northern hemisphere
            if ((LocalTime < WiSuAbs) || (LocalTime >= SuWiAbs)) dst = 0;
            SuWiAbs -= 3600;
            if ((LocalTime >= WiSuAbs)&&(LocalTime < SuWiAbs)) dst = 1;
        } else { //southern hemisphere
            if ((LocalTime < WiSuAbs)&&(LocalTime >= SuWiAbs)) dst = 0;
            SuWiAbs -= 3600;
            if ((LocalTime >= WiSuAbs) || (LocalTime < SuWiAbs)) dst = 1;
        }

        // calculate DST
        if (dst) TimeOffset += 3600;
    } else {
        dst = 0; //no DST
    }
    if (DSTActive) {
        *DSTActive = dst;
    }

    return LocalTime - TimeOffset;
}

/**
 * Calculate UTC from local time using calculated DST
 * @param LocalTime
 * @param UseDST
 * @param TimeOffset offset in seconds
 * @param DSTParamWiSu
 * @param DSTParamSuWi
 * @param DSTActive
 * @return
 */
uint32_t DST_UTCFromLocal_DSTParam(uint32_t LocalTime, uint8_t UseDST, int32_t TimeOffset, const struct DSTParam_s *DSTParamWiSu, const struct DSTParam_s *DSTParamSuWi, uint8_t *DSTActive) {

    if (UseDST) {
        struct DSTFix_s DSTFixWiSu, DSTFixSuWi;

        uint32_t Year;
        uint32_t Dummy;
        AbsTime_DayOfYear(LocalTime, &Year, &Dummy); //use only Year value
        DST_ParamToFix(DSTParamWiSu, Year, &DSTFixWiSu);
        DST_ParamToFix(DSTParamSuWi, Year, &DSTFixSuWi);

        return DST_UTCFromLocal_DSTFix(LocalTime, 1, TimeOffset, &DSTFixWiSu, &DSTFixSuWi, DSTActive);
    } else {
        return DST_UTCFromLocal_DSTNone(LocalTime, TimeOffset, DSTActive);
    }
}

/**
 * Calculates UTC time from Local time using time-zone table
 * @param LocalTime
 * @param TimeZone
 * @param LocalSummerSource
 * @param LocalSummer
 * @return
 */
uint32_t DST_UTCFromLocal_ZoneTable(uint32_t LocalTime, uint8_t TimeZone, uint8_t *DSTActive) {
    int32_t TimeOffset = ((ZoneOffsets[TimeZone] & 0x7F) - 53)*15 * 60;

    if (ZoneOffsets[TimeZone]&0x80) { // DST scheme is used
        struct DSTParam_s DSTParamWiSu, DSTParamSuWi;

        //calculate local Wi/Su Absolute time
        DST_DecodeZoneTable(TimeZone, PART_WI_SU, &DSTParamWiSu);
        DST_DecodeZoneTable(TimeZone, PART_SU_WI, &DSTParamSuWi);
        return DST_UTCFromLocal_DSTParam(LocalTime, 1, TimeOffset, &DSTParamWiSu, &DSTParamSuWi, DSTActive);
    } else {
        return DST_UTCFromLocal_DSTNone(LocalTime, TimeOffset, DSTActive);
    }
}

int Date_DaysInMonth(uint32_t Year, uint32_t Month) {
    int result;
    if (Month == 0) Month = 1;
    if (Month > 12) Month = 12;
    result = Months[Month - 1];
    if (Date_IsLeapMonth(Year, Month)) result++;
    return result;
}
