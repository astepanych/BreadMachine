#pragma once
#include <stdint.h>
#include <string.h>
#ifdef QT_BUILD
#include <qtextcodec.h>
#endif

#define MaxLengthNameMode 24	//Максимальлная длина названия программы выпечки
#define MaxStageMode 7			//максимальное количество этапов

#define OffsetAddrNumPrograms 4		// смещение в байтах , где хранится количество программ
#define OffsetAddrPrograms 8		//смещение в байтах, откуда начинаются данные программ
constexpr int MagicNumber = 0x09abcdf9; // признак, что данные программ в памяти валидны

enum eStatesFan
{
    FanOff = 0,
    FanX1,
    FanX2,
};

/**
    @struct StageWorkMode
    @brief  Структура описывает этап рабочей программы
**/
struct StageWorkMode {
    StageWorkMode() {
        duration = 600;
        waterVolume = 1000;
        temperature = 250;
        waterVolume2 = 1000;
        watertimeout = 40;
        damper = 1;
        fan = 0;
    }
    ;
    uint16_t duration; //!< продолжительность этапа
    int16_t waterVolume;  //!< объем выливаемой воды
    int16_t waterVolume2; //!< объем выливаемой воды
    int16_t watertimeout; //!< объем выливаемой воды
    uint16_t temperature : 10; //!< температура для текущего этапа
    uint16_t damper : 4; //!< состояние шибера
    uint16_t fan : 2;  //!< состояние вентилятора
};
/**
    @struct WorkMode
    @brief  Структура описывает рабочий режим(программа выпекания)
**/
#pragma pack(push, 1)
struct WorkMode {
    char nameMode[MaxLengthNameMode]; //!< имя программы
    uint8_t lenNameMode : 5; //!< длина имени программы
    uint8_t numStage : 3; //!< количество этапов
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
        stages[0].duration = 600;
        stages[0].waterVolume = 2000;
        stages[0].temperature = 200;
        stages[0].fan = 1;
    }
};
#pragma pack(pop)
