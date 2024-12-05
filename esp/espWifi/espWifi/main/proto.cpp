#include "dataexchenge.h"
#include "tcp_server.h"
#include "uart1_driver.h"
#include <string.h>

#include "esp_log.h"
#include "vector"
#include "proto.h"

std::vector<uint8_t> tcpBuf;
const static char *TAG = "TCP";

WifiState gWifiState;

void putDataFromNetwork(const uint8_t* data, const int length) {
	
	tcpBuf.insert(tcpBuf.end(), data, data + length);
	
	PackageNetworkFormat *p;
	while (tcpBuf.size() >= sizeof(PackageNetworkFormat))
	{
		p = (PackageNetworkFormat*)tcpBuf.data();
	//	ESP_LOGI(TAG, "len = %d,ID = 0x%04x, %d", p->dataSize, p->cmdId, tcpBuf.size());
		if (p->cmdId >= ID_HOST_EXTERN) {
			objDataExchenge.sendPackage(p->cmdId, p->msgType, p->dataSize, (uint8_t*)p->data);
		}
		tcpBuf.erase(tcpBuf.begin(), tcpBuf.begin()+sizeof(PackageNetworkFormat));
	}
	
}


void putDataFromUart(const uint8_t* data, const int length) {
	objDataExchenge.putByte(data, length);
}
extern void updateStateWifi();
void procUartData(const PackageNetworkFormat&p) {
	//ESP_LOGI(TAG, "uart ID = 0x%04x", p.cmdId);
	if (p.cmdId >= ID_HOST_EXTERN) {
		sendDataNetwork((uint8_t*)&p, sizeof(PackageNetworkFormat));
		return;
	}
	switch (p.cmdId) {
		case IdWifiPassword:
			memcpy(gWifiState.wifiPassword, p.data, p.dataSize);
			updateStateWifi();
			break;
		case IdWifiSSID:
			memcpy(gWifiState.wifiSSID, p.data, p.dataSize);
			break;
		case IdWifiState:
			
			memcpy(&gWifiState.state, p.data, sizeof(uint32_t));
			updateStateWifi();
			
			break;
		default:
			break;
	}
}

void sendTest() {
	uint8_t arr[32];
	memset(arr, 0, 32);
	memset(arr, 0x66, 6);
	
	
}

void initProto() {
	
	initTcpServer(putDataFromNetwork);
	initUart(putDataFromUart);
	objDataExchenge.sendBytes = sendDataToUart;
	objDataExchenge.procPack = procUartData;
	tcpBuf.reserve(sizeof(PackageNetworkFormat)*10);
//	sendTest();
}

