// Created a read to go task, must call vTaskResume() to start it
BaseType_t inline createSuspendedTask(
	TaskFunction_t taskFunction,
	const char *const name,
	const uint32_t stackDepth,
	void *const parameters,
	UBaseType_t priority,
	TaskHandle_t *const taskHandle,
	const BaseType_t coreID = tskNO_AFFINITY
) {
	BaseType_t rc;
	rc = xTaskCreatePinnedToCore(taskFunction, name, stackDepth, parameters, 0, taskHandle, coreID);
	if (rc != pdPASS) {
		return rc;
	}

	vTaskSuspend(*taskHandle);
	vTaskPrioritySet(*taskHandle, priority);
	return rc;
}

TaskHandle_t task1Handle, task2Handle;

void task2(void *pvParameters) {
	printf("Task 2 running on core %d\n", xPortGetCoreID());
	for (;;) {
		delay(1000);
	}
}

void task1(void *pvParameters) {
	printf("Task 1 running on core %d\n", xPortGetCoreID());
	printf("Creating Task 2 ...\n");
	createSuspendedTask(task2, "Task 2", 2048, NULL, 4, &task2Handle, xPortGetCoreID());
	delay(1);
	vTaskResume(task2Handle);
	printf("Task 2 created and suspended\n"); // print after line 22
	for (;;) {
		delay(1000);
	}
}

void setup() {
	delay(1000); // Wait for the serial monitor to open
	BaseType_t rc, app_cpu = xPortGetCoreID();

	printf("Demo of creating a suspended task\n");

	printf("Creating Task 1 ...\n");
	rc = createSuspendedTask(task1, "Task 1", 2048, NULL, 1, &task1Handle, app_cpu);
	assert(rc == pdPASS);
	vTaskDelay(1);
	printf("Task 1 created and suspended\n");
	vTaskResume(task1Handle);
}

void loop() {
	delay(1000);
}