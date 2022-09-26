#include "./include/sorting_control.h"

Device lightgate_out = {
	.mode = INPUT,
	.pins = {PIN_LIGHTGATE_OUT}, // DATA_IN_1
};

Device stepmotor = {
	.mode = OUTPUT,
	.pins = {
		PIN_STEPMOTOR_EN,
		PIN_STEPMOTOR_STEP,
		PIN_STEPMOTOR_DIR} // ENABLE, STEP, DIRECTION
};

/**
 * @brief ISR; get detected color and move stepmotor
 * To avoid multiple calls of this ISR a software
 * debouncing was implemented.
 * @param gpio Pin triggered
 * @param level Level of triggered pin (high or low)
 * @param tick number of ticks
 * @return ** void
 */
void sorting_control_ISR(int gpio, int level, unsigned int tick)
{
	// software debouncing with current time
	unsigned long interrupt_time = millis();
	signed int diff = interrupt_time - last_interrupt_time_lg2;

	printLine("L0", __FILE__, __func__, "ISR", "S...", NULL, NULL);
	// only become active if there was enough time between
	// the last call of the ISR and now &&
	// if it isn't the first call (at starting time)
	// if (diff > ISR_DEBOUNCE_TIME && last_interrupt_time_lg2 != 0)
	if (diff > ISR_DEBOUNCE_TIME)
	{
		printLine("L0", __FILE__, __func__, "ISR", "START", NULL, NULL);
		// reset timer to starting value
		timer = WAIT_TO_STOP;

		// init. message buffer for recieving from message queue
		struct message_buffer m;
		size_t size = sizeof(m.msg_color);

		// get detected color from message queue
		if (msgrcv(msg_id, &m, size, 0, 0) < 0)
		{
			perror("msgrcv_");
			printLine("L0", __FILE__, __func__, "ERROR!", "NULL", NULL, NULL);
		}

		// move stepmotor to detected color
		move_stepmotor_to(m.msg_color);

		printLine("L0", __FILE__, __func__, "ISR", "STOP", NULL, NULL);
	}

	// set time of the last call of ISR for software debouncing
	last_interrupt_time_lg2 = interrupt_time;
}

/**
 * @brief Init. GPIO Pins and wait for interrupts
 * Terminates if timer reaches 0
 * @return int
 */
int sorting_control_init()
{
	printLine("L0", __FILE__, __func__, NULL, NULL, NULL, NULL);

	// init. global variables
	timer = WAIT_TO_STOP;
	current_position = RESET_POSITION;
	last_interrupt_time_lg2 = 0;

	// set pins of stepmotor as OUTPUT
	gpioSetMode(stepmotor.pins[0], stepmotor.mode);
	gpioSetMode(stepmotor.pins[1], stepmotor.mode);
	gpioSetMode(stepmotor.pins[2], stepmotor.mode);

	// enable motor (low-active)
	gpioWrite(stepmotor.pins[0], 0);

	// wait for interrupt
	gpioSetISRFunc(lightgate_out.pins[0], RISING_EDGE, INTERRUPT_TIMEOUT, sorting_control_ISR);

	do
	{
		// start decrementing timer after first object
		timer--;
		// printLine("L0", __FILE__, __func__, "Timer", NULL, &timer, NULL);

		sleep(1);
	} while (timer > 0);

	// move stepmotor to reset position
	move_stepmotor_to(RESET_POSITION);
	if (gpioWrite(stepmotor.pins[0], 1) < 0) // disable motor (low-active)
	{
		printLine("L0", __FILE__, __func__, "ERROR!", "NULL", NULL, NULL);
		perror("gpioWrite");
		exit(1);
	}

	printLine("L0", __FILE__, __func__, "STOP PROGRAM", "NULL", NULL, NULL);
	return 0;
}

/**
 * @brief Rotate the motor with degrees steps.
 * @param degrees Number of steps the motor should do.
 * @param MOTORSPEED Time in µs between two signals
 * @return 0 if the direction-pin was properly set, -1 on failure.
 */
int rotate_stepmotor(int gpio_pin, int degrees)
{
	printLine("L0", __FILE__, __func__, "Measure time for number of steps", NULL, &degrees, NULL);

	// get timestamp 1 for measuring the processing time
	struct timeval now;
	gettimeofday(&now, NULL);
	t = now.tv_usec;

	// toggle STEP-pin of the stepmotor driver to move the motor
	for (int step = 0; step < degrees; step++)
	{
		if (gpioWrite(gpio_pin, 1) < 0)
		{
			perror("gpioWrite");
			exit(1);
		}
		usleep(MOTORSPEED);
		if (gpioWrite(gpio_pin, 0) < 0)
		{
			perror("gpioWrite");
			exit(1);
		}
		usleep(MOTORSPEED);
	}

	// get timestamp 2 for measuring the processing time
	gettimeofday(&now, NULL);
	t = now.tv_usec - t;

	// print out measurements
	// printLine("L1", __FILE__, __func__, "t(us) for 1 step is", NULL, (const int *)&t, NULL);
	// printLine("L0", __FILE__, __func__, "Measure Time", "Finished", NULL, NULL);
	// printLine("L1", __FILE__, __func__, "step", "STOP", NULL, NULL);
	// printLine("L2", __FILE__, __func__, "Motor degrees", NULL, &degrees, NULL);

	return 0;
}

/**
 * @brief handle motor movement
 * set direction of the movement, move motor and save the last position for the next call
 * @param detected_color Which position the stepmotor should move to
 * @return int 0 if succeded
 */
int move_stepmotor_to(int detected_color)
{
	printLine("L1", __FILE__, __func__, "STEP TO", "START", NULL, NULL);
	int next_position = detected_color;

	// calculate the number of positions
	float number_of_positions = next_position - current_position;

	// calculate degrees and direction
	int degrees = abs(MOTOR_ANGLE * number_of_positions);
	int direction = (number_of_positions < 0) ? 1 : 0;

	// set direction
	if (gpioWrite(stepmotor.pins[2], direction) < 0)
	{
		perror("gpioWrite");
		exit(1);
	}

	// move stepmotor
	rotate_stepmotor(stepmotor.pins[1], degrees); // make step

	// set current position for next call
	current_position = next_position;

	printLine("L1", __FILE__, __func__, "STEP TO", "STOP", NULL, NULL);

	return 0;
}