#ifndef SUNSET_SUNRISE_H
#define SUNSET_SUNRISE_H
#include <stdint.h>

enum SunPhaseType_e {
    SUN_POLARNIGHT,
    SUN_POLARDAY,
    SUN_NORMAL,
};


struct SunPhase_s {
    int16_t sunrise;
    int16_t sunset;
};

uint16_t Interpolate32(uint16_t data1, uint16_t data2, uint8_t weight);
uint32_t Interpolate8(uint32_t data1, uint32_t data2, uint8_t weight);

int32_t Div3_75(int32_t Input);

enum SunPhaseType_e Sun_PhaseTable(uint16_t DayOfYear, int16_t UtcOffsetMin, struct SunPhase_s * phase);
enum SunPhaseType_e Sun_Phase(uint16_t DayOfYear, int16_t UtcOffsetMin, struct SunPhase_s * phase, int Longitude, int Lattitude);

#endif // SUNSET_SUNRISE_H
