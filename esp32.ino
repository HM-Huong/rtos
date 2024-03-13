#define LED 2	 // built-in LED
#define BUTTON 0 // built-in button (boot button)

#include "AlertLED.hpp"

static AlertLED alert_led(LED, 1000);
static unsigned loop_count = 0;

void setup() {
	delay(1000); // Wait for the serial console to open
	alert_led.alert();
}

void loop() {
	if (loop_count >= 70) {
		loop_count = 0;
		alert_led.alert();
	}

	delay(100);

	++loop_count;
	if (loop_count >= 50) {
		alert_led.stop();
	}
}