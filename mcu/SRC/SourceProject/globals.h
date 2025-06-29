#pragma once
#include <stdint.h>

#define FlashAddrGlobalParams 0x08060000
#define FlashAddrPrograms 0x08068000

#define EepromAddrGlobalParams  0x0000
#define EepromAddrProgramsAttribute  0x0100
#define EepromPageSize  0x010
#define EepromAddrPrograms  (EepromAddrProgramsAttribute + EepromPageSize)



#pragma pack(push,1)
struct RomParams {
	uint32_t crc32;
	float k1;
	float k2;
	uint32_t period;
	uint32_t timeoutAddWater;
	char wifiSSID[LenWifiSSID];
	char wifiPassword[LenWifiPassword];
	uint32_t stateWifi;
	uint8_t numSound;
	uint8_t volume;
	
};
#pragma pack(pop)
