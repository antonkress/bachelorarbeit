#pragma once

// #include <stdio.h>
// #include <unistd.h>
// #include <gpiod.h>
// #include <string.h>
// #include <stdlib.h>
// #include <fcntl.h>
// #include <limits.h>
// #include <wiringPi.h>
#include <signal.h>
#include <sys/wait.h>
// #include <sys/stat.h>
// #include <sys/time.h>
// #include <sys/ipc.h>
// #include <sys/types.h>
// #include <sys/msg.h>
#include <pigpio.h>

#include "config.h"
#include "light_gate.h"
#include "utils.h"
#include "sorting_control.h"
#include "color_detection.h"

int main(int argc, char *argv[]);