#include <AppCore.h>
#include <stdio.h>
#include <misc.h>
#include <string.h>
#include <string>
#include <stm32f4xx_flash.h>

#include "typedef.h"
#include <time.h>
#include <math.h>

eFailSensorTemperature AppCore::checkTemperatureSensors()
{
    eFailSensorTemperature curState = NoFailSensorTemperature;

	if (adc->value2() > thresholdErrorTemperature)
	{
		curState = FailSensorTemperature2;
		LOG::instance().log("err temp sen2"); 
	}
	else
	{
    	curState = NoFailSensorTemperature;		
	}
	if (adc->value1() > thresholdErrorTemperature) {
    	curState = static_cast<eFailSensorTemperature>(curState | FailSensorTemperature1) ;
		LOG::instance().log("err temp sen1"); 
	}
	else
	{	
    	curState = static_cast<eFailSensorTemperature>(curState&(~FailSensorTemperature1));
	}
	
	if (stateTemperatureSensor != curState)
	{
    	if (curState != NoFailSensorTemperature)
			display->showMessage(PageMessage, curState);
		else
			display->hideMessage();
		stateTemperatureSensor = curState;
	}
	return stateTemperatureSensor;
	
}

