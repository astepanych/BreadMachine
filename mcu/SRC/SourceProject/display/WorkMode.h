#pragma once
#include <stdint.h>
#include <string.h>
#ifdef QT_BUILD
#include <qtextcodec.h>
#endif

#define MaxLengthNameMode 24	//Максимальлная длина названия программы выпечки
#define MaxStageMode 4			//максимальное количество этапов

#define OffsetAddrNumPrograms 4		// смещение в байтах , где хранится количество программ
#define OffsetAddrPrograms 8		//смещение в байтах, откуда начинаются данные программ
constexpr int MagicNumber = 0x09abcdf9; // признак, что данные программ в памяти валидны

#define TO_SECONDS(x) (60 * (x))
#define MAX_SETTINGS_FUN_AND_DAMP 6

enum eStatesFan
{
    FanOff = 0,
    FanX1,
    FanX2,
};

#pragma pack(push, 1)
struct SettingsFanAndDamper
{
	SettingsFanAndDamper()
	{
		interval = 2;
		state = 0;
	}
	uint8_t interval;
	uint8_t state;
};
#pragma pack(pop)

/**
    @struct StageWorkMode
    @brief  Структура описывает этап рабочей программы
**/
struct StageWorkMode {
    StageWorkMode() {
        duration = 25;
        waterVolume = 1000;
        temperature = 250;
        waterVolume2 = 1000;
        watertimeout = 40;
	    memset(fan, 0, sizeof(SettingsFanAndDamper)*MAX_SETTINGS_FUN_AND_DAMP);
	    memset(damper, 0, sizeof(SettingsFanAndDamper)*MAX_SETTINGS_FUN_AND_DAMP);
    };
    uint16_t duration: 6; //!< продолжительность этапа
	uint16_t temperature : 10; //!< температура для текущего этапа
    int16_t waterVolume;  //!< объем выливаемой воды
    int16_t waterVolume2; //!< объем выливаемой воды
    int16_t watertimeout; //!< таймаут выливаемой воды
    
	SettingsFanAndDamper damper[MAX_SETTINGS_FUN_AND_DAMP];
	SettingsFanAndDamper fan[MAX_SETTINGS_FUN_AND_DAMP];
};
/**
    @struct WorkMode
    @brief  Структура описывает рабочий режим(программа выпекания)
**/
#pragma pack(push, 1)
struct WorkMode {
    char nameMode[MaxLengthNameMode]; //!< имя программы
    uint8_t lenNameMode; //!< длина имени программы
    uint8_t numStage; //!< количество этапов
    StageWorkMode stages[MaxStageMode]; //!< настройки этапов
    /**
        @brief произоводит сброс параметров рабочего режима 
    **/
    void reset() {
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
        stages[0].duration = 8;
        stages[0].waterVolume = 2000;
        stages[0].temperature = 200;
	    memset(stages[0].fan, 0, sizeof(SettingsFanAndDamper)*MAX_SETTINGS_FUN_AND_DAMP);
	    memset(stages[0].damper, 0, sizeof(SettingsFanAndDamper)*MAX_SETTINGS_FUN_AND_DAMP);

    }
};
#pragma pack(pop)
