#pragma once
#include <stdint.h>

#define MaxLengthNameMode 41
#define MaxStageMode 10
#define FlashAddrPrograms 0x080e0000
#define OffsetAddrNumPrograms 4
#define OffsetAddrPrograms 8
constexpr int MagicNumber = 0x08abcdfe;

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
	uint16_t waterVolume;
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
	
};
