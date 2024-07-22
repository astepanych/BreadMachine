#pragma once
#include "stm32f4xx_gpio.h"

class GpioDriver
{
	
public:
	enum PinsGpioOut
	{
		PinVcnt = 0,
		PinH2O,
		PinTemperatureUp,
		PinTemperatureDown,
		PinShiberX,
		PinShiberO,
		PinGreen,
		PinYellow,
		PinX1,
		PinX2, 
		Led
		
	};
	enum StatesPin
	{
		StatePinZero =  Bit_RESET,
		StatePinOne
	}; 
	GpioDriver();
	~GpioDriver();
	void initModule();
	void setPin(PinsGpioOut pin, StatesPin state);
	void togglePin(PinsGpioOut pin);

	static GpioDriver *instace() {return ins;};
	
	

private:
	static GpioDriver *ins;
};


