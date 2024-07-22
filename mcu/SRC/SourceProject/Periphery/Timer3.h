#pragma once

#include <functional>

#define TimeoutTim3 (999)
class Timer3
{
public:
	Timer3();
	~Timer3();
	
	static Timer3* instance() {return m_instance;};
	static void init();
	
	static void start();
	
	static void stop();
	
	static std::function<void()> handlerTimeout;
		

private:
	static Timer3 *m_instance;
	
	static const int m_timeout = 999;
};
