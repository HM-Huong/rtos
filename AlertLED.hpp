#pragma once

#include <Arduino.h>

class AlertLED {
	TimerHandle_t timer_handle = NULL;
	volatile bool state;
	volatile unsigned count;
	unsigned period_ms;
	int gpio;

	void reset(bool state);

public:
	AlertLED(int gpio, unsigned period_ms = 1000);
	void alert();
	void stop();

	static void callback(TimerHandle_t th);
};