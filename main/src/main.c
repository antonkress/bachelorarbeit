/**
  @file main Fiel
  @brief thats the main file
*/
#include "./include/main.h"

Process color_detection = {
	.name = "color_detection",
};

Process sorting_control = {
	.name = "sorting_control",
};

/**
 * @brief main function
 *
 * @param argc
 * @param argv
 * @return int 0 if succeded
 */
int main(int argc, char *argv[])
{
	printLine("L0", __FILE__, __func__, NULL, NULL, NULL, NULL);

	// main variables
	int wait_stat;

	// setup gpio pins
	if (gpioInitialise() < 0)
	{
		puts("pigpio initialisation failed.");
		return 1;
	}

	start_environment();

	// create processes for color_detection and sorting_control
	color_detection.pid = create_process(color_detection);
	sorting_control.pid = create_process(sorting_control);

	waitpid(sorting_control.pid, &wait_stat, 0);

	stop_environment();

	// close gpio's, release memory etc.
	kill(color_detection.pid, SIGKILL);

	gpioTerminate();

	return 0;
}
