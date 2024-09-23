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

#define NumItemList  (7)
#define NumItemListEdit  (5)



constexpr uint16_t xProgresStage = 17;
constexpr uint16_t yProgresStage = 175;
constexpr uint16_t hProgresStage = 60;
constexpr uint16_t wProgresStage = 568;


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

private:
	
	static void parsePackDisplay(const uint16_t id, uint8_t len, uint8_t* data);
	
	static void keyEvent(uint16_t key);
	void initHal();
	void initOsal();
	
	static void initText();
	static void taskPeriodic(void *p);

	static GpioDriver *gpio;
	
	
	static AdcDriver *adc;

	static DisplayDriver *display;
	
	BaseType_t xReturned;
	TaskHandle_t xHandle = NULL;

	static uint8_t helperBuf[256];
	
	static MyList *lstPrograms;
	static WorkModeEdit *lstProgramsEdit;
	static Widget *p_widget;
	
	static std::vector<WorkMode> m_programs;
	static void initDefaultPrograms();
	static void readPrograms();
	static void writePrograms();
	
	static void fillProgram(const std::string &name, const uint16_t numStages, const uint16_t *stage = nullptr);
	static void updateProgressBar(uint16_t value);
	
	static void paintStageProgress();
	static WorkMode currentWorkMode;
	
	static void getSizeWRectangle(const WorkMode &mode, uint16_t *wList);
	static void updateParamStage();
	static void updateTime(uint16_t sec);
	
	
	static uint16_t stateRun;
	static uint32_t commonDuration;
	static uint16_t currentStage;
	static uint16_t stageDuration;
	static uint16_t modeDuration;
	
	static float U;
	static uint16_t temperature;
	static void correctTemperature(float &currentTemp, uint16_t &targetTemp);
	
	
	

};

