#include "DisplayDriver.h"
#include <string.h>



#define uart uart2


//display
static const uint8_t cmdTouch[] = { 0x5a, 0xa5, 0x4, 0x83, 0x00, 0x16, 0x03 };
xQueueHandle DisplayDriver::xQueueDisplay;

DisplayDriver *DisplayDriver::m_instance;
DisplayDriver::StateParcePacketDisplay DisplayDriver::statePacket;
xSemaphoreHandle DisplayDriver::xSemDisplay;

const uint8_t DisplayDriver::startByte1;
const uint8_t DisplayDriver::startByte2;

const uint8_t DisplayDriver::cmdByteWrite;
const uint8_t DisplayDriver::cmdByteRead;

uint8_t DisplayDriver::bufParce[SizeBuffer];
uint8_t DisplayDriver::lenPacket;
uint8_t DisplayDriver::currentIndex;

int DisplayDriver::cntPutByte;

std::function<void(const uint16_t, uint8_t, uint8_t*)> DisplayDriver::newCmd;

DisplayDriver::DisplayDriver() {
	m_instance = this;
	xSemDisplay = xSemaphoreCreateBinary();
	xQueueDisplay = xQueueCreate(SizeQueUart, sizeof(ElementUart));
	
	xTaskCreate(this->taskDisplay,
		"display",
		1024,
		(void*)2,
		tskIDLE_PRIORITY+1,
		&xHandleDisplay); 
	
	
	uart.init();
	uart.putByte = this->putByte;
	statePacket = DisplayDriver::StateWaitByte1;
}

void DisplayDriver::putByte(const uint8_t byte)
{
	switch (statePacket)
	{ 
	case DisplayDriver::StateWaitByte1:
		if (byte == startByte1) {
			statePacket = DisplayDriver::StateWaitByte2;
			cntPutByte = 1;
		}
		break;
	case DisplayDriver::StateWaitByte2:
		if (byte == startByte2 && cntPutByte == 1) {
			statePacket = DisplayDriver::StateWaitLen;
			cntPutByte++;
		}
		else {
			statePacket = DisplayDriver::StateWaitByte1;	
		}
		break;
	case DisplayDriver::StateWaitLen:
		cntPutByte++;
		lenPacket = byte;
		currentIndex = 0;
		statePacket = DisplayDriver::StateReadByte;
		break;
	case DisplayDriver::StateReadByte:
		bufParce[currentIndex] = byte;
		currentIndex++;
		if (currentIndex == lenPacket) {
			ElementUart el;
			memcpy(el.buf, bufParce, lenPacket); 
			el.len = lenPacket;
			xQueueSendToBackFromISR(xQueueDisplay, &el, 0);
			statePacket = DisplayDriver::StateWaitByte1;
			portYIELD_FROM_ISR(pdTRUE);
		}
		break;
	default:
		break;
	}
}

void DisplayDriver::initDisplay()
{

}

void DisplayDriver::parsePackFromDisplay(uint8_t len, uint8_t *data)
{
	uint8_t cmd = data[0]; //команда 
	uint16_t id; //идентификатор, он же адрес
	id = (data[1] << 8) + data[2];
	
	uint8_t realLen = len - 3; //реальная длина данных
	if (cmd == cmdByteWrite && id == CmdCofirm)
	{
		return;
	}
	
	switch (id)
	{
	default:
		newCmd(id, len,data + 3);
		break;
	}
}

void DisplayDriver::sendToDisplay(const uint16_t id, const std::string &str)
{
	static uint8_t buf[SizeBuffer + 3];
	memset(buf, 0, SizeBuffer + 3);
	buf[0] = startByte1;
	buf[1] = startByte2;
	
	buf[3] = cmdByteWrite;
	//заполняем адрес
	u16be *p1 = (u16be*)&buf[4]; 
	*p1 = id;
	uint8_t *p = (uint8_t*)str.data();
	int len = 6;
	char *pOut = (char*)(buf + len);
	static char buf1[SizeBuffer + 3];
	memset(buf1, 0, SizeBuffer + 3);
	memcpy(buf1, str.data(), str.length());
	int l = convertUtf8ToCp1251(buf1, pOut);
	if (l != -1)
	{
		buf[2] = l + 3;
	}
	else
	{
		memcpy(pOut, str.data(), str.length());
		l = str.length();
		buf[2] = l + 3;
	}

	len += l;
	
	uart.write(buf, len);
}

void DisplayDriver::reset()
{
	uint8_t buf[] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x04, 0x55, 0xAA, 0x5A, 0xA5};
	uart.write(buf, sizeof(buf));
}

void DisplayDriver::sendToDisplay(uint16_t id, uint8_t len, uint8_t *data)
{
	static uint8_t buf[SizeBuffer + 3];
	buf[0] = startByte1;
	buf[1] = startByte2;
	buf[2] = len+3;
	buf[3] = cmdByteWrite;
	//заполняем адрес
	u16be *p = (u16be*)&buf[4]; 
	*p = id;
	memcpy(buf + 6, data, len);
	uart.write(buf, len + 6);
	
}

void DisplayDriver::switchPage(uint16_t page)
{
	static uint8_t buf[10];
	buf[0] = startByte1;
	buf[1] = startByte2;
	buf[2] = 7;
	buf[3] = cmdByteWrite;
	buf[4] = 0;
	buf[5] = 0x84;
	buf[6] = 0x5a;
	buf[7] = 0x01;
	u16be *p = (u16be*)&buf[8]; 
	*p = page;

	uart.write(buf, 10);
	
}

void DisplayDriver::sendToDisplayStr(uint16_t id, uint8_t len, char *data)
{
	static uint8_t buf[SizeBuffer + 3];
	memset(buf, 0xff, SizeBuffer + 3);
	buf[0] = startByte1;
	buf[1] = startByte2;
	buf[2] = len + 5;
	buf[3] = cmdByteWrite;
	//заполняем адрес
	u16be *p = (u16be*)&buf[4]; 
	*p = id;
	if(data!=nullptr)
		memcpy(buf + 6, data, len);
	uart.write(buf, len + 8);
	
}




void DisplayDriver::sendToDisplay(uint16_t id, uint16_t data)
{
	static uint8_t buf[SizeBuffer + 3];
	buf[0] = startByte1;
	buf[1] = startByte2;
	buf[2] = sizeof(uint16_t) + 3;
	buf[3] = cmdByteWrite;
	
	u16be *p = (u16be*)&buf[4]; 
	*p = id;
	p++;
	*p = data;
	
	
	uart.write(buf, 8);
}

void DisplayDriver::sendToDisplayF(uint16_t id, float &data)
{
	static uint8_t buf[SizeBuffer + 3];
	buf[0] = startByte1;
	buf[1] = startByte2;
	buf[2] = sizeof(float) + 3;
	buf[3] = cmdByteWrite;
	
	u16be *p = (u16be*)&buf[4]; 
	*p = id;
	u32be *p32 = (u32be*)&buf[6];
	uint32_t tmp;
	memcpy(&tmp, &data, sizeof(float));
	*p32 = (u32be)tmp;
	
	uart.write(buf, 10);
}

void DisplayDriver::taskDisplay(void *p)
{
	ElementUart pop;
	BaseType_t res;

	int len; 
	while (true)
	{
		res = xQueueReceive(xQueueDisplay, &pop, portMAX_DELAY); // Ожидаем элемента в очереди
		//если дождались - значит это команда
		if (res == pdPASS)
		{
			parsePackFromDisplay(pop.len ,pop.buf);
		}

	}
}