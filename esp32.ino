#define GPIO 2

/*
In a single-core MCU (Micro-Controller Unit), only one task can execute at any instance in time. The task that is executing runs until a hardware timer indicates that the time slice has expired. At timeout, the FreeRTOS scheduler saves the state of the current task by saving its registers. The current task is said to have been preempted by the timer.

The scheduler then chooses another task that is ready to run. The state of the highest priority task, which is ready to run, is restored and resumed where it left off. The duration of the time slice is small enough, that the MCU can run several tasks per second. This is known as concurrent processing.
*/

static void gpioOn(void *pvParameters) {
	while (1) {
		digitalWrite(GPIO, HIGH);
	}
}

static void gpioOff(void *pvParameters) {
	while (1) {
		digitalWrite(GPIO, LOW);
	}
}

// when running
// loopTask has priority 1
// IDLE1 task has priority 0
// ipc1 task has priority 24, but it often waits for an event
void setup() {
	delay(1000); // wait for the serial to be ready
	printf("----- setup\n");

	// get the core this code is running on
	int app_cpu = xPortGetCoreID();
	printf("app_cpu = %d\n", app_cpu);

	pinMode(GPIO, OUTPUT);
	// we use assert macro to check if the task was created successfully
	// if not (the expression is false), the error is reported to the serial monitor and the program aborts
	// uncomment the line below to see the error:
	// assert(0 == 1);
	assert(xTaskCreatePinnedToCore(gpioOn, "gpioOn", 2048, NULL, 1, NULL, app_cpu) == pdPASS);
	assert(xTaskCreatePinnedToCore(gpioOff, "gpioOff", 2048, NULL, 1, NULL, app_cpu) == pdPASS);
	printf("----- setup done\n");
}

void loop() {
	vTaskDelete(xTaskGetCurrentTaskHandle());
}