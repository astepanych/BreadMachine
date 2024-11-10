#pragma once
#include <stdint.h>

#define FlashAddrGlobalParams 0x08060000
#define FlashAddrPrograms 0x08068000
#define FlashAddrFlagBootloader 0x0806f000


constexpr uint16_t LenWifiSSID = 32;
constexpr uint16_t LenWifiPassword = 64;
#define IsStartBootloader  0xaaaaaaaa

#pragma pack(push,1)
struct RomParams {
	uint32_t crc32;
	float k1;
	float k2;
	uint32_t period;
	uint32_t timeoutAddWater;
	char wifiSSID[LenWifiSSID];
	char wifiPassword[LenWifiPassword];
	
};
#pragma pack(pop)
