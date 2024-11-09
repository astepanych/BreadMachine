#pragma once

#include "common_project.h"

extern void initTcpServer(fnxProcessRxData cbFunc);
extern int sendDataNetwork(const uint8_t* data, const uint8_t len);