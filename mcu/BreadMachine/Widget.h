#pragma once
#include "DisplayDriver.h"
#include "KeyEvent.h"

enum Colors
{
	ColorBlack = 0x0000,
	ColorRed = 0xf800,
	ColorGrey = 0xc618,
	ColorGreen = 0x07e0,
};

class Widget
{
public:
	Widget();
	virtual Widget* keyEvent(uint16_t key) = 0;
	virtual void updateDisplay() {};
	virtual void resetWidget() {};
	virtual void changeParams(const uint16_t id, uint8_t len, uint8_t* data) {};
	void setOutputDevice(DisplayDriver *display);
	void setPrevWidgwt(Widget *w)
	{m_prevWidget = w;}
protected:
	DisplayDriver *m_display;
	Widget *m_prevWidget;
};

