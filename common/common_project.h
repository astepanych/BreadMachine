#pragma once

#include <stdint.h>
/*!
 * тип описывающий указатель на функцию для обработки принятых данных по уарту
 */
typedef void(*fnxProcessRxData)(const uint8_t* data, const int length);

#define BODY_WORD_COUNT  16
//структура, описывающая пакет обмена по сети с радиостнцией
typedef struct {
    uint16_t msgType; //тип сообщения
    uint16_t cmdId; //идентификатор сообщения
    uint16_t counter; //счетчик сообщения
    uint16_t dataSize; //длина сообщения
    uint16_t data[BODY_WORD_COUNT]; //данные
    uint16_t crc; //котрольная сумма
} PackageNetworkFormat;


#define ID_HOST_ESP         0
#define ID_ESP_EXTERN		128
#define ID_HOST_EXTERN      256




enum StateCrc{
    StateCrcOk,
    StateCrcBad,
    StateCrcNoRcv
};

enum IdHostEsp
{
	IdBootHost = ID_HOST_ESP,
	IdWifiState,
	IdWifiSSID,
	IdWifiPassword
};

enum IdHostExtern{
    IdSoftVersion = ID_HOST_EXTERN,
	IdStartBootloader,
	IdDoneBootloader,
    IdFirmwareStart,
	IdFirmwareData,
	IdFirmwareCrc,
	IdFirmwareResult,
	IdResetMcu,
	IdConnectExternSoft,
	IdGetLog,
	IdDataLog,
    IdEndLog
};


constexpr uint16_t LenWifiSSID = 32;
constexpr uint16_t LenWifiPassword = 64;
