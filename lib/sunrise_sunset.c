#include "sunrise_sunset.h"

#include <stdint.h>

static const uint8_t Tangens[11] = {0, 4, 9, 14, 20, 27, 36, 47, 66, 98, 181}; // x 32, index from 0 to 10, range from 0 to 80°, values are negative in range 0 to -80°, 8° step
static const uint8_t SunCorrect[11] = {32, 32, 33, 35, 38, 42, 48, 57, 73, 103, 184}; // index from 0 to 10, range from 0 to 80°, same values are in range 0 to -80°, 8° step
// correction of sun position for -50'
static const uint8_t Arccos[34] = {180, 176, 173, 169, 166, 162, 158, 155, 151, 147, 144, 140, 136, 132, 128, 124, 120, 116, 112, 107, 103, 98, 93, 88, 83, 77, 71, 65, 58, 50, 41, 29, 0, 0};
// x 30, index from 0 to 32 (data no.33 - dummy value for interpolation), range from 0 to 1, 0.03125 units step
// arg>=0 then arccos(arg)=Arccos[arg]
// arg<0 then arccos(arg)=360-Arccos[-arg]

static const uint8_t WozMoz[47] = {84, 71, 60, 51, 46, 44, 45, 49, 55, 63, 73, 83, 92, 101, 107, 112, 115, 115, 113, 108, 103, 96, 90, 84, 79, 76, 75, 76, 81, 87,
    96, 107, 118, 130, 141, 151, 159, 164, 166, 165, 160, 152, 141, 127, 112, 97, 81};
// x 240 + offset 100, resolution in min/4, index from 0 to 46, range from day 0 (1.January) to day 368 (31.December +3 or +4 days), 8 days step

/*  //original data without correction
const char TgDeclination[47]={-109,-104,-97,-89,-79,-67,-55,-41,-27,-13,1,16,30,44,57,69,80,90,98,104,108,111,111,108,103,
                              96,87,77,65,52,39,25,11,-4,-18,-32,-46,-59,-71,-82,-92,-100,-106,-110,-111,-110,-107};
        // x 256, index from 0 to 46, range from day 0 (1.January) to day 368 (31.December +3 or +4 days), 8 days step
 */

static const int8_t TgDeclination[47] = {-106, -101, -93, -85, -74, -62, -51, -38, -26, -13, 1, 16, 30, 44, 57, 69, 80, 90, 98, 104, 108, 111, 111, 108, 103,
    96, 87, 77, 65, 53, 43, 30, 15, 2, -10, -20, -34, -45, -59, -71, -82, -92, -100, -106, -110, -110, -107};
// x 256, index from 0 to 46, range from day 0 (1.January) to day 368 (31.December +3 or +4 days), 8 days step
// data corrected according to http://www.csgnetwork.com/sunriseset.html at 22.6.2006 by ml

/**
 * Interpolate between data1/data2 using 33 steps
 * @param data1
 * @param data2
 * @param weight number between 0..32
 * @return result * 32
 */
uint16_t Interpolate32(uint16_t data1, uint16_t data2, uint8_t weight) {
    return weight * (data2 - data1) +  data1 * 32L;
}

/**
 * Interpolate between data1/data2 using 9 steps
 * @param data1
 * @param data2
 * @param weight number between 0..8
 * @return result * 2
 */
uint32_t Interpolate8(uint32_t data1, uint32_t data2, uint8_t weight) {
    return (weight * (data2 - data1) +  data1 * 8L + 2) / 4;
}

/**
 * GetInterpolated value from array of constants
 * @param ArrayPtr
 * @param Address
 * @return
 */
static uint16_t GetInterpolatedValue(const uint8_t *array, uint16_t Argument) {
    uint16_t result;
    uint8_t Weight;
    uint8_t Index;

    Index = Argument / 32;
    Weight = Argument % 32;
    result = Interpolate32(array[Index], array[Index + 1], Weight); //returns result * 32
    return (result + 16) / 32;
}

/*---------------- Get Sun difference time --------------------------------*/
static uint16_t GetSunDifferenceTime(int32_t DayIndex, int32_t TgLat, int32_t SunCorrection) {
    int32_t Argument, result;

    Argument = -TgDeclination[DayIndex] * TgLat - SunCorrection;

    if (Argument<-8192) {
        Argument = -8192; //sun doesn't set - polar day - relay stays switched-off
    }
    if (Argument > 8192) {
        Argument = 8192; //sun doesn't rise - polar night - relay stays switched-on
    }

    if (Argument < 0) { //arg<0 then arccos(arg)=360-Arccos[-arg]
        result = 360 - GetInterpolatedValue(Arccos, -Argument / 8); // min*2
    } else { //arg>=0 then arccos(arg)=Arccos[arg]
        result = GetInterpolatedValue(Arccos, Argument / 8); // min*2
    }

    return result; // min*2
}

/**
 * Check range of calculate Time
 * @param value number of minutes
 * @return value in range <0, 1440) (one day)
 */
static int16_t CheckSunPhaseTimeRange(int16_t value) {
    if (value < 0) value += 1440;
    if (value >= 1440) value -= 1440;
    return value;
}

int32_t Div3_75(int32_t Input) {
    return Input * 4 / 15;
}

/**
 * Calculate Sunrise and Sunset Time
 * @param DayNum
 * @param UtcOffsetMin
 * @param phase resulted sunrise and sunset in minutes
 * @param Lattitude lattitude in minutes
 * @param Longitude longitude in minutes
 * @return phase type polar night, polar day or normal
 */
enum SunPhaseType_e Sun_Phase(uint16_t DayOfYear, int16_t UtcOffsetMin, struct SunPhase_s * phase, int Longitude, int Lattitude) {
    int8_t DayIndex, DayWeight;
    int32_t TempW1, TempW2, SunCorrection, TgLat, Noon;

    if (DayOfYear > 0) {
        DayOfYear--;
    }

    DayIndex = DayOfYear / 8;
    DayWeight = DayOfYear % 8;

    Lattitude = Lattitude / 15; //degree/4, 15 angle minutes resolution
    if (Lattitude < 0) {
        TgLat = -GetInterpolatedValue(Tangens, -Lattitude); //southern hemisphere -tg(Abs(Lattitude))
    } else {
        TgLat = GetInterpolatedValue(Tangens, Lattitude); // tg(Abs(Lattitude))
    }

    Longitude = Div3_75(Longitude); //degree/16, 3.75 angle minutes resolution

    SunCorrection = 4 * GetInterpolatedValue(SunCorrect, Lattitude > 0 ? Lattitude : -Lattitude); //4*SunCorrection(Abs(Lattitude))

    TempW1 = GetSunDifferenceTime(DayIndex, TgLat, SunCorrection);
    TempW2 = GetSunDifferenceTime(DayIndex + 1, TgLat, SunCorrection);

    TempW1 = ((Interpolate32(TempW1, TempW2, DayWeight * 4)) + 2) / 4; //Sun Difference time in min/4

    Noon = 2880 - (GetInterpolatedValue(WozMoz, DayOfYear * 4) - 100); //corrected Noon time value in min/4

    Noon = Noon - Longitude + UtcOffsetMin * 4; //Noon time shifted by Longitude

    phase->sunrise = CheckSunPhaseTimeRange((Noon - TempW1 + 2) / 4); // sunrise time in minutes
    phase->sunset = CheckSunPhaseTimeRange((Noon + TempW1 + 2) / 4); // sunset time in minutes

    if (TempW1 < 120) {
        return SUN_POLARNIGHT;
    } else if (TempW1 > (2880 - 60)) {
        return SUN_POLARDAY;
    } else {
        return SUN_NORMAL;
    }
}

static const uint8_t SunriseTable[47] = {
    210, 208, 206, 202, 196, 190, 183, 175, 167, 158,
    150, 141, 132, 124, 116, 108, 102, 96, 91, 88,
    86, 85, 86, 89, 92, 97, 102, 108, 114, 120, 125,
    131, 137, 143, 149, 156, 162, 168, 175, 182, 188,
    194, 200, 204, 208, 209, 210
};
//Sunrise times in UTC, units= min/2, start from 1.1., 8-days step

static uint16_t SunsetTable(uint8_t index) {
    uint16_t result;
    if (index < 44) {
        index = 43 - index;
    } else {
        index = index - 44;
    }
    result = 659 - SunriseTable[index];
    return result;
}

/**
 * 
 * @param DayNum
 * @param UtcOffsetMin
 * @param phase resulted sunrise and sunset in minutes
 * @return phase type polar night, polar day or normal
 */
enum SunPhaseType_e Sun_PhaseTable(uint16_t DayOfYear, int16_t UtcOffsetMin, struct SunPhase_s * phase) {
    uint8_t Index, Weight;

    if (DayOfYear > 0) {
        DayOfYear--;
    }

    Index = DayOfYear / 8;
    Weight = DayOfYear % 8;

    phase->sunrise = Interpolate8(SunriseTable[Index], SunriseTable[Index + 1], Weight);
    phase->sunset = Interpolate8(SunsetTable(Index), SunsetTable(Index + 1), Weight);

    phase->sunrise += UtcOffsetMin;
    phase->sunset += UtcOffsetMin;

    return SUN_NORMAL;
}
