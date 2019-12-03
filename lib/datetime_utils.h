#ifndef __DATETIME_UTILS_H_
#define __DATETIME_UTILS_H_

#include <stdint.h>

#define GET_LEAP_DAYS(Year)                     ((uint32_t)(Year) / 4 - (Year) / 100 + (Year) / 400)
#define GET_ELAPSED_DAYS(Year)                  ((uint32_t)(Year) * 365 + GET_LEAP_DAYS(Year))

#define ABSTIMESTARTYEAR                        2000
#define NTPSTARTYEAR                            1900
#define MOBASTARTYEAR                           1993
#define SNMPSTARTYEAR                           1970

#define ABSTIME_DAY_CORR                        (GET_ELAPSED_DAYS(ABSTIMESTARTYEAR))
// 1.1.2000 was saturday
#define ABSTIME_START_DAY                       6
#define ABSTIME_NTP_CORR                        ((uint32_t)((uint64_t)(ABSTIME_DAY_CORR - GET_ELAPSED_DAYS(NTPSTARTYEAR)) * 86400))
#define ABSTIME_MOBA_CORR                       ((uint32_t)((uint64_t)(ABSTIME_DAY_CORR - 1 - GET_ELAPSED_DAYS(MOBASTARTYEAR)) * 86400))
#define ABSTIME_SNMP_CORR                       ((uint32_t)((uint64_t)(ABSTIME_DAY_CORR - 1 - GET_ELAPSED_DAYS(SNMPSTARTYEAR)) * 86400))

#define TIMEZONE_TABLE_VERSION	10

#define MINYEAR					1900
#define MAXYEAR					3000

#define ZONETYPE_TABLE                          0
#define ZONETYPE_USERNODST                      1
#define ZONETYPE_USERFIXDST                     2
#define ZONETYPE_USERCALCDST                    3

#define LOCALSUMMERSOURCE_LINE                  0
#define LOCALSUMMERSOURCE_TABLE                 1

#define CALCDSTWHICH_FIXED                      0
#define CALCDSTWHICH_LAST                       5
#define CALCDSTWHICH_PRELAST                    6
#define CALCDSTWHICH_AFTER15                    7
#define CALCDSTWHICH_PREPRELAST                 8

/* Structs --------------------------------------------------------------------*/

typedef struct {
    uint8_t century;
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t weekday;
} RTC_DATE;

typedef struct {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint16_t milliseconds;
} RTC_TIME;

struct AbsTime_s {
    uint32_t Sec;
    uint16_t Millisec;
};

enum _CompareTime_e {
    LESS = -1,
    EQUALS = 0,
    MORE = 1,
};
typedef enum _CompareTime_e CompareTime_e;

enum _CompareTimeResolution_e {
    HOURS,
    MINUTES,
    SECONDS,
    MILLISECONDS
};
typedef enum _CompareTimeResolution_e CompareTimeResolution_e;

struct DSTParam_s {
    uint8_t Which; //kolikaty
    uint8_t Day; //den v tydnu nebo den mesice (fix)
    uint8_t Month; //mesic
    uint8_t Hour; //hodina
    uint8_t Min; //minutes
} __attribute__((packed));

struct DSTFix_s {
    uint8_t Min; //minuta
    uint8_t Hour; //hodina
    uint8_t Month; //mesic
    uint8_t Day; //den
} __attribute__((packed));

/*
struct ZoneCache_s {
    uint32_t UTCTime; //absolute UTC time
    uint32_t LocalTime; //absolute local time
    uint8_t TimeZone; //time zone
    uint8_t UseDST; //use DST scheme if available
    RTC_TIME RealTime;
    RTC_DATE RealDate;
};
 */

void AbsTime_Copy(struct AbsTime_s *Dest, const struct AbsTime_s *Source);
void AbsTime_Set(struct AbsTime_s *Acc, uint32_t Sec, uint16_t Millisec);
void AbsTime_Ceil(struct AbsTime_s *Acc, uint32_t Divisor);
void AbsTime_Floor(struct AbsTime_s *Acc, uint32_t Divisor);
void AbsTime_Increment(struct AbsTime_s *Acc, int32_t Millisec);
void AbsTime_NormTime(struct AbsTime_s *Acc);
void AbsTime_Add(struct AbsTime_s *Acc, const struct AbsTime_s *Diff);
void AbsTime_Sub(struct AbsTime_s *Acc, const struct AbsTime_s *Diff);
void AbsTime_Div2(struct AbsTime_s *Acc);
uint32_t AbsTime_TotalMillisec(struct AbsTime_s *Acc);
CompareTime_e AbsTime_Compare(const struct AbsTime_s *Time, const struct AbsTime_s *Comp);
int AbsTime_CompareWithinMargin(const struct AbsTime_s *Time, const struct AbsTime_s *Margin, const struct AbsTime_s *Comp);

uint32_t Date_LeapDays(uint32_t Year);
uint32_t Date_ElapsedDays(uint32_t Year);
int Date_IsLeapYear(uint32_t Year);

int Date_WeekDayFromDayCount(uint32_t DayCountVal);
int Date_WeekDay(uint32_t day, uint32_t month, uint32_t year);

uint32_t Date_DayCount(uint32_t Day, uint32_t Month, uint32_t Year);
uint32_t Time_SecCount(uint32_t Hour, uint32_t Min, uint32_t Sec);
uint32_t Date_DayOfYear(uint32_t Day, uint32_t Month, uint32_t Year);
uint32_t DateTime_TotalSec(uint16_t Day, uint16_t Month, uint16_t Year, uint16_t Hour, uint16_t Min, uint16_t Sec);
uint32_t DateTime_TotalSecFromCount(uint32_t DayCountVal, uint32_t SecCountVal);
void Date_DayOfYearToDate(uint32_t Year, uint32_t DayOfYear, RTC_DATE *DateObject);

void AbsTime_ToTime(uint32_t AbsVal, uint16_t Millisec, RTC_TIME *TimeObject);
void AbsTime_DayOfYear(uint32_t AbsVal, uint32_t *Year, uint32_t *DayOfYear);
int AbsTime_ISOWeek(uint32_t AbsVal);
void AbsTime_ToDate(uint32_t AbsVal, RTC_DATE *DateObject);
int Date_IsLeapMonth(uint32_t Year, uint32_t Month);

CompareTime_e Date_Compare(const RTC_DATE * DateA, const RTC_DATE * DateB);
CompareTime_e Time_Compare(const RTC_TIME * TimeA, const RTC_TIME * TimeB, CompareTimeResolution_e resolution);

uint32_t AbsTime_SecCount(uint32_t AbsVal);
uint32_t AbsTime_DayCount(uint32_t AbsVal);

void DST_AbsTimeToFix(struct DSTFix_s *DSTFix, uint32_t AbsTime);

uint32_t DST_UTCFromLocal_DSTNone(uint32_t LocalTime, int32_t TimeOffset, uint8_t *DSTActive);
uint32_t DST_UTCFromLocal_DSTFix(uint32_t LocalTime, uint8_t UseDST, int32_t TimeOffset, const struct DSTFix_s *DSTFixWiSu, const struct DSTFix_s *DSTFixSuWi, uint8_t *DSTActive);
uint32_t DST_UTCFromLocal_DSTParam(uint32_t LocalTime, uint8_t UseDST, int32_t TimeOffset, const struct DSTParam_s *DSTParamWiSu, const struct DSTParam_s *DSTParamSuWi, uint8_t *DSTActive);
uint32_t DST_UTCFromLocal_ZoneTable(uint32_t LocalTime, uint8_t TimeZone, uint8_t *DSTActive);

int Date_DaysInMonth(uint32_t Year, uint32_t Month);

#if UNIT_TEST
void DST_DecodeZoneTable(int TimeZone, int Part, struct DSTParam_s *DSTParam);
void DST_ParamToFix(const struct DSTParam_s *DSTParam, uint32_t Year, struct DSTFix_s *DSTFix);
uint32_t DST_FixToAbsTime(const struct DSTFix_s *DSTFix, uint32_t Year);
#endif

// ZONETYPE_USERNODST
uint32_t DST_LocalFromUTC_DSTNone(uint32_t UTCTime, int32_t TimeOffset, uint8_t *DSTActive);
// ZONETYPE_USERFIXDST
uint32_t DST_LocalFromUTC_DSTFix(uint32_t UTCTime, uint8_t UseDST, int32_t TimeOffset, const struct DSTFix_s *DSTFixWiSu, const struct DSTFix_s *DSTFixSuWi, uint8_t *DSTActive);
// ZONETYPE_USERCALCDST
uint32_t DST_LocalFromUTC_DSTParam(uint32_t UTCTime, uint8_t UseDST, int32_t TimeOffset, const struct DSTParam_s *DSTParamWiSu, const struct DSTParam_s *DSTParamSuWi, uint8_t *DSTActive);
// ZONETYPE_TABLE
uint32_t DST_LocalFromUTC_ZoneTable(uint32_t UTCTime, uint8_t TimeZone, uint8_t *DSTActive);

#define USE_OLD_TIME_API 1
#if USE_OLD_TIME_API

#define CopyTime                    AbsTime_Copy
#define SetTime                     AbsTime_Set
#define CeilTime                    AbsTime_Ceil
#define FloorTime                   AbsTime_Floor
#define IncrementTime               AbsTime_Increment
#define NormTime                    AbsTime_NormTime
#define AddTime                     AbsTime_Add
#define SubTime                     AbsTime_Sub
#define TimeDiv2                    AbsTime_Div2
#define GetMillisec                 AbsTime_TotalMillisec
#define CompareTime                 AbsTime_Compare
#define CompareTimeWithMargin       AbsTime_CompareWithinMargin
#define GetComponentTime            AbsTime_ToTime
#define GetYearAndDayOfYearFromAbs  AbsTime_DayOfYear
#define GetISOWeekFromAbs           AbsTime_ISOWeek
#define GetComponentDate            AbsTime_ToDate
#define GetSecCountFromAbs          AbsTime_SecCount
#define GetDayCountFromAbs          AbsTime_DayCount

#define GetLeapDays                 Date_LeapDays
#define GetElapsedDays              Date_ElapsedDays
#define IsLeapYear                  Date_IsLeapYear
#define WeekDayFromDayCount         Date_WeekDayFromDayCount
#define DayOfWeek                   Date_WeekDay
#define DayCount                    Date_DayCount
#define NumberOfDaysInMonth         Date_DaysInMonth
#define IsLeapMonth                 Date_IsLeapMonth
#define DayOfYearToComponentDate    Date_DayOfYearToDate
#define GetDayOfYear                Date_DayOfYear

#define SecCount                    Time_SecCount

#define GetAbsTime                  DateTime_TotalSec
#define GetAbsTimeFromCount         DateTime_TotalSecFromCount

#define CalcDSTParamFixToAbsDSTFix  DST_AbsTimeToFix
#define GetUTCFromLocal             DST_UTCFromLocal_@@@
#define GetLocalFromUTC             DST_LocalFromUTC_@@@
#define NegTime                     @@@
#define IsTimeNegative              @@@

#endif

#endif /* __DATETIME_UTILS_H_ */
