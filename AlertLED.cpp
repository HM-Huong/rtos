#include "AlertLED.hpp"

AlertLED::AlertLED(int gpio, unsigned period_ms) {
	this->gpio = gpio;
	this->period_ms = period_ms;
	pinMode(this->gpio, OUTPUT);
	digitalWrite(this->gpio, LOW);
}

void AlertLED::reset(bool state) {
	this->state = state;
	this->count = 0;
	digitalWrite(this->gpio, this->state);
}

void AlertLED::alert() {
	if (timer_handle == NULL) {
		timer_handle = xTimerCreate("alert_tmr", pdMS_TO_TICKS(period_ms / 20), pdTRUE, this, AlertLED::callback);
		assert(timer_handle);
	}
	AlertLED::reset(true);
	xTimerStart(timer_handle, portMAX_DELAY);
}

void AlertLED::stop() {
	if (this->timer_handle) {
		xTimerStop(this->timer_handle, portMAX_DELAY);
		digitalWrite(this->gpio, LOW);
	}
}

void AlertLED::callback(TimerHandle_t th) {
	AlertLED *obj = (AlertLED *)pvTimerGetTimerID(th);
	assert(obj->timer_handle == th);
	obj->state ^= true;
	digitalWrite(obj->gpio, obj->state);

	++obj->count;
	if (obj->count == 9) {
		xTimerChangePeriod(th, pdMS_TO_TICKS(obj->period_ms / 20 + obj->period_ms / 2), portMAX_DELAY);
		assert(!obj->state);
	} else if (obj->count > 9) {
		obj->reset(true);
		xTimerChangePeriod(th, pdMS_TO_TICKS(obj->period_ms / 20), portMAX_DELAY);
	}
}