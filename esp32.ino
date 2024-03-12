#define LED 2	 // built-in LED
#define BUTTON 0 // built-in button (boot button)

static QueueHandle_t queue;

static void debounce_task(void *pvParameters) {
	uint32_t level, state = 0, last = 0xFFFFFFFF;
	uint32_t mask = 0x7FFFFFFF;
	bool event;

	while (1) {
		// !! is used to convert the level to a boolean value (0 or 1)
		level = !!digitalRead(BUTTON);
		// each bit in state represents the last 32 readings
		state = (state << 1) | level;

		// if all the bits are 1 or 0, then the button has been stable
		if ((state & mask) == mask || (state & mask) == 0) {
			// if the current level is different from the last one
			if (level != last) {
				printf("Button %s\n", level ? "released" : "pressed");
				// send an event to the queue and remember the last level
				event = !!level;
				if (xQueueSendToBack(queue, &event, 1) == pdPASS) {
					last = level;
				}
			}
		}
		// share the CPU with led_task
		taskYIELD();
	}
}

static void led_task(void *pvParameters) {
	BaseType_t s;
	bool event, led = false;

	digitalWrite(LED, led);

	while (1) {
		s = xQueueReceive(queue, &event, portMAX_DELAY);
		assert(s == pdTRUE);
		// if button is pressed then toggle the LED
		if (event) {
			led = !led;
			digitalWrite(LED, led);
		}
	}
}

void setup() {
	int app_cpu = xPortGetCoreID();
	TaskHandle_t h;
	BaseType_t rc;

	delay(1000); // Wait for the serial console to open
	queue = xQueueCreate(32, sizeof(bool));
	assert(queue);

	pinMode(LED, OUTPUT);
	pinMode(BUTTON, INPUT_PULLUP);

	rc = xTaskCreatePinnedToCore(debounce_task, "debounce", 2048, NULL, 1, &h, app_cpu);
	assert(rc == pdPASS);
	assert(h);

	rc = xTaskCreatePinnedToCore(led_task, "led", 2048, NULL, 1, &h, app_cpu);
	assert(rc == pdPASS);
	assert(h);
}

void loop() {
	vTaskDelete(NULL);
}