#include "lib/datetime_utils.h"
#include "lib/sunrise_sunset.h"

#define LATTITUDE   (49*60+16) //49deg16
#define LONGTITUDE  (16*60+59) //16deg59

bool calulateOutputState(uint8_t day, uint8_t month, uint8_t year, uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    uint8_t dst;
    int16_t UTCsec;
    struct AbsTime_s time;
    bool _state = false;
    RTC_TIME tm;
    RTC_DATE dt;

    time.Millisec = 0;
    time.Sec = DateTime_TotalSec(day, month, year, hours, minutes, seconds);

    AbsTime_ToTime(time.Sec, time.Millisec, &tm);
    AbsTime_ToDate(time.Sec, &dt);

    UTCsec = time.Sec;

    time.Sec = DST_LocalFromUTC_ZoneTable(time.Sec, 2, &dst);
    UTCsec = time.Sec - UTCsec;

    struct SunPhase_s phase;
    Sun_Phase(Date_DayOfYear(dt.day, dt.month, dt.year), UTCsec / 60, &phase, LONGTITUDE, LATTITUDE);
    
    AbsTime_ToTime(time.Sec, time.Millisec, &tm);
    
    if ((tm.minutes + tm.hours * 60) >= phase.sunrise)
        _state = false;
    else
        _state = true;
    if ((tm.minutes + tm.hours * 60) >= phase.sunset)
        _state = true;

    return _state;
}
