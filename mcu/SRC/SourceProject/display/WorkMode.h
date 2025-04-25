#pragma once
#include <stdint.h>
#include <string.h>
#ifdef QT_BUILD
#include <qtextcodec.h>
#endif

#define MaxLengthNameMode 24
#define MaxStageMode 7

#define OffsetAddrNumPrograms 4
#define OffsetAddrPrograms 8
constexpr int MagicNumber = 0x09abcdf3;

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
	uint16_t temperature:14;
	uint16_t damper : 1;
	uint16_t fan : 1;
};
struct WorkMode
{
	char nameMode[MaxLengthNameMode];
	uint8_t lenNameMode:5;
	uint8_t numStage:3;
	StageWorkMode stages[MaxStageMode];
	void reset()
    {
        #ifdef QT_BUILD
        QString s = QTextCodec::codecForName("utf-8")->toUnicode("НОВАЯ ПРОГРАММА");
        QByteArray ba = QTextCodec::codecForName("Windows-1251")->fromUnicode(s);
        memset(nameMode, 0, MaxLengthNameMode);
        lenNameMode = ba.length();
        memcpy(nameMode, ba.data(), lenNameMode);
        #else
		lenNameMode = convertUtf8ToCp1251("НОВАЯ ПРОГРАММА", nameMode);
        #endif
		numStage =  1;
		memset(stages, 0, sizeof(StageWorkMode)*MaxStageMode);
		stages[0].duration = 600;
		stages[0].waterVolume = 2000;
		stages[0].temperature = 200;
        stages[0].fan = 1;
	}
};
