#pragma once
#include "MyList.h"

#define StartAddrListEditItemsVP 0x64a0
#define StartAddrListEditItemsSP 0x6400
#define OffsetColorsText 3
#define StepListEditVP 0x40
#define StepListEditSP 0x20
#define AddrScrolBar 0x65e0

#define AddrNameV  0x6620
#define AddrNumStageV  0x6660
#define AddrTimeStageV  0x6670
#define AddrTempStageV  0x6680
#define AddrWaterStageV  0x6682
#define AddrDamperStageV  0x6684
#define AddrFanStageV  0x6686


#define AddrNameE			0x6720
#define AddrNumStageE		0x6760
#define AddrTimeStageE		0x6770
#define AddrTempStageE		0x6780
#define AddrWaterStageE		0x6782
#define AddrDamperStageE	0x6784
#define AddrFanStageE		0x6786


class WorkModeEdit :
    public MyList
{
public:
	WorkModeEdit(DisplayDriver *p, uint16_t maxNumItem);
	Widget* keyEvent(uint16_t key);
	void changeParams(const uint16_t id, uint8_t len, uint8_t* data);
private:
	void paintNameWorkMode(uint16_t addrItem);
	void paintSettingsWorkMode(bool isEdited = false);
	WorkMode tempWMode;
	int16_t currentStage;
	bool isEdited;
};

