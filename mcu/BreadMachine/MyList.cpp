#include "MyList.h"


MyList::MyList(DisplayDriver *p, uint16_t maxNumItem)
{
	m_display = p;
	indexDisplay = indexCommon = 0;
	maxItemDisplay = maxNumItem;
	maxPosItemDisplay = maxNumItem - 1;
	addrScrollValue = 0;
}
Widget* MyList::keyEvent(uint16_t key)
{
	switch (key)
	{
	case ReturnCodeKeyDownSelectProgramm:
		indexCommon++;
		if (pWModes->size() == indexCommon)
		{
			for (int i = 0; i < maxItemDisplay; i++)
			{
				m_display->sendToDisplay(lst.at(i).addrText, std::string(pWModes->at(i).nameMode, pWModes->at(i).nameMode + MaxLengthNameMode));
				vTaskDelay(2 / portTICK_PERIOD_MS);
			}
			m_display->sendToDisplay(lst.at(0).addrColor, ColorRed);
			m_display->sendToDisplay(lst.at(maxPosItemDisplay).addrColor, ColorBlack);
			indexCommon = 0;
			indexDisplay = 0;
			
		}
		else {
			if (indexDisplay != maxPosItemDisplay)
			{
				m_display->sendToDisplay(lst.at(indexDisplay).addrColor, ColorBlack);
				indexDisplay++;
				m_display->sendToDisplay(lst.at(indexDisplay).addrColor, ColorRed);
			}
			else
			{
				int index = indexCommon - maxPosItemDisplay;
				for (int i = 0; i < maxItemDisplay; i++)
				{
					m_display->sendToDisplay(lst.at(i).addrText, std::string(pWModes->at(index).nameMode, pWModes->at(index).nameMode + MaxLengthNameMode));
					index++;
					vTaskDelay(2 / portTICK_PERIOD_MS);
				}
			}
		}
		m_display->sendToDisplay(addrScrollValue, indexCommon*maxValueScroll / (pWModes->size() - 1));
		break;
	case ReturnCodeKeySaveProgramm:
		currentIndex = indexDisplay;
		currentPos = indexCommon;
		changeValue(currentPos);
		

		break;
	case ReturnCodeKeyInMenuListProgramms:
		
		break;
	case ReturnCodeKeyUpSelectProgramm:
		indexCommon--;
		if (indexCommon == -1)
		{
			indexCommon = pWModes->size() - 1;
			indexDisplay = maxPosItemDisplay;
			int index = indexCommon - maxPosItemDisplay;
			for (int i = 0; i < maxItemDisplay; i++)
			{
				m_display->sendToDisplay(lst.at(i).addrText, std::string(pWModes->at(index).nameMode, pWModes->at(index).nameMode + MaxLengthNameMode));
				index++;
				vTaskDelay(2 / portTICK_PERIOD_MS);
			}
			m_display->sendToDisplay(lst.at(0).addrColor, ColorBlack);
			m_display->sendToDisplay(lst.at(maxPosItemDisplay).addrColor, ColorRed);

			
		}
		else {
			if (indexDisplay != 0)
			{
				m_display->sendToDisplay(lst.at(indexDisplay).addrColor, ColorBlack);
				indexDisplay--;
				m_display->sendToDisplay(lst.at(indexDisplay).addrColor, ColorRed);
			}
			else
			{
				int index = indexCommon;
				for (int i = 0; i < maxItemDisplay; i++)
				{
					m_display->sendToDisplay(lst.at(i).addrText, std::string(pWModes->at(index).nameMode, pWModes->at(index).nameMode + MaxLengthNameMode));
					index++;
					vTaskDelay(2 / portTICK_PERIOD_MS);
				}
			}
		}
		m_display->sendToDisplay(addrScrollValue, indexCommon*maxValueScroll / (pWModes->size() - 1));
		break;
	default:
		break;
	}
	return this;
}

void MyList::updateDisplay()
{
	int index = 0;
	index = indexCommon < maxItemDisplay ? 0 : indexCommon - maxItemDisplay;
	for (int i = 0; i < maxItemDisplay; i++)
	{
		
		m_display->sendToDisplay(lst.at(i).addrText, std::string(pWModes->at(index+i).nameMode, pWModes->at(index+i).nameMode + MaxLengthNameMode));
		vTaskDelay(2 / portTICK_PERIOD_MS);
		m_display->sendToDisplay(lst.at(i).addrColor, ColorBlack);
	}
	m_display->sendToDisplay(lst.at(0).addrColor, ColorRed);
	m_display->sendToDisplay(addrScrollValue, indexCommon*maxValueScroll / (pWModes->size() - 1));
}

void MyList::resetWidget() {
	indexDisplay = indexCommon = 0;
	for (int i = 0; i < maxItemDisplay; i++)
	{
		m_display->sendToDisplay(lst.at(i).addrText, std::string(pWModes->at(i).nameMode, pWModes->at(i).nameMode + MaxLengthNameMode));
		m_display->sendToDisplay(lst.at(i).addrColor, ColorBlack);
		vTaskDelay(2 / portTICK_PERIOD_MS);
	}
	m_display->sendToDisplay(addrScrollValue, 0);
	m_display->sendToDisplay(lst.at(0).addrColor, ColorRed);
}


void MyList::addItemHard(ElementList &el)
{
	lst.insert(lst.end(), el);

}
	 
std::string MyList::text(uint16_t index)
{
	return std::string(pWModes->at(index).nameMode, pWModes->at(index).nameMode + MaxLengthNameMode);
}
ElementList MyList::at(int index)
{
	return lst.at(index);
}
int MyList::length()
{
	return lst.size();
}
void MyList::clear()
{
	lst.clear();
	indexDisplay = indexCommon = 0;
}
