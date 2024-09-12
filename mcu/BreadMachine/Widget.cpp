#include "Widget.h"


Widget::Widget()  
{
	m_prevWidget = nullptr;
}

void Widget::setOutputDevice(DisplayDriver *display)
{
	m_display = display;
}