#include "GpioDriver.h"
#include <stm32f4xx.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_syscfg.h>
#include <stm32f4xx_exti.h>
#include <misc.h>
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"

struct GpioPin
{
	int indexPin;
	GPIO_TypeDef *port;
	const uint16_t pin;
	const uint32_t clock;
	GpioDriver::StatesPin state;
};

const GpioPin settingsPins[] = { 
		{GpioDriver::PinFanLowSpeed, GPIOA, GPIO_Pin_0, RCC_AHB1Periph_GPIOA, GpioDriver::StatePinOne},//23
		{GpioDriver::PinH2O, GPIOA, GPIO_Pin_1, RCC_AHB1Periph_GPIOA, GpioDriver::StatePinOne},//24
		{GpioDriver::PinTemperatureUp, GPIOA, GPIO_Pin_2, RCC_AHB1Periph_GPIOA, GpioDriver::StatePinOne},//25
		{GpioDriver::PinTemperatureDown, GPIOA, GPIO_Pin_3, RCC_AHB1Periph_GPIOA, GpioDriver::StatePinOne},//26
		{GpioDriver::PinShiberX, GPIOA, GPIO_Pin_4, RCC_AHB1Periph_GPIOA, GpioDriver::StatePinOne},
		{GpioDriver::PinShiberO, GPIOA, GPIO_Pin_5, RCC_AHB1Periph_GPIOA, GpioDriver::StatePinOne},
		
		{GpioDriver::PinGreen, GPIOD, GPIO_Pin_12, RCC_AHB1Periph_GPIOD, GpioDriver::StatePinOne },//59
		{GpioDriver::PinYellow,	GPIOD, GPIO_Pin_13, RCC_AHB1Periph_GPIOD,GpioDriver::StatePinOne },//60
		
		{GpioDriver::GlobalEnable, GPIOD, GPIO_Pin_14, RCC_AHB1Periph_GPIOD, GpioDriver::StatePinOne },//61
		{GpioDriver::HoodVisor, GPIOD, GPIO_Pin_15, RCC_AHB1Periph_GPIOD, GpioDriver::StatePinOne },//62
		{GpioDriver::MainHood, GPIOE, GPIO_Pin_4, RCC_AHB1Periph_GPIOE, GpioDriver::StatePinOne }, //3
		{GpioDriver::EnableLightDoorLight, GPIOE, GPIO_Pin_5, RCC_AHB1Periph_GPIOE, GpioDriver::StatePinOne }, //4
		{GpioDriver::PinFanFastSpeed, GPIOE, GPIO_Pin_6, RCC_AHB1Periph_GPIOE, GpioDriver::StatePinOne }, //5
		{GpioDriver::PinLoadBread, GPIOE, GPIO_Pin_2, RCC_AHB1Periph_GPIOE, GpioDriver::StatePinOne }, //1
		{GpioDriver::PinDownloadBread, GPIOE, GPIO_Pin_3, RCC_AHB1Periph_GPIOE, GpioDriver::StatePinOne }, //2
};
const int sizeSettingsPins = sizeof(settingsPins) / sizeof(settingsPins[0]);


GpioDriver *GpioDriver::ins;
GpioDriver::GpioDriver()
{
	ins = this;
}

void GpioDriver::setPin(PinsGpioOut pin, StatesPin state) {
	if (state == StatePinOne)
		GPIO_SetBits(settingsPins[pin].port, settingsPins[pin].pin);
	else
		GPIO_ResetBits(settingsPins[pin].port, settingsPins[pin].pin);
	if (pin == GpioDriver::PinH2O) {
		if (state == StatePinOne) {
			enableInt();
		}
		else { 
			disableInt();
		}
	}
		
	
}
void GpioDriver::togglePin(PinsGpioOut pin) {
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
		ini.GPIO_PuPd = GPIO_PuPd_DOWN;
		if (settingsPins[i].indexPin == GpioDriver::PinGreen || settingsPins[i].indexPin == GpioDriver::PinYellow) {
			ini.GPIO_Mode = GPIO_Mode_AF;
			ini.GPIO_Speed = GPIO_Speed_2MHz;
		}
		GPIO_Init(settingsPins[i].port, &ini);
	}
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_TIM4);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF_TIM4);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	TIM_TimeBaseInitTypeDef timer;

	TIM_TimeBaseStructInit(&timer);
	timer.TIM_ClockDivision = TIM_CKD_DIV1;
	timer.TIM_CounterMode = TIM_CounterMode_Up;
	timer.TIM_Prescaler = 839;
	timer.TIM_Period = 9;
	TIM_TimeBaseInit(TIM4, &timer);
	
	TIM_OCInitTypeDef timerPWM;
	TIM_OCStructInit(&timerPWM);
	timerPWM.TIM_Pulse = 2;
	timerPWM.TIM_OCMode = TIM_OCMode_PWM1;
	timerPWM.TIM_OutputState = TIM_OutputState_Enable;
	timerPWM.TIM_OCPolarity = TIM_OCPolarity_High; 
	TIM_OC1Init(TIM4, &timerPWM);
	timerPWM.TIM_Pulse = 2;
	TIM_OC2Init(TIM4, &timerPWM);
	
	TIM_SetCounter(TIM4, 0);
	TIM_Cmd(TIM4, ENABLE);

	
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
	//датчик воды
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource13);
	exti.EXTI_Line = EXTI_Line13;
	/* Enable interrupt */
	exti.EXTI_LineCmd = ENABLE;
	/* Interrupt mode */
	exti.EXTI_Mode = EXTI_Mode_Interrupt;
	/* Triggers on rising and falling edge */
    exti.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
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
	
	
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);	
    ini.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_12 | GPIO_Pin_14 | GPIO_Pin_15;
    ini.GPIO_OType = GPIO_OType_PP;
    ini.GPIO_Mode = GPIO_Mode_IN;
    ini.GPIO_Speed = GPIO_Speed_2MHz;
    ini.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &ini);

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);	
    ini.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_4;
    ini.GPIO_OType = GPIO_OType_PP;
    ini.GPIO_Mode = GPIO_Mode_IN;
    ini.GPIO_Speed = GPIO_Speed_2MHz;
    ini.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOD, &ini);
  
    //кнопка стоп
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOD, EXTI_PinSource1);
    exti.EXTI_Line = EXTI_Line1;
    /* Enable interrupt */
    exti.EXTI_LineCmd = ENABLE;
    /* Interrupt mode */
    exti.EXTI_Mode = EXTI_Mode_Interrupt;
    /* Triggers on rising and falling edge */
    exti.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_Init(&exti);
	
    /* Add IRQ vector to NVIC */
    /* PA3 is connected to EXTI_Line3, which has EXTI3_IRQn vector */
    nvic.NVIC_IRQChannel = EXTI1_IRQn;
    /* Set priority */
    nvic.NVIC_IRQChannelPreemptionPriority = 0x09;
    /* Set sub priority */
    nvic.NVIC_IRQChannelSubPriority = 0x0c;
    /* Enable interrupt */
    nvic.NVIC_IRQChannelCmd = ENABLE;
    /* Add to NVIC */
    NVIC_Init(&nvic);

    //датчик-концевик шибера
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOD, EXTI_PinSource0);
    exti.EXTI_Line = EXTI_Line0;
    /* Enable interrupt */
    exti.EXTI_LineCmd = ENABLE;
    /* Interrupt mode */
    exti.EXTI_Mode = EXTI_Mode_Interrupt;
    /* Triggers on rising and falling edge */
    exti.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_Init(&exti);
	
    /* Add IRQ vector to NVIC */
    /* PA3 is connected to EXTI_Line3, which has EXTI3_IRQn vector */
    nvic.NVIC_IRQChannel = EXTI0_IRQn;
    /* Set priority */
    nvic.NVIC_IRQChannelPreemptionPriority = 0x09;
    /* Set sub priority */
    nvic.NVIC_IRQChannelSubPriority = 0x0d;
    /* Enable interrupt */
    nvic.NVIC_IRQChannelCmd = ENABLE;
    /* Add to NVIC */
    NVIC_Init(&nvic);

    //датчик-концевик шибера
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource15);
    exti.EXTI_Line = EXTI_Line15;
    /* Enable interrupt */
    exti.EXTI_LineCmd = ENABLE;
    /* Interrupt mode */
    exti.EXTI_Mode = EXTI_Mode_Interrupt;
    /* Triggers on rising and falling edge */
    exti.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
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

void GpioDriver::enableInt()
{
	EXTI_InitTypeDef exti;
	exti.EXTI_Line = EXTI_Line13;
	/* Enable interrupt */
	exti.EXTI_LineCmd = ENABLE;
	/* Interrupt mode */
	exti.EXTI_Mode = EXTI_Mode_Interrupt;
	/* Triggers on rising and falling edge */
	exti.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_Init(&exti);
}

void GpioDriver::disableInt() {
	EXTI_InitTypeDef exti;
	exti.EXTI_Line = EXTI_Line13;
	/* Enable interrupt */
	exti.EXTI_LineCmd = DISABLE;
	/* Interrupt mode */
	exti.EXTI_Mode = EXTI_Mode_Interrupt;
	/* Triggers on rising and falling edge */
	exti.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_Init(&exti);
}

void GpioDriver::enableYellowLed()
{
	TIM4->CCER |= TIM_CCER_CC1E;
}
void GpioDriver::disableYellowLed()
{
	TIM4->CCER &= (~TIM_CCER_CC1E);
	
}

void GpioDriver::enableGreenLed() {
	TIM4->CCER |= TIM_CCER_CC2E;
}
void GpioDriver::disableGreenLed() {
	TIM4->CCER &= (~TIM_CCER_CC2E);
}

GpioDriver::~GpioDriver()
{
}

bool GpioDriver::isEventLoadBread()
{
    static uint8_t prevState = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7);
    uint8_t state = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7);
    if (state != prevState) {
        prevState = state;
        if (state == Bit_SET) {
            return true;
        }
    }
    return false;
}
bool GpioDriver::isEventDownloadBread()
{static uint8_t prevState = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3);
    uint8_t state = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3);
    if (state != prevState) {
        prevState = state;
        if (state == Bit_SET) {
            return true;
        }
    }
    return false;
}
bool GpioDriver::isEventSensorTempDrive1(){static uint8_t prevState = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12);
    uint8_t state = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12);
    if (state != prevState) {
        prevState = state;
        if (state == Bit_SET) {
            return true;
        }
    }
    return false;
}
bool GpioDriver::isEventSensorTempDrive2(){static uint8_t prevState = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13);
    uint8_t state = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13);
    if (state != prevState) {
        prevState = state;
        if (state == Bit_SET) {
            return true;
        }
    }
    return false;
}
bool GpioDriver::isEventSensorTempDrive3(){static uint8_t prevState = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14);
    uint8_t state = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14);
    if (state != prevState) {
        prevState = state;
        if (state == Bit_SET) {
            return true;
        }
    }
    return false;
}
bool GpioDriver::isDoorClosed(){static uint8_t prevState = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_3);
    uint8_t state = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_3);
    if (state != prevState) {
        prevState = state;
        if (state == Bit_SET) {
            return true;
        }
    }
    return false;
}
bool GpioDriver::isStartKey(){static uint8_t prevState = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_4);
    uint8_t state = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_4);
    if (state != prevState) {
        prevState = state;
        if (state == Bit_SET) {
            return true;
        }
    }
    return false;
}

uint16_t cntInt = 0;

extern "C" 
void EXTI1_IRQHandler() {
    EXTI_ClearITPendingBit(EXTI_Line1);
}
extern "C" 
void EXTI0_IRQHandler() {
    EXTI_ClearITPendingBit(EXTI_Line0);
}

extern "C" 
void EXTI15_10_IRQHandler() {
	cntInt++;
	uint16_t delay = 10000;
	while (delay--); 
    if (EXTI_GetITStatus(EXTI_Line13)) {
        GpioDriver::instace()->pinEvent(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13));
        EXTI_ClearITPendingBit(EXTI_Line13);
    }
    if (EXTI_GetITStatus(EXTI_Line15)) {
        //GpioDriver::instace()->pinEvent(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_15));
        EXTI_ClearITPendingBit(EXTI_Line15);
    }

}

