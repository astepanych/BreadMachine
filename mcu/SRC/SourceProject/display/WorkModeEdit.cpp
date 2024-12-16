#include "WorkModeEdit.h"
#include <string.h>

WorkModeEdit::WorkModeEdit(DisplayDriver *p, uint16_t maxNumItem)
	: MyList(p, maxNumItem), stateEdited(StateView)
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
		if (pWModes->size() == 0)
			break;
		stateEdited = StateView;
		currentStage = 0;
		tempWMode = pWModes->at(indexCommon);
		paintNameWorkMode(AddrNameV);
		paintSettingsWorkMode();
		break;
	case ReturnCodeKeyEditWorkMode:
		if (pWModes->size() == 0)
			break;
		stateEdited = StateEdit;
		currentStage = 0;
		tempWMode = pWModes->at(indexCommon);
		paintNameWorkMode(AddrNameE);
		paintSettingsWorkMode(stateEdited != StateView);
		break;
	case ReturnCodeKeySwitchWorkModeUp:
		currentStage = (currentStage + 1) % tempWMode.numStage;
		paintSettingsWorkMode(stateEdited != StateView);
		break;
	case ReturnCodeKeySwitchWorkModeDown:
		currentStage--;
		if (currentStage < 0) currentStage = tempWMode.numStage - 1;
		paintSettingsWorkMode(stateEdited != StateView);
		break;
	case ReturnCodeKeyAddWorkMode:
		stateEdited = StateNew;
		currentStage = 0;
		tempWMode.reset();
		paintNameWorkMode(AddrNameE);
		paintSettingsWorkMode(stateEdited != StateView);
		break;
	case ReturnCodeKeyDeleteWorkMode: {
			if (pWModes->size() == 0)
				break;
			auto iter = pWModes->cbegin(); // указатель на первый элемент
			pWModes->erase(iter + indexCommon);
		
			if (pWModes->size() == indexCommon) {
				indexCommon--;
			}
			if (indexDisplay == pWModes->size())
				indexDisplay--;
			if (startIndex + maxItemDisplay >= pWModes->size() && startIndex != 0)
				startIndex--;
			updateDisplay();
			break;
		}
	case ReturnCodeKeySaveWorkMode:
		if (stateEdited == StateEdit)
		{
			pWModes->at(indexCommon) = tempWMode;
		}
		if (stateEdited == StateNew)
		{
			pWModes->push_back(tempWMode);
		}
		saveWorkModes();
		updateDisplay();
		break;
	case ReturnCodeKeyAddStageWorkMode:
		if (tempWMode.numStage < MaxStageMode) {
			tempWMode.stages[tempWMode.numStage].damper = 0;
			tempWMode.stages[tempWMode.numStage].fan = 0;
			tempWMode.stages[tempWMode.numStage].duration = 900;
			tempWMode.stages[tempWMode.numStage].temperature = 180;
			tempWMode.stages[tempWMode.numStage].waterVolume = 1000;
			tempWMode.numStage++;
			paintSettingsWorkMode(stateEdited != StateView);
		}
		
		break;
	case ReturnCodeKeyDeleteStageWorkMode:
		if (tempWMode.numStage > 0) {
			memmove(&tempWMode.stages[currentStage], &tempWMode.stages[currentStage + 1], sizeof(StageWorkMode)*(MaxStageMode - currentStage - 1));
			tempWMode.numStage--;
			if (currentStage == tempWMode.numStage)
				currentStage--;
			paintSettingsWorkMode(stateEdited != StateView);
		}
		
		break;
	default:
		break;
	}
	return this;
}

void WorkModeEdit::changeParams(const uint16_t id, uint8_t len, uint8_t* data)
{	
	u16be valBe;
	memcpy(&valBe, data + 1, 2);
	uint16_t val = (uint16_t)valBe;
	
	switch(id) {
	case AddrNameE: {
		tempWMode.lenNameMode = 0;
		uint8_t *pName = data + 1;
		while (1)
		{
			if (*pName == 0xff)
			{
				break;
			}
			pName++;
			tempWMode.lenNameMode++;
		}
		memcpy(tempWMode.nameMode, data + 1, tempWMode.lenNameMode+2);
		to1251((uint8_t*)tempWMode.nameMode, tempWMode.lenNameMode);
		
	}
		
	break;		
	case AddrNumStageE: 
		
	break;	
	case AddrTimeStageE:
		tempWMode.stages[currentStage].duration = data[1] * 60 + data[2];
			printAllTimeMode();
	break;	
	case AddrTempStageE: 
		tempWMode.stages[currentStage].temperature = val;
	break;	
	case AddrWaterStageE: 
		tempWMode.stages[currentStage].waterVolume = val;
	break;	
	case AddrDamperStageE: 
		tempWMode.stages[currentStage].damper = val;
	break;
	case AddrFanStageE: 
		tempWMode.stages[currentStage].fan = val;
		
	break;	
	}
}

void WorkModeEdit::paintNameWorkMode(uint16_t addrItem)
{
	
	if (stateEdited != StateView)
		toMyCodec((uint8_t*)tempWMode.nameMode, tempWMode.lenNameMode);
	m_display->sendToDisplayStr(addrItem, tempWMode.lenNameMode, tempWMode.nameMode);
	if (stateEdited != StateView)
		to1251((uint8_t*)tempWMode.nameMode, tempWMode.lenNameMode);
}

void WorkModeEdit::to1251(uint8_t *buf, const uint16_t len) {
	for (int i = 0; i < len; i++) {
		if ((uint8_t)buf[i] == 0x5f) {
			buf[i] = (char)0xA8;
		}
		else if (buf[i] >= 0x60 && buf[i] <= 0x7f) {
			buf[i] = (char)(buf[i] + 0x60);
		}
	}
}

void WorkModeEdit::toMyCodec(uint8_t *buf, const uint16_t len) {
	for (int i = 0; i < len; i++) {
		if ((uint8_t)buf[i] == 0xA8) {
			buf[i] = (char)0x5f;
		}
		else if (buf[i] >= 0xC0 && buf[i] <= 0xDF) {
			buf[i] = (char)(buf[i] - 0x60);
		}
	}
}

void WorkModeEdit::printAllTimeMode()
{
	char buf[10];
	int commonDur = 0;
	memset(buf, 0xff, 10);
	for (int i = 0; i < tempWMode.numStage; i++) {
		commonDur += tempWMode.stages[i].duration;
	}
	int len = sprintf(buf, "%d:%d  ", commonDur / 60, commonDur % 60);
	m_display->sendToDisplay(AddrNumTimeModeEdit, len, (uint8_t*)buf);
}

void WorkModeEdit::paintSettingsWorkMode(bool isEdited_)
{
	uint16_t offeset = isEdited_ ? 0x100 : 0;
	char buf[10];
	memset(buf, 0, 10);
	uint16_t len;
	len = sprintf(buf, "%d/%d  ", currentStage + 1, tempWMode.numStage);
	m_display->sendToDisplay(AddrNumStageV+offeset, len, (uint8_t*)buf);
	memset(buf, 0xff, 10);
	printAllTimeMode();
	uint16_t time = ((tempWMode.stages[currentStage].duration / 60) << 8) + tempWMode.stages[currentStage].duration % 60;
	m_display->sendToDisplay(AddrTimeStageV + offeset, time);
	m_display->sendToDisplay(AddrTempStageV + offeset, tempWMode.stages[currentStage].temperature);
	m_display->sendToDisplay(AddrWaterStageV + offeset, tempWMode.stages[currentStage].waterVolume);
	m_display->sendToDisplay(AddrDamperStageV + offeset, tempWMode.stages[currentStage].damper);
	m_display->sendToDisplay(AddrFanStageV + offeset, tempWMode.stages[currentStage].fan);
		
}


