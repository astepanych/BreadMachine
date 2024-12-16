#pragma once
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "functional"

class GpioDriver
{
	
public:
	enum PinsGpioOut
	{
		PinFan = 0,
		PinH2O,
		PinTemperatureUp,
		PinTemperatureDown,
		PinShiberX,
		PinShiberO,
		PinGreen,
		PinYellow,
		PinX1,
		PinX2, 
		Led, 
		Led1, 
		GlobalEnable
		
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
	void enableYellowLed();
	void disableYellowLed();
	void enableGreenLed();
	void disableGreenLed();
	
	void disableInt() ;
		void enableInt() ;

	static GpioDriver *instace() {return ins;};
	std::function<void(bool)> pinEvent; 
	

private:
	static GpioDriver *ins;
};


