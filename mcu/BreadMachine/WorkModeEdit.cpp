#include "WorkModeEdit.h"
#include <string.h>

WorkModeEdit::WorkModeEdit(DisplayDriver *p, uint16_t maxNumItem) : MyList(p, maxNumItem), isEdited(false)
{
	
}
Widget* WorkModeEdit::keyEvent(uint16_t key)
{
	MyList::keyEvent(key);
	switch (key)
	{
	case ReturnCodeKeyExitFromMenu:
		return m_prevWidget;
		break;
	case ReturnCodeKeyViewWorkMode:
		isEdited = false;
		currentStage = 0;
		tempWMode = pWModes->at(indexCommon);
		paintNameWorkMode(AddrNameV);
		paintSettingsWorkMode();
		break;
	case ReturnCodeKeyEditWorkMode:
		isEdited = true;
		currentStage = 0;
		tempWMode = pWModes->at(indexCommon);
		paintNameWorkMode(AddrNameE);
		paintSettingsWorkMode(isEdited);
		break;
	case ReturnCodeKeySwitchWorkModeUp:
		currentStage = (currentStage + 1) % pWModes->at(indexCommon).numStage;
		paintSettingsWorkMode(isEdited);
		break;
	case ReturnCodeKeySwitchWorkModeDown:
		currentStage--;
		if (currentStage < 0) currentStage = pWModes->at(indexCommon).numStage - 1;
		paintSettingsWorkMode(isEdited);
		break;
	case ReturnCodeKeyAddWorkMode:
		break;
	case ReturnCodeKeyDeleteWorkMode:
		break;
	case ReturnCodeKeySaveWorkMode:
		break;
	default:
		break;
	}
	return this;
}

void WorkModeEdit::changeParams(const uint16_t id, uint8_t len, uint8_t* data)
{	
	uint16_t *p16 = (uint16_t*)data;
	
	switch(id) {
	case AddrNameE:
		
	break;		
	case AddrNumStageE: 
		
	break;	
	case AddrTimeStageE:
		
	break;	
	case AddrTempStageE: 
		tempWMode.stages[currentStage].temperature = *p16;
	break;	
	case AddrWaterStageE: 
		tempWMode.stages[currentStage].waterVolume = *p16;
	break;	
	case AddrDamperStageE: 
		tempWMode.stages[currentStage].damper = *p16;
	break;
	case AddrFanStageE: 
		tempWMode.stages[currentStage].fan = *p16;
	break;	
	}
}

void WorkModeEdit::paintNameWorkMode(uint16_t addrItem)
{
	m_display->sendToDisplay(addrItem, std::string(tempWMode.nameMode, tempWMode.nameMode + MaxLengthNameMode));
	
}
void WorkModeEdit::paintSettingsWorkMode(bool isEdited_)
{
	uint16_t offeset = isEdited_ ? 0x100 : 0;
	char buf[10];
	memset(buf, 0, 10);
	uint16_t len;
	len = sprintf(buf, "%d/%d  ", currentStage + 1, tempWMode.numStage);
	m_display->sendToDisplay(AddrNumStageV+offeset, len, (uint8_t*)buf);
	memset(buf, 0, 10);
	len = sprintf(buf, "%02d:%02d", tempWMode.stages[currentStage].duration / 60, tempWMode.stages[currentStage].duration % 60);
	m_display->sendToDisplay(AddrTimeStageV + offeset, len, (uint8_t*)buf);
	m_display->sendToDisplay(AddrTempStageV + offeset, tempWMode.stages[currentStage].temperature);
	m_display->sendToDisplay(AddrWaterStageV + offeset, tempWMode.stages[currentStage].waterVolume);
	m_display->sendToDisplay(AddrDamperStageV + offeset, tempWMode.stages[currentStage].damper);
	m_display->sendToDisplay(AddrFanStageV + offeset, tempWMode.stages[currentStage].fan);
		
}


