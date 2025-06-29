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
constexpr int MagicNumber = 0x09abcdf3; // признак, что данные программ в памяти валидны

/**
    @struct StageWorkMode
    @brief  Структура описывает этап рабочей программы
**/
struct StageWorkMode {
    StageWorkMode() {
        duration = 600;
        waterVolume = 1000;
        temperature = 250;
        damper = 1;
        fan = 0;
    }
    ;
    uint16_t duration; //!< продолжительность этапа
    int16_t waterVolume;  //!< объем выливаемой воды
    uint16_t temperature : 14; //!< температура для текущего этапа
    uint16_t damper : 1; //!< состояние шибера
    uint16_t fan : 1;  //!< состояние вентилятора
};
/**
    @struct WorkMode
    @brief  Структура описывает рабочий режим(программа выпекания)
**/
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
