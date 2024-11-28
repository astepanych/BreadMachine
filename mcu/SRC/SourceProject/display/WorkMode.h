#pragma once
#include <stdint.h>
#include <string.h>

#define MaxLengthNameMode 41
#define MaxStageMode 10

#define OffsetAddrNumPrograms 4
#define OffsetAddrPrograms 8
constexpr int MagicNumber = 0x09abcdf1;

struct StageWorkMode
{
	StageWorkMode()
	{
		duration = 600;
		waterVolume = 1000;
		temperature = 250;
		damper = 1;
		fan = 0;
	}
	;
	uint16_t duration;
	int16_t waterVolume;
	uint16_t temperature;
	uint16_t damper;
	uint16_t fan;
};
struct WorkMode
{
	char nameMode[MaxLengthNameMode];
	uint8_t lenNameMode;
	uint16_t numStage;
	StageWorkMode stages[MaxStageMode];
	void reset()
	{
		lenNameMode = convertUtf8ToCp1251("НОВАЯ ПРОГРАММА", nameMode);
		numStage =  1;
		memset(stages, 0, sizeof(StageWorkMode)*MaxStageMode);
		stages[0].duration = 600;
		stages[0].waterVolume = 2000;
		stages[0].temperature = 200;
		
		
	}
};
