#pragma once

#include <stdint.h>
class Rtc {
public:
	static Rtc &instance();
	void initRtc();
	void getTime(uint8_t &h, uint8_t &m, uint8_t &s);
	uint8_t getMinute();
	uint8_t getHoure();
	void getDate(uint16_t &y, uint8_t &m, uint8_t &d);
	void setTime(uint8_t &h, uint8_t &m, uint8_t &s);
	void setDate(uint8_t &y, uint8_t &m, uint8_t &d);
};

