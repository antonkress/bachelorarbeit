#pragma once

#define OBJ_CNTR_ARRAY_SIZE 10 // size of color queue
#define WAIT_TO_START 2        // time before starting main loop (sec)
#define WAIT_TO_STOP 15        // 20        // time before ending main loop (sec) / gearmotor is deactivated
#define MS_TO_WAIT 500

#define FIFO_IN "/tmp/in.fifo"
#define FIFO_OUT "/tmp/out.fifo"
#define FIFO_COLOR_DETECTION "/tmp/color_detection.fifo"
#define FIFO_SORTING_CONTROL "/tmp/sorting_control.fifo"

#define PATH_IMAGE_1 "img/raspistill/first.jpg"
// #define PATH_IMAGE_2 "img/raspistill/first_02.jpg"
// #define PATH_IMAGE_22 "img/raspistill/first_22.jpg"
// #define PATH_IMAGE_2 "img/raspistill/test_braun.jpg"
#define PATH_IMAGE_2 "img/raspistill/second.jpg"
#define PATH_IMAGE_OUTPUT "img/output/"
#define DETECTION_THRESHOLD 7
#define COUNTED_PIXELS_THRESHOLD 1500
#define NUMBER_OF_COLORS 6
#define COLOR_FIELDS_SIZE 6
#define COLOR_MATRIX_SIZE 100

#define PIN_LIGHTGATE_IN 17   // gpioset: 0
#define PIN_LIGHTGATE_OUT 27  // gpioset: 2
#define PIN_GEARMOTOR 5       // gpioset: 21
#define PIN_LED 0             // gpioset: 22
#define PIN_STEPMOTOR_EN 13   // gpioset: 13
#define PIN_STEPMOTOR_STEP 19 // gpioset: 19
#define PIN_STEPMOTOR_DIR 26  // gpioset: 26

#define MESSAGE_QUEUE_PERM 0600

#define INTERRUPT_TIMEOUT 9999999
#define CAMERA_TIMEOUT "9999999"

#define ISR_DEBOUNCE_TIME 400

#define MOTORSPEED 3000 // = 3 ms; should not be smaller than 2000.
#define MOTOR_ANGLE 12
#define RESET_POSITION 0