#pragma once
#include "stdint.h"
#include "common_project.h"


struct WifiState {
    uint32_t state;
    char wifiSSID[LenWifiSSID];
    char wifiPassword[LenWifiPassword];
};
extern WifiState gWifiState;
extern void initProto();