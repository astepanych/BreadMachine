#pragma once
#include <stdint.h>
#include <Uart3.h>
#include <string>

constexpr uint16_t CmdSoftVersion = 0x2000;	
constexpr uint16_t CmdDateTime = 0x2014;
constexpr uint16_t CmdSetDateTime = 0x009c;
constexpr uint16_t CmdNumProgramm = 0x4000;
constexpr uint16_t addrMainItem = 0x2230;

constexpr uint16_t CmdCofirm = 0x4f4b;

enum ReturnCodeKey
{
	ReturnCodeKeyStart = 0x1000,
	ReturnCodeKeyCancelSettings,
	ReturnCodeKeyApplySettings,
	ReturnCodeKeyStop,
	ReturnCodeKeySaveProgramm,
	ReturnCodeKeyUpSelectProgramm,
	ReturnCodeKeyDownSelectProgramm,
	ReturnCodeKeyInMenuListProgramms
};


class DisplayDriver
{
	enum StateParcePacketDisplay
	{
		StateWaitByte1,
		StateWaitByte2,
		StateWaitLen,
		StateReadByte
		
	};
public:
	DisplayDriver();
	static DisplayDriver* instance() {return m_instance;};
	static void initDisplay();	
	
	static void putByte(const uint8_t byte); 
	static void sendToDisplay(uint16_t id, uint8_t len, uint8_t *data);
	static void sendToDisplay(uint16_t id, std::wstring &str);
	static void sendToDisplay(uint16_t id, uint16_t data);
	
	static std::function<void(const uint16_t, uint8_t*)> newCmd;
	
	static void reset();
	
private:
	static DisplayDriver *m_instance;
	static Uart3 *uart3;
	
	static const uint8_t startByte1 = 0x5a;
	static const uint8_t startByte2 = 0xa5;
	static const uint8_t cmdByteWrite = 0x82;
	static const uint8_t cmdByteRead = 0x83;
	
	static uint8_t bufParce[SizeBuffer];
	static uint8_t lenPacket;
	static uint8_t currentIndex;
	
	//display 
	static void taskDisplay(void *p);
	
	static void parsePackFromDisplay(uint8_t len, uint8_t *data);
		
	static int cntPutByte;
	static xQueueHandle xQueueDisplay;
	static xSemaphoreHandle xSemDisplay;
	TaskHandle_t xHandleDisplay = NULL;
	static StateParcePacketDisplay statePacket;
	


};

