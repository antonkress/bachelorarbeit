#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <wiringPi.h>
#include <time.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/msg.h>
#include <pigpio.h>

#include "config.h"
#include "utils.h"

// NEW
int initialize_color_detection();

// OLD

struct rgbModel
{
    int r;
    int g;
    int b;
};

struct yCbCrModel
{
    unsigned int y;
    signed int cb;
    signed int cr;
};

struct colorModel
{
    int count_green;
    int count_blue;
    int count_pink;
    int count_red;
};

int histogram[COLOR_MATRIX_SIZE]; /** represents the ycbcr color model as a matrix */
int color_proportions[6];         /** the amount of detected pixels of every color */
int i_img_nr;

char *target_dir;
// ---

int detected_color;
int pin_lvl;
int ISR_counter;
int width, height, channels;
size_t rgb_image_size;
unsigned char *output_img_memory;
uint8_t *rgb_image_1;
uint8_t *rgb_image_2;
uint8_t *rgb_image_out;
unsigned char *ycbcr_image_1;
unsigned char *ycbcr_image_2;
unsigned char *ycbcr_image_out;

clock_t t;
int time_taken;

uint8_t *loadImage();
int get_object_image();
int *iterateOverPixels(uint8_t *rgb_image_1, uint8_t *rgb_image_2, size_t rgb_image_size);
void rgb_to_ycbcr(uint8_t *rgb_image, unsigned char *ycbcr_image);
void ycbcr_to_rgb(unsigned char *ycbcr_image, unsigned char *rgb_image);
void difference_imaging(unsigned char *ycbcr_1, unsigned char *ycbcr_2, unsigned char *ycbcr_out);
int clusterColorModel_new(struct yCbCrModel yCbCr_output, int *histogram);
int fill_histogram(unsigned char *ycbcr_image, int *histogram);
int *get_color_proportions(int *histogram);
// char* detect_color(int* colorValues);
int detect_color(int *colorValues);
void printMatrix(int *colorValues);
void write_image(int detectedColor);
void generate_output(int *colorValues, int detectedColor);
// char *color_detection_ISR();
// int color_detection_ISR(uint8_t *rgb_image);
void color_detection_ISR(int gpio, int level, unsigned int tick);
