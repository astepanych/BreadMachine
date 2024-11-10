#pragma once
#include  <Periphery/GpioDriver.h>
#include <FreeRTOS.h>
#include <task.h>
#include <portmacro.h>
#include <AdcDriver.h>
#include <DisplayDriver.h>
#include <vector>
#include <WorkModeEdit.h>
#include "typedef.h"
#include "timers.h"
#include "Uart5.h"
#include <../../../common/dataexchenge.h>
#include <globals.h>

#define NumItemList  (7)
#define NumItemListEdit  (5)
#define DeltaTemperature  (0.5)


constexpr uint16_t xProgresStage = 17;
constexpr uint16_t yProgresStage = 175;
constexpr uint16_t hProgresStage = 60;
constexpr uint16_t wProgresStage = 568;

constexpr uint16_t password = 2024;


enum StateRun
{
	StateRunIdle,       
	StateRunStart,
	StateRunWork,
	StateRunStop
};

class AppCore
{
public:
	AppCore();
	~AppCore();
	static AppCore &instance();

	void taskPeriodic(void *p = 0);
	void taskExchange(void *p = 0);

	void init();

	void eventTimeoutLeds(TimerHandle_t timer);

	
	void parsePackDisplay(const uint16_t id, uint8_t len, uint8_t* data);
	
	void keyEvent(uint16_t key);
	void initHal();
	void initOsal();
	
	void initText();
	
	 void initDefaultPrograms();
	 void readPrograms();
	 void writeGlobalParams();
	
	void fillProgram(const std::string &name, const uint16_t numStages, const uint16_t *stage = nullptr);
	void updateProgressBar(uint16_t value);
	
	void paintStageProgress();
	WorkMode currentWorkMode;
	
	void getSizeWRectangle(const WorkMode &mode, uint16_t *wList);
	void updateParamStage();
	void updateTime(uint16_t sec);
	void toggleYellow()
	{GpioDriver::instace()->togglePin(GpioDriver::GpioDriver::PinYellow); };
	void toggleGreen()
	{GpioDriver::instace()->togglePin(GpioDriver::GpioDriver::PinGreen); };
	static unsigned int CRC32_function(unsigned char *buf, unsigned long len);
	void checkTemperatureSensors();
	void checkWater();
	
private:
	void procUartData(const PackageNetworkFormat&p);
	void initExchange();
	TimerHandle_t timerYellow;
	TimerHandle_t timerGreen;
	uint16_t countGreenLeds{0};
	uint16_t countYellowLeds{0};
	uint16_t stateRun;
	uint32_t commonDuration;
	uint16_t currentStage;
	uint16_t stageDuration;
	uint16_t modeDuration;
	
	float U;
	uint16_t temperature;
	float prevTemperature;

	void correctTemperature(float &currentTemp, uint16_t &targetTemp);
	
	xSemaphoreHandle xSemPeriodic;
	xQueueHandle queExchange;
	GpioDriver *gpio;
	
	
	AdcDriver *adc;

	DisplayDriver *display;
	
	BaseType_t xReturned;
	TaskHandle_t xHandle = NULL;
	TaskHandle_t xHandleExchange = NULL;

	uint8_t helperBuf[256];
	
	MyList *lstPrograms;
	WorkModeEdit *lstProgramsEdit;
	Widget *p_widget;
	
	std::vector<WorkMode> m_programs;
	
	RomParams gParams;
	
	bool isErrorSensor1{false};
	bool isErrorSensor2 {false};

};

