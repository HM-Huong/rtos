#define LED 2	 // built-in LED
#define BUTTON 0 // built-in button (boot button)

static const int RESET_PRESS = -998;

static QueueHandle_t queue;

// debounce only when the button is pressed
static void debounce_task(void *pvParameters) {
	uint32_t level, state = 0;
	const uint32_t mask = 0x7FFFFFFF;
	int event, last = 0;

	while (1) {
		// ! is used to convert the level to a boolean value (0 or 1)
		level = !digitalRead(BUTTON);
		// each bit in state represents the last 32 readings
		state = (state << 1) | level;

		// if all the bits are 1 then the button is pressed
		if ((state & mask) == mask) {
			event = 1;
		} else { // otherwise the button is released
			event = -1;
		}

		// if the button state has changed then send an event
		if (event != last) {
			if (xQueueSend(queue, &event, 0) == pdTRUE) {
				last = event;
			} else if (event < 0) {
				// the queue is full, and the button released event is critical
				// so we clear the queue and send the RESET_PRESS event
				do {
					xQueueReset(queue);
				} while (xQueueSend(queue, &RESET_PRESS, 0) != pdTRUE);
				last = event;
			}
		}

		// share the CPU with led_task
		taskYIELD();
	}
}

static void led_task(void *pvParameters) {
	BaseType_t s;
	int event;
	bool led = false;

	digitalWrite(LED, LOW);

	while (1) {
		s = xQueueReceive(queue, &event, portMAX_DELAY);
		assert(s == pdTRUE);
		if (event == RESET_PRESS) {
			digitalWrite(LED, LOW);
			printf("!!!Reset!!!\n");
			continue;
		}
		// if button is pressed then toggle the LED
		if (event > 0) {
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

	queue = xQueueCreate(2, sizeof(int));
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