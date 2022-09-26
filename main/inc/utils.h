#pragma once
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <pigpio.h>
#include <dirent.h> // for opening directory

#include "color_detection.h"
#include "sorting_control.h"

#define ANSI_COLOR_BLACK "\x1b[30m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_WHITE "\x1b[37m"

#define ANSI_COLOR_GRAY "\x1b[90m"
#define ANSI_COLOR_BR_RED "\x1b[91m"
#define ANSI_COLOR_BR_GREEN "\x1b[92m"
#define ANSI_COLOR_BR_YELLOW "\x1b[93m"
#define ANSI_COLOR_BR_BLUE "\x1b[94m"
#define ANSI_COLOR_BR_MAGENTA "\x1b[95m"
#define ANSI_COLOR_BR_CYAN "\x1b[96m"
#define ANSI_COLOR_BR_WHITE "\x1b[97m"
#define ANSI_COLOR_RESET "\x1b[0m"

#ifndef device
#define device
typedef struct Device
{
    int mode;
    int pins[3];
} Device;
#endif

#ifndef process
#define process
typedef struct Process
{
    const char *name;
    int pid;

} Process;
#endif

#ifndef m_buffer
#define m_buffer
struct message_buffer
{
    int msg_type;
    int msg_color;
    char msg_text[12];
};

int msg_id;
#endif

// new
int create_process(Process specification);

// old

int start_environment();
int stop_environment();

int forkAndExecute(const char *path, char *const args[]);
void create_output_dir();

int printLine(const char *type,
              const char *file,
              const char *function,
              const char *infotext,
              const char *payload_string,
              const int *payload_int,
              const bool payload_bool);
