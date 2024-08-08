#include "DisplayDriver.h"
#include <string.h>

Uart3 *DisplayDriver::uart3;

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

std::function<void(const uint16_t, uint8_t*)> DisplayDriver::newCmd;

DisplayDriver::DisplayDriver() {
	m_instance = this;
	xSemDisplay = xSemaphoreCreateBinary();
	xQueueDisplay = xQueueCreate(SizeQueUart, sizeof(ElementUart));
	
	xTaskCreate(this->taskDisplay,
		"display",
		512,
		(void*)2,
		tskIDLE_PRIORITY+1,
		&xHandleDisplay); 
	
	uart3 = new Uart3();
	uart3->init();
	uart3->putByte = this->putByte;
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
		newCmd(id, data + 3);
		break;
	}
}

void DisplayDriver::sendToDisplay(uint16_t id, std::wstring &str)
{
	uint8_t buf[SizeBuffer + 3];
	memset(buf, 0, SizeBuffer + 3);
	buf[0] = startByte1;
	buf[1] = startByte2;
	buf[2] = str.length()*2 + 3;
	buf[3] = cmdByteWrite;
	//заполняем адрес
	uint8_t *p = (uint8_t*)&id;
	for (int i = sizeof(uint16_t) - 1; i >= 0; i--) {
		buf[4 + i] = *p;
		p++;
	}
	p = (uint8_t*)str.data();
	int len = 6;
	for (int i = 0; i < str.length(); i++)
	{
		buf[6 + 2*i] = *(p + 1);
		buf[6 + 2*i +1] = *(p);
		p += 4;
		len += 2;
	}
	uart3->write(buf, len);
}

void DisplayDriver::reset()
{
	uint8_t buf[] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x04, 0x55, 0xAA, 0x5A, 0xA5};
	uart3->write(buf, sizeof(buf));
}

void DisplayDriver::sendToDisplay(uint16_t id, uint8_t len, uint8_t *data)
{
	uint8_t buf[SizeBuffer + 3];
	buf[0] = startByte1;
	buf[1] = startByte2;
	buf[2] = len+3;
	buf[3] = cmdByteWrite;
	//заполняем адрес
	uint8_t *p = (uint8_t*)&id;
	for (int i = sizeof(uint16_t) - 1; i >= 0; i--)	{
		buf[4 + i] = *p;
		p++;
	}
	memcpy(buf + 6, data, len);
	uart3->write(buf, len + 6);
	
}

void DisplayDriver::sendToDisplay(uint16_t id, uint16_t data)
{
	uint8_t buf[SizeBuffer + 3];
	buf[0] = startByte1;
	buf[1] = startByte2;
	buf[2] = sizeof(uint16_t) + 3;
	buf[3] = cmdByteWrite;
	
	uint8_t *p = (uint8_t*)&id;
	for (int i = sizeof(uint16_t) - 1; i >= 0; i--) {
		buf[4 + i] = *p;
		p++;
	}
	p = (uint8_t*)&data;
	for (int i = sizeof(uint16_t) - 1; i >= 0; i--) {
		buf[6 + i] = *p;
		p++;
	}
	uart3->write(buf, 8);
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