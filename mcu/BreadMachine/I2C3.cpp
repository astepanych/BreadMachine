#include "I2C3.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
I2C3Interface &I2C3Interface::instance()
{
	static I2C3Interface obj;
	return obj;
}

I2C3Interface::I2C3Interface()
{
	
}

void I2C3Interface::init()
{
	// enable APB1 peripheral clock for I2C1
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C3, ENABLE);
	// enable clock for SCL and SDA pins
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	
	GPIO_InitTypeDef pins;
	GPIO_StructInit(&pins);
	pins.GPIO_Mode = GPIO_Mode_AF;
	pins.GPIO_Pin = GPIO_Pin_9;
	pins.GPIO_Speed = GPIO_Speed_50MHz;
	pins.GPIO_OType = GPIO_OType_OD;
	pins.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &pins);
	pins.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOA, &pins);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_I2C3);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, GPIO_AF_I2C3);
	
	I2C_InitTypeDef I2C_InitStruct;
	
	// configure I2C1 
	
	I2C_InitStruct.I2C_ClockSpeed = 100000; // 100kHz
	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C; // I2C mode
	I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2; // 50% duty cycle --> standard
	I2C_InitStruct.I2C_OwnAddress1 = 0x00; // own address, not relevant in master mode
	I2C_InitStruct.I2C_Ack = I2C_Ack_Disable; // disable acknowledge when reading (can be changed later on)
	I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; // set address length to 7 bit addresses
	I2C_Init(I2C3, &I2C_InitStruct); // init I2C1
	I2C_Cmd(I2C3, ENABLE);
	// enable I2C1

}

void I2C3Interface::write(uint16_t addr, uint8_t* data, uint8_t len)
{
	while (I2C_GetFlagStatus(I2C3, I2C_FLAG_BUSY)) ;
	uint8_t *a = (uint8_t*)&addr;
	/* Send I2C1 START condition */

	I2C_GenerateSTART(I2C3, ENABLE);
	while (!I2C_CheckEvent(I2C3, I2C_EVENT_MASTER_MODE_SELECT)) {};
	/* Test on I2C1 EV5 and clear it */

	//  while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

	 /* Send EEPROM slave Address for write */
	uint8_t addrByte = (0x50 + ((addr >> 8) & 0x07)) << 1;
	I2C_Send7bitAddress(I2C3, addrByte, I2C_Direction_Transmitter);
	while (!I2C_CheckEvent(I2C3, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {}; 

	/* Test on I2C1 EV6 and clear it */

	//  while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	 /* Send I2C1 EEPROM internal address */
	for (int i = 0; i >= 0; i--)
	{
		I2C_SendData(I2C3, a[i]); // 0x02 is config register
		while(!I2C_CheckEvent(I2C3, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	}
	for (int i = 0; i < len; i++) {
		I2C_SendData(I2C3, data[i]); // 0x02 is config register	
		while (!I2C_CheckEvent(I2C3, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) ;
	}
	I2C_GenerateSTOP(I2C3, ENABLE);
	uint16_t del = 10000;
	while(del--);
}
void I2C3Interface::read(uint16_t addr, uint8_t* data, uint8_t len)
{
	int16_t del = 100;
	uint8_t *a = (uint8_t*)&addr;
	while (I2C_GetFlagStatus(I2C3, I2C_FLAG_BUSY)) ;
	
	I2C_GenerateSTART(I2C3, ENABLE);
	while (!I2C_CheckEvent(I2C3, I2C_EVENT_MASTER_MODE_SELECT));
	uint8_t addrByte = (0x50+((addr>>8)&0x07))<<1;
	I2C_Send7bitAddress(I2C3, addrByte, I2C_Direction_Transmitter);
	while (!I2C_CheckEvent(I2C3, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	
	for (int i = 0; i >= 0; i--) {
		I2C_SendData(I2C3, a[i]); // 0x02 is config register
		while (!I2C_CheckEvent(I2C3, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) ;
	}
	/*I2C_GenerateSTOP(I2C3, ENABLE);
	while (I2C_GetFlagStatus(I2C3, I2C_FLAG_BUSY)) ;*/
	I2C_GenerateSTART(I2C3, ENABLE);
	while (!I2C_CheckEvent(I2C3, I2C_EVENT_MASTER_MODE_SELECT)) ;
	I2C_Send7bitAddress(I2C3, addrByte, I2C_Direction_Receiver);
	while (!I2C_CheckEvent(I2C3, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
	
	for (int i = 0; i < len-1; i++) {
		I2C_AcknowledgeConfig(I2C3, ENABLE);
		while (!I2C_CheckEvent(I2C3, I2C_EVENT_MASTER_BYTE_RECEIVED)) ;
		data[i] = I2C_ReceiveData(I2C3); // 0x02 is config register	

	}
	I2C_AcknowledgeConfig(I2C3, DISABLE);
	I2C_GenerateSTOP(I2C3, ENABLE);
	while (!I2C_CheckEvent(I2C3, I2C_EVENT_MASTER_BYTE_RECEIVED)) ;
	data[len-1] = I2C_ReceiveData(I2C3); // 0x02 is config register	
	 del = 10000;
	while (del--) ;
	
}	
