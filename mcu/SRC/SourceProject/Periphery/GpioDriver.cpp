#include "GpioDriver.h"
#include <stm32f4xx.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_syscfg.h>
#include <stm32f4xx_exti.h>
#include <misc.h>

struct GpioPin
{
	int indexPin;
	GPIO_TypeDef *port;
	const uint16_t pin;
	const uint32_t clock;
	GpioDriver::StatesPin state;
};

const GpioPin settingsPins[] = { 
		{GpioDriver::PinFan, GPIOA, GPIO_Pin_0, RCC_AHB1Periph_GPIOA, GpioDriver::StatePinOne},
		{GpioDriver::PinH2O, GPIOA, GPIO_Pin_1, RCC_AHB1Periph_GPIOA, GpioDriver::StatePinOne},
		{GpioDriver::PinTemperatureUp, GPIOA, GPIO_Pin_2, RCC_AHB1Periph_GPIOA, GpioDriver::StatePinOne},
		{GpioDriver::PinTemperatureDown, GPIOA, GPIO_Pin_3, RCC_AHB1Periph_GPIOA, GpioDriver::StatePinOne},
		{GpioDriver::PinShiberX, GPIOA, GPIO_Pin_4, RCC_AHB1Periph_GPIOA, GpioDriver::StatePinOne},
		{GpioDriver::PinShiberO, GPIOA, GPIO_Pin_5, RCC_AHB1Periph_GPIOA, GpioDriver::StatePinOne},
		{GpioDriver::PinGreen, GPIOD, GPIO_Pin_12, RCC_AHB1Periph_GPIOD, GpioDriver::StatePinOne },
		{GpioDriver::PinYellow,				GPIOD, GPIO_Pin_13, RCC_AHB1Periph_GPIOD,GpioDriver::StatePinOne },
		{GpioDriver::PinX1, GPIOD, GPIO_Pin_14, RCC_AHB1Periph_GPIOD, GpioDriver::StatePinOne },
		{GpioDriver::PinX2, GPIOD, GPIO_Pin_15, RCC_AHB1Periph_GPIOD, GpioDriver::StatePinOne },
		{GpioDriver::Led, GPIOF, GPIO_Pin_9, RCC_AHB1Periph_GPIOF, GpioDriver::StatePinOne },
		{GpioDriver::Led1, GPIOF, GPIO_Pin_10, RCC_AHB1Periph_GPIOF, GpioDriver::StatePinOne }
};
const int sizeSettingsPins = sizeof(settingsPins) / sizeof(settingsPins[0]);


GpioDriver *GpioDriver::ins;
GpioDriver::GpioDriver()
{
	ins = this;
}

void GpioDriver::setPin(PinsGpioOut pin, StatesPin state)
{
	if (state == StatePinOne)
		GPIO_SetBits(settingsPins[pin].port, settingsPins[pin].pin);
	else
		GPIO_ResetBits(settingsPins[pin].port, settingsPins[pin].pin);
}
void GpioDriver::togglePin(PinsGpioOut pin)
{
	GPIO_ToggleBits(settingsPins[pin].port, settingsPins[pin].pin);
}

void GpioDriver::initModule()
{
	GPIO_InitTypeDef ini;
	for (int i = 0; i < sizeSettingsPins; i++)
	{
		RCC_AHB1PeriphClockCmd(settingsPins[i].clock, ENABLE);	
		ini.GPIO_Pin = settingsPins[i].pin;
		ini.GPIO_Mode = GPIO_Mode_OUT;
		ini.GPIO_OType = GPIO_OType_PP;
		ini.GPIO_Speed = GPIO_Speed_50MHz;
		ini.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(settingsPins[i].port, &ini);
	}
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC , ENABLE);	
	ini.GPIO_Pin = GPIO_Pin_13;
	ini.GPIO_OType = GPIO_OType_PP;
	ini.GPIO_Mode = GPIO_Mode_IN;
	ini.GPIO_Speed = GPIO_Speed_2MHz;
	ini.GPIO_PuPd = GPIO_PuPd_UP;
	
	GPIO_Init(GPIOC, &ini);
	
	EXTI_InitTypeDef exti;
	NVIC_InitTypeDef nvic;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource13);
	
	exti.EXTI_Line = EXTI_Line13;

	/* Enable interrupt */

	exti.EXTI_LineCmd = ENABLE;

	/* Interrupt mode */

	exti.EXTI_Mode = EXTI_Mode_Interrupt;

	/* Triggers on rising and falling edge */

	exti.EXTI_Trigger = EXTI_Trigger_Rising_Falling;

	/* Add to EXTI */

	EXTI_Init(&exti);

	/* Add IRQ vector to NVIC */

	/* PA3 is connected to EXTI_Line3, which has EXTI3_IRQn vector */

	nvic.NVIC_IRQChannel = EXTI15_10_IRQn;

	/* Set priority */

	nvic.NVIC_IRQChannelPreemptionPriority = 0x09;

	/* Set sub priority */

	nvic.NVIC_IRQChannelSubPriority = 0x09;

	/* Enable interrupt */

	nvic.NVIC_IRQChannelCmd = ENABLE;

	/* Add to NVIC */

	NVIC_Init(&nvic);
	
}

GpioDriver::~GpioDriver()
{
}


uint16_t cntInt = 0;
extern "C" void EXTI15_10_IRQHandler()
{
	cntInt++;
	uint16_t delay = 10000;
	while (delay--) ;       
	
	GpioDriver::instace()->pinEvent(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13));
	EXTI_ClearITPendingBit(EXTI_Line13);
}

