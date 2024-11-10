#include <AppCore.h>
#include <stdio.h>
#include <misc.h>
#include <string.h>
#include <string>
#include <stm32f4xx_flash.h>

#include "typedef.h"
#include <time.h>
#include <math.h>

void AppCore::checkTemperatureSensors()
{

	const float thresholdErrorTemperature = 4085;
	if (adc->value2() > thresholdErrorTemperature)
	{
		if (!isErrorSensor2) {
			isErrorSensor2 = true;
			//TODO output error
		}
		
	}
	else
	{
		if (isErrorSensor2) {
			isErrorSensor2 = false;
			//TODO output no error
		}
		
	}
	if (adc->value1() > thresholdErrorTemperature) {
		if (!isErrorSensor1)
		{
			isErrorSensor1 = true;
			//TODO output error
		}
	}
	else
	{	if (isErrorSensor1) {
			isErrorSensor1 = false;
		//TODO output no error
		}
		
	}
}

void AppCore::checkWater()
{
	
}
