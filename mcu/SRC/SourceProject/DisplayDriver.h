#pragma once
#include <stdint.h>
#include <Uart3.h>
#include <Uart1.h>
#include <string>
#include "typedef.h"

constexpr uint16_t CmdSoftVersion = 0x2000;	

constexpr uint16_t AddrDamper = 0x1200;
constexpr uint16_t AddrFan = 0x1202;

constexpr uint16_t CmdDateTime = 0x2014;
constexpr uint16_t CmdSetDateTime = 0x009c;
constexpr uint16_t CmdNumProgramm = 0x4000;
constexpr uint16_t AddrScrollMainList = 0x4002;
constexpr uint16_t AddrProgressBar = 0x4004;
constexpr uint16_t AddrNumStage = 0x4006;
constexpr uint16_t AddrNumWater = 0x4008;
constexpr uint16_t AddrNumTemperature = 0x400a;
constexpr uint16_t AddrNumTemperatureMeasure = 0x400C;
constexpr uint16_t AddrNumDamper = 0x400E;
constexpr uint16_t AddrNumFan = 0x4010;
constexpr uint16_t AddrNumTime = 0x4012;

constexpr uint16_t AddrStages = 0x7000;
constexpr uint16_t CmdPaintFillRectangle = 0x0004;	
constexpr uint16_t addrMainItem = 0x2220;
constexpr uint16_t addrNameProg = 0x2280;

constexpr uint16_t CmdCofirm = 0x4f4b;

struct Rectangle
{
	u16be beginX;
	u16be beginY;
	u16be endX;
	u16be endY;
	u16be color;
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
	static void sendToDisplay1(uint16_t id, uint8_t len, uint8_t *data);
	static void sendToDisplay(const uint16_t id, const std::string &str);
	static void sendToDisplay(uint16_t id, uint16_t data);
	
	static std::function<void(const uint16_t, uint8_t, uint8_t*)> newCmd;
	
	static void reset();
	
private:
	static DisplayDriver *m_instance;
	
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

