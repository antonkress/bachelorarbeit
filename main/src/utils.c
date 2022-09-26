#include "./include/utils.h"

// NEW

Device led = {
	.mode = OUTPUT,
	.pins = {PIN_LED} // DATA_IN_1
};

Device gearmotor = {
	.mode = OUTPUT,
	.pins = {PIN_GEARMOTOR} // DATA_IN_1
};

Process camera = {
	.name = "/bin/raspistill",
};

static uint64_t epochMilli;

/**
 * @brief Create a process object
 *
 * @param process_name
 * @return int pid if its the parent process
 */
int create_process(Process specification)
{
	int pid = fork();

	// create an individual process to monitor lightgate 0
	if (pid == 0)
	{
		printf("process_name: %s\n", specification.name);
		if (strcmp(specification.name, "color_detection") == 0)
		{
			initialize_color_detection();
		}
		else if (strcmp(specification.name, "sorting_control") == 0)
		{
			sorting_control_init();
		}

		exit(0);
	}
	else
	{
		return pid;
	}
}

int start_environment()
{
	printLine("L0", __FILE__, __func__, NULL, NULL, NULL, NULL);

	struct timeval tv;
	struct timezone tz;

	// set start time
	gettimeofday(&tv, &tz);
	epochMilli = (uint64_t)tv.tv_sec * (uint64_t)1000 + (uint64_t)(tv.tv_usec / 1000);

	// create_output_dir();

	// start led
	gpioSetMode(led.pins[0], led.mode);
	gpioWrite(led.pins[0], 0);

	// start camera
	char *const camera_args_init[] = {"raspistill",
									  "-w", "320",
									  "-h", "240",
									  "--quality", "5",
									  "-o", "img/raspistill/first.jpg",
									  "--nopreview",
									  "-bm",
									  NULL};
	int cpid = forkAndExecute(camera.name, camera_args_init);

	printLine("L0", __FILE__, __func__, "Initialize Camera", "Please wait...", NULL, NULL);
	sleep(3);
	kill(cpid, SIGKILL);

	char *const camera_args[] = {"raspistill",
								 "-w", "320",
								 "-h", "240",
								 "--quality", "5",
								 "-t", CAMERA_TIMEOUT,
								 "-tl", "700",
								 //  "-tl", "600",
								 "-o", "img/raspistill/second_%2d.jpg",
								 "-l", "img/raspistill/second.jpg",
								 //  "-o", "img/raspistill/second.jpg",
								 "--nopreview",
								 "-bm",
								 NULL};
	camera.pid = forkAndExecute(camera.name, camera_args);

	printf("Press Enter to Continue");
	while (getchar() != '\n')
		;

	// start gearmotor
	gpioSetMode(gearmotor.pins[0], gearmotor.mode);
	gpioWrite(gearmotor.pins[0], 1);

	// create message queue
	msg_id = msgget(IPC_PRIVATE, MESSAGE_QUEUE_PERM);

	return 0;
}

int stop_environment()
{
	printLine("L0", __FILE__, __func__, NULL, NULL, NULL, NULL);

	// stop camera
	kill(camera.pid, SIGKILL);

	// stop led
	gpioWrite(led.pins[0], 1);

	// stop gearmotor
	gpioWrite(gearmotor.pins[0], 0);

	// close message queue
	msgctl(msg_id, IPC_RMID, NULL);

	return 0;
}

/**
 * run raspistill
 */
int forkAndExecute(const char *path, char *const args[])
{
	int pid = fork();
	if (pid == -1)
	{
		fprintf(stderr,
				"fork() failed: %s",
				strerror(errno));
		return -1;
	}
	if (pid != 0)
		return pid;
	// If pid == 0, this is the child process.
	if (execv(path, args) == -1)
	{
		fprintf(stderr,
				"execvp(%s) failed: %s",
				path, strerror(errno));
		exit(-1);
	}

	return 0;
}

void create_output_dir()
{
	// get date and time for directory name
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	printf("%s", asctime(tm));

	target_dir = asctime(tm);
	printf("Name: %s\n", target_dir);

	DIR *dir = opendir("./img/output/");
	int dfd = dirfd(dir);
	errno = 0;
	mkdirat(dfd, target_dir, S_IRWXU);
}

unsigned int millis(void)
{
	struct timeval tv;
	struct timezone tz;
	uint64_t now;

	gettimeofday(&tv, &tz);

	now = (uint64_t)tv.tv_sec * (uint64_t)1000 + (uint64_t)(tv.tv_usec / 1000);

	return (uint32_t)(now - epochMilli);
}

/**
	@note return number to avoid some kind of messages
*/

int printLine(const char *type, const char *file, const char *function, const char *infotext, const char *payload_string, const int *payload_int, const bool payload_bool)
{
	char *color_infotext = ANSI_COLOR_BR_WHITE;
	char *color_payload = ANSI_COLOR_BR_WHITE;
	char *color_file = ANSI_COLOR_GRAY;
	char *color_function = ANSI_COLOR_BR_RED;

	// filter files
	if (strcmp(file, "src/main.c") == 0)
	{
		// return 0;
	}
	else if (strcmp(file, "src/utils.c") == 0)
	{
		// return 0;
	}
	else if (strcmp(file, "src/sorting_control.c") == 0)
	{
		return 0;
	}
	else if (strcmp(file, "src/color_detection.c") == 0)
	{
		// return 0;
	}
	else if (strcmp(file, "src/light_gate.c") == 0)
	{
		return 0;
	}

	// filter message type
	if (strcmp(type, "L0") == 0)
	{
		// return 0;
		color_file = ANSI_COLOR_WHITE;
		color_function = ANSI_COLOR_GREEN;
		color_infotext = ANSI_COLOR_BR_RED;
		color_payload = ANSI_COLOR_CYAN;
	}
	else if (strcmp(type, "L1") == 0)
	{
		// return 0;
		color_file = ANSI_COLOR_GRAY;
		color_function = ANSI_COLOR_GRAY;
		color_infotext = ANSI_COLOR_WHITE;
		color_payload = ANSI_COLOR_CYAN;
	}
	else if (strcmp(type, "L2") == 0)
	{
		// return 0;
		color_file = ANSI_COLOR_GRAY;
		color_function = ANSI_COLOR_GRAY;
		color_infotext = ANSI_COLOR_GRAY;
		color_payload = ANSI_COLOR_WHITE;
	}
	else if (strcmp(type, "L3") == 0)
	{
		// return 0;
		color_file = ANSI_COLOR_GRAY;
		color_function = ANSI_COLOR_GRAY;
		color_infotext = ANSI_COLOR_GRAY;
		color_payload = ANSI_COLOR_WHITE;
	}

	if (payload_string)
	{
		printf("%s%22s/%s%-20s %s%20s: %s%-20s" ANSI_COLOR_RESET "\n", color_file, file, color_function, function, color_infotext, infotext, color_payload, payload_string);
	}
	else if (payload_int)
	{
		printf("%s%22s/%s%-20s %s%20s: %s%-20i" ANSI_COLOR_RESET "\n", color_file, file, color_function, function, color_infotext, infotext, color_payload, *payload_int);
	}
	else if (payload_bool)
	{
		printf("%s%22s/%s%-20s %s%20s: %s%-20d" ANSI_COLOR_RESET "\n", color_file, file, color_function, function, color_infotext, infotext, color_payload, payload_bool);
	}
	else
	{
		printf("%s%22s/%s%-20s" ANSI_COLOR_RESET "\n", color_file, file, color_function, function);
	}

	return 0;
}
