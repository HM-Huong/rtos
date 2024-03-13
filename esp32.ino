#define PREVENT_DEADLOCK 0

static const int N = 4;
static const int N_EATERS = N - 1;

static QueueHandle_t msgQueue;
static SemaphoreHandle_t cSems;
static int app_cpu = 0;

enum State {
	THINKING = 0,
	HUNGRY,
	EATING
};

static const char *stateNames[] = {
	"Thinking",
	"Hungry",
	"Eating"};

struct Philosopher {
	TaskHandle_t task;
	unsigned int num;
	State state;
	unsigned seed;
};

struct Message {
	unsigned int num;
	State state;
};

static Philosopher philosophers[N];
static SemaphoreHandle_t forks[N];
static volatile unsigned logNo = 0;

static void sendState(Philosopher *ph) {
	Message msg;
	BaseType_t rc;
	msg.num = ph->num;
	msg.state = ph->state;
	rc = xQueueSend(msgQueue, &msg, portMAX_DELAY);
}

static void philosopherTask(void *arg) {
	Philosopher *philo = (Philosopher *)arg;
	SemaphoreHandle_t folk1, folk2;
	BaseType_t rc;

	delay(rand_r(&philo->seed) % 5 + 1);
	while (1) {
		philo->state = THINKING;
		sendState(philo);
		delay(rand_r(&philo->seed) % 5 + 1);

		philo->state = HUNGRY;
		sendState(philo);
		delay(rand_r(&philo->seed) % 5 + 1);

#if PREVENT_DEADLOCK
		rc = xSemaphoreTake(cSems, portMAX_DELAY);
		assert(rc == pdTRUE);
#endif

		// pick up forks
		folk1 = forks[philo->num];
		folk2 = forks[(philo->num + 1) % N];
		rc = xSemaphoreTake(folk1, portMAX_DELAY);
		assert(rc == pdTRUE);
		delay(rand_r(&philo->seed) % 5 + 1);
		rc = xSemaphoreTake(folk2, portMAX_DELAY);
		assert(rc == pdTRUE);

		philo->state = EATING;
		sendState(philo);
		delay(rand_r(&philo->seed) % 5 + 1);

		// Put down forks
		rc = xSemaphoreGive(folk1);
		assert(rc == pdTRUE);
		delay(1);
		rc = xSemaphoreGive(folk2);
		assert(rc == pdTRUE);

#if PREVENT_DEADLOCK
		rc = xSemaphoreGive(cSems);
		assert(rc == pdTRUE);
#endif
	}
}

void setup() {
	delay(1000); // Allow time to start the serial monitor

	BaseType_t rc;
	app_cpu = xPortGetCoreID();
	msgQueue = xQueueCreate(30, sizeof(Message));
	assert(msgQueue);

	for (int i = 0; i < N; i++) {
		forks[i] = xSemaphoreCreateBinary();
		assert(forks[i]);
		rc = xSemaphoreGive(forks[i]);
		assert(rc == pdTRUE);
	}

	printf("\n==== The Dining Philosophers's Problem ====\n");
	printf("Number of philosophers: %d\n", N);

#if PREVENT_DEADLOCK
	printf("Deadlock prevention is enabled\n");
	cSems = xSemaphoreCreateCounting(N_EATERS, N_EATERS);
	assert(cSems);
#else
	printf("Deadlock prevention is disabled\n");
	cSems = NULL;
#endif

	for (unsigned i = 0; i < N; ++i) {
		philosophers[i].num = i;
		philosophers[i].state = THINKING;
		philosophers[i].seed = hallRead();
		// philosophers[i].seed = 7369 + i;
	}

	for (int i = 0; i < N; ++i) {
		rc = xTaskCreatePinnedToCore(philosopherTask, "Philosopher", 5000, &philosophers[i], 1, &philosophers[i].task, app_cpu);
		assert(rc == pdPASS);
		assert(philosophers[i].task);
	}
}

void loop() {
	Message msg;

	if (xQueueReceive(msgQueue, &msg, 1) == pdTRUE) {
		printf("%5u: Philosopher %d is %s\n", ++logNo, msg.num, stateNames[msg.state]);
	}
	delay(1);
}