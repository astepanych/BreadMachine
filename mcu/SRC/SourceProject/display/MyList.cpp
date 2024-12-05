#include "MyList.h"


MyList::MyList(DisplayDriver *p, uint16_t maxNumItem)
{
	m_display = p;
	indexDisplay = indexCommon = 0;
	maxItemDisplay = maxNumItem;
	maxPosItemDisplay = maxNumItem - 1;
	addrScrollValue = 0;
	startIndex = 0;
}
Widget* MyList::keyEvent(uint16_t key)
{
	switch (key)
	{
	case ReturnCodeKeyDownSelectProgramm: {
			if (pWModes->size() == 0)
				break;
			int printNumDisplay = pWModes->size() >= maxItemDisplay ? maxItemDisplay : pWModes->size() ;
			maxPosItemDisplay = printNumDisplay - 1;
			indexCommon++;
			if (pWModes->size() == indexCommon)
			{
				for (int i = 0; i < printNumDisplay; i++)
				{
					m_display->sendToDisplayStr(lst.at(i).addrText, pWModes->at(i).lenNameMode, pWModes->at(i).nameMode);
					vTaskDelay(2 / portTICK_PERIOD_MS);
				}
				
				m_display->sendToDisplay(lst.at(maxPosItemDisplay).addrColor, ColorBlack);
				m_display->sendToDisplay(lst.at(0).addrColor, ColorRed);
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
					int index = startIndex = indexCommon - maxPosItemDisplay;
					for (int i = 0; i < printNumDisplay; i++)
					{
					
						m_display->sendToDisplayStr(lst.at(i).addrText, pWModes->at(index).lenNameMode, pWModes->at(index).nameMode);
						index++;
						vTaskDelay(2 / portTICK_PERIOD_MS);
					}
				}
			}
			m_display->sendToDisplay(addrScrollValue, indexCommon*maxValueScroll / (pWModes->size() - 1));
			break;
		}
	case ReturnCodeKeySaveProgramm:
		currentIndex = indexDisplay;
		currentPos = indexCommon;
		changeValue(currentPos);
		break;
	case ReturnCodeKeyInMenuListProgramms:
		
		break;
	case ReturnCodeKeyUpSelectProgramm: {
			if (pWModes->size() == 0)
				break;
			int i = 0;
			int printNumDisplay = pWModes->size() >= maxItemDisplay ? maxItemDisplay : pWModes->size() ;
			maxPosItemDisplay = printNumDisplay - 1;
			indexCommon--;
			if (indexCommon == -1)
			{
				indexCommon = pWModes->size() - 1;
				indexDisplay = maxPosItemDisplay;
				int index = startIndex = indexCommon - maxPosItemDisplay;
				for (i = 0; i < printNumDisplay; i++)
				{
					m_display->sendToDisplayStr(lst.at(i).addrText, pWModes->at(index).lenNameMode, pWModes->at(index).nameMode);
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
					int index = startIndex = indexCommon;
					for (int i = 0; i < printNumDisplay; i++)
					{
						m_display->sendToDisplayStr(lst.at(i).addrText, pWModes->at(index).lenNameMode, pWModes->at(index).nameMode);
						index++;
						vTaskDelay(2 / portTICK_PERIOD_MS);
					}
				}
			}
			m_display->sendToDisplay(addrScrollValue, indexCommon*maxValueScroll / (pWModes->size() - 1));
			break;
		}
	default:
		break;
	}
	return this;
}

void MyList::updateDisplay()
{
	int i = 0;
	if (pWModes->size() > 0)
	{
		int printNumDisplay = pWModes->size()  >= maxItemDisplay ? maxItemDisplay : pWModes->size();
		for (i = 0; i < printNumDisplay; i++) {
			m_display->sendToDisplayStr(lst.at(i).addrText, pWModes->at(startIndex + i).lenNameMode, pWModes->at(startIndex + i).nameMode);
			vTaskDelay(2 / portTICK_PERIOD_MS);
			m_display->sendToDisplay(lst.at(i).addrColor, ColorBlack);
		}

		m_display->sendToDisplay(lst.at(indexDisplay).addrColor, ColorRed);
		m_display->sendToDisplay(addrScrollValue, indexCommon*maxValueScroll / (pWModes->size() - 1));
	}
	for (; i < maxItemDisplay; i++)
	{
		m_display->sendToDisplayStr(lst.at(i).addrText, 0, nullptr);
	}
}

void MyList::resetWidget() {
	
	int i = 0;
	if (pWModes->size() > 0)
	{
		
		int printNumDisplay = pWModes->size() >= maxItemDisplay ? maxItemDisplay : pWModes->size();
		indexDisplay = indexCommon = 0;
		for (i = 0; i < printNumDisplay; i++)
		{
			m_display->sendToDisplayStr(lst.at(i).addrText, pWModes->at(i).lenNameMode, pWModes->at(i).nameMode);
			m_display->sendToDisplay(lst.at(i).addrColor, ColorBlack);
			vTaskDelay(2 / portTICK_PERIOD_MS);
		}
		m_display->sendToDisplay(lst.at(0).addrColor, ColorRed);
	}
	m_display->sendToDisplay(addrScrollValue, 0);
	
	for (; i < maxItemDisplay; i++)
	{
		m_display->sendToDisplayStr(lst.at(i).addrText, 0, nullptr);
	}
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
