#include <AppCore.h>
#include <stdio.h>
#include <misc.h>
#include <string.h>
#include <string>
#include <stm32f4xx_flash.h>

#include "typedef.h"
#include <time.h>
#include <math.h>

uint16_t AppCore::checkTemperatureSensors()
{
	uint16_t curState = 0;

	if (adc->value2() > thresholdErrorTemperature)
	{
		curState = 2;
		LOG::instance().log("err temp sen2"); 
	}
	else
	{
		curState = 0;		
	}
	if (adc->value1() > thresholdErrorTemperature) {
		curState |= 1;
		LOG::instance().log("err temp sen1"); 
	}
	else
	{	
		curState &= (~1);
	}
	
	if (stateTemperatureSensor != curState)
	{
		if (curState != 0)
			display->showMessage(PageMessage, curState);
		else
			display->hideMessage();
		stateTemperatureSensor = curState;
	}
	return stateTemperatureSensor;
	
}

