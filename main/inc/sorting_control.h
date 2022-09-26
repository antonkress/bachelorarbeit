#ifndef _SORTING_CONTROL_H
#define _SORTING_CONTROL_H 1

#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <sys/msg.h>
#include <pigpio.h>

/* For error handling */
#include <errno.h>
#include <error.h>

#include "utils.h"

int timer, current_position;
unsigned long last_interrupt_time_lg2;

void sorting_control_ISR(int gpio, int level, unsigned int tick);
int sorting_control_init();

int rotate_stepmotor(int gpio_pin, int degrees);
int move_stepmotor_to(int detectedColor);

#endif