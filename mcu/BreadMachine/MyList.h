#pragma once
#include "Widget.h"
#include "WorkMode.h"
#include <list>
#include <string>
#include <vector>
#include <functional>

struct ElementList
{
	uint16_t addrColor;
	uint16_t addrText;
};

class MyList :  public Widget
{
public:
	MyList(DisplayDriver *p, uint16_t maxNumItem);
	Widget* keyEvent(uint16_t key);
	void addItemHard(ElementList &el);

	void setWModes(std::vector<WorkMode> *p)
	{
		pWModes = p;
	};
	 
	std::string text(uint16_t index);

	ElementList at(int index);
	int length();
	void clear();
	void updateDisplay();
	void resetWidget();
	void setAddrScrollValue(uint16_t addr) {
		addrScrollValue = addr;
	};
	
	std::function<void(int)> changeValue;
protected:
	std::vector<ElementList> lst;
	std::vector<WorkMode> *pWModes;
	uint16_t indexDisplay;
	int16_t startIndex;
	int16_t indexCommon;
	uint16_t maxItemDisplay;
	uint16_t maxPosItemDisplay;
	uint16_t currentIndex;
	uint16_t currentPos;
	uint16_t addrScrollValue;
	const uint16_t maxValueScroll = 84;
};

