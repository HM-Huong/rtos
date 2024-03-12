#define LED1 12
#define LED2 2
#define LED3 14

struct s_led {
	byte gpio;
	byte state;
	unsigned napms; // nap time in milliseconds
	TaskHandle_t task;
};

static struct s_led leds[] = {
	{LED1, 0, 500, 0},
	{LED2, 0, 200, 0},
	{LED3, 0, 750, 0}};

static void led_task(void *argp) {
	struct s_led *led = (struct s_led *)argp;
	unsigned stack_hwm = 0, tmp;
	// printf("Task for GPIO %d is running on core %d\n", led->gpio, xPortGetCoreID());
	delay(1000); // allow the setup() to finish before the task gets underway
	while (1) {
		digitalWrite(led->gpio, led->state ^= 1);
		// get the number of free stack bytes
		tmp = uxTaskGetStackHighWaterMark(NULL);
		if (!stack_hwm || tmp < stack_hwm) {
			stack_hwm = tmp;
			// if there was no call to printf in the previous,
			// in the next report, the free stack bytes
			// will be decremented because of the call
			// to printf (which uses the stack)
			printf("Task for GPIO %d has stack HWM of %u bytes, free heap = %u\n", led->gpio, stack_hwm, xPortGetFreeHeapSize());
		}
		vTaskDelay(pdMS_TO_TICKS(led->napms));
	}
}

// the setup and loop functions belong to the loopTask (has priority 1)
void setup() {
	delay(1000); // wait for the serial to be ready
	printf("----- setup\n");

	// get the core this code is running on
	int app_cpu = xPortGetCoreID();
	printf("app_cpu = %d\n", app_cpu);

	for (auto &led : leds) {
		pinMode(led.gpio, OUTPUT);
		digitalWrite(led.gpio, led.state);
		xTaskCreatePinnedToCore(led_task, "led_task", 2048, &led, 1, &led.task, app_cpu);
		printf("Created task for GPIO %d\n", led.gpio);
	}
	printf("----- setup done\n");
	printf("There're %u heap bytes available\n", xPortGetFreeHeapSize());
}

void loop() {
	delay(5000);
	printf("Suspending middle LED task (GPIO %d)\n", leds[1].gpio);
	vTaskSuspend(leds[1].task);
	delay(5000);
	printf("Resuming middle LED task (GPIO %d)\n", leds[1].gpio);
	vTaskResume(leds[1].task);
}