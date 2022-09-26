#include "./include/color_detection.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

unsigned long last_interrupt_time_lg1 = 0;
static uint64_t epochMilli;
int object_counter = 0;

// NEW
Device lightgate_in = {
    .mode = INPUT,
    .pins = {PIN_LIGHTGATE_IN}, // DATA_IN_1
};

int color_masks[NUMBER_OF_COLORS][COLOR_FIELDS_SIZE] = {
    {44, 45, 54, 64},         // green
    {44, 45, 55, 56, 66, 67}, // blue
    {24, 34, 44, 45},         // red
    {23, 33, 34, 44, 45},     // orange
    {32, 33, 42, 43, 44, 45}, // yellow
    {44, 45}                  // brown
};                            /** shows which field of the histogram belongs to which color */
float color_multiplier[NUMBER_OF_COLORS][COLOR_FIELDS_SIZE] = {
    {1.9, 0.4, 6.2, 3.6},           // green
    {1, 0.7, 0.8, 1.7, 1.3, 5.1},   // blue
    {7, 2.4, 2.4, 0.3},             // red
    {2, 4.7, 1.4, 2.5, 0.3},        // orange
    {1.9, 1.3, 2.8, 3.3, 3.3, 0.3}, // yellow
    {6.1, 0.4}                      // brown
};                                  /** declares the weight of the fields */

char *color_names[7] = {"green", "blue", "red", "orange", "yellow", "brown", "idle"}; /** the names of the colors */

/**
 * @brief initialize color_detection process
 *
 * @param color_detection
 * @return int
 */
int initialize_color_detection(Device lightgate)
{
    printLine("L0", __FILE__, __func__, NULL, NULL, NULL, NULL);

    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    epochMilli = (uint64_t)tv.tv_sec * (uint64_t)1000 + (uint64_t)(tv.tv_usec / 1000);

    i_img_nr = 0;                                                         // init. image number for write image output
    rgb_image_1 = stbi_load(PATH_IMAGE_1, &width, &height, &channels, 0); // load image 1
    printf("RGB image 1 pixel(0,0) = %i/%i/%i\n", *rgb_image_1, *(rgb_image_1 + 1), *(rgb_image_1 + 2));

    rgb_image_size = width * height * channels;

    ycbcr_image_1 = malloc(rgb_image_size);
    ycbcr_image_2 = malloc(rgb_image_size);
    ycbcr_image_out = malloc(rgb_image_size);
    rgb_image_out = malloc(rgb_image_size);
    // allocate memory for new image
    output_img_memory = malloc(rgb_image_size);
    if (output_img_memory == NULL)
    {
        perror("Error allocating output image");
    }

    rgb_to_ycbcr(rgb_image_1, ycbcr_image_1);
    printf("YCBCR image 1 pixel(0,0) = %i/%i/%i\n", *ycbcr_image_1, *(ycbcr_image_1 + 1), *(ycbcr_image_1 + 2));

    ISR_counter = 0;

    gpioSetISRFunc(lightgate_in.pins[0], RISING_EDGE, INTERRUPT_TIMEOUT, color_detection_ISR);

    while (1)
    {
        // printLine("L1", __FILE__, __func__, "Wait for Interrupt", NULL, NULL, NULL);
        sleep(1);
        ISR_counter = 0;
    }

    return 0;
}

/** METHODS */

/**
 * @brief setup for color detection
 * @return 0, if succeded
 */
int get_object_image()
{
    // t = clock();
    printLine("L3", __FILE__, __func__, NULL, NULL, NULL, NULL);

    // reading image
    rgb_image_1 = stbi_load(PATH_IMAGE_1, &width, &height, &channels, 0);
    rgb_image_2 = stbi_load(PATH_IMAGE_2, &width, &height, &channels, 0);

    if ((rgb_image_1 == NULL) || (rgb_image_2 == NULL))
    {
        perror("Image not found");
        return (-1);
    }
    // printLine("L2", __FILE__, __func__, "Image loaded", PATH_IMAGE_1, NULL, NULL);
    // printLine("L2", __FILE__, __func__, "Image loaded", PATH_IMAGE_2, NULL, NULL);

    // get the size of the image
    // rgb_image_size = width * height * channels;

    // t = clock() - t;
    // time_taken = (int)(1000000 * (((double)t) / CLOCKS_PER_SEC)); // in seconds
    // printLine("L2", __FILE__, __func__, "time taken (us)", NULL, &time_taken, NULL);

    return 0;
}

/**
 * @brief transform pixel from RGB to YCbCr.
 * @param p pixel with rgb-values.
 * @return pixel with yCbCr-values.
 */
void rgb_to_ycbcr(uint8_t *rgb_image, unsigned char *ycbcr_image)
{
    printLine("L0", __FILE__, __func__, NULL, NULL, NULL, NULL);
    struct rgbModel rgb;
    struct yCbCrModel yCbCr;

    for (unsigned char *p_in = rgb_image, *p_out = ycbcr_image;
         p_in != rgb_image + rgb_image_size;
         p_in += channels, p_out += channels)
    {
        // transform images to yCbCr
        rgb.r = *(p_in + 0);
        rgb.g = *(p_in + 1);
        rgb.b = *(p_in + 2);

        yCbCr.y = (16 + (65.481 * rgb.r) / 255 + (128.553 * rgb.g) / 255 + (24.966 * rgb.b) / 255);
        yCbCr.cb = 128 - (37.797 * rgb.r) / 255 - (74.203 * rgb.g) / 255 + (112.0 * rgb.b) / 255;
        yCbCr.cr = 128 + (112.0 * rgb.r) / 255 - (93.786 * rgb.g) / 255 - (18.214 * rgb.b) / 255;

        *(p_out + 0) = yCbCr.y;
        *(p_out + 1) = yCbCr.cb;
        *(p_out + 2) = yCbCr.cr;
    }

    // return yCbCr_image;
}

/**
 * @brief transform pixel from YCbCr to RGB and write new pixels
 * @param yCbCr pixel with yCbCr-values.
 * @return pixel with rgb-values.
 */
void ycbcr_to_rgb(unsigned char *ycbcr_image, unsigned char *rgb_image)
{
    printLine("L0", __FILE__, __func__, NULL, NULL, NULL, NULL);
    struct yCbCrModel yCbCr;
    struct rgbModel rgb;

    for (unsigned char *p_in = ycbcr_image, *p_out = rgb_image;
         p_in != ycbcr_image + rgb_image_size;
         p_in += channels, p_out += channels)
    {
        // transform images to yCbCr
        yCbCr.y = *(p_in + 0);
        yCbCr.cb = *(p_in + 1);
        yCbCr.cr = *(p_in + 2);

        rgb.r = (255 / 219) * (yCbCr.y - 16) + (255 / 224) * 1.402 * (yCbCr.cr - 128);
        rgb.g = (255 / 219) * (yCbCr.y - 16) - (255 / 224) * 1.772 * (0.114 / 0.587) * (yCbCr.cb - 128) - (255 / 224) * 1.402 * (0.299 / 0.587) * (yCbCr.cr - 128);
        rgb.b = (255 / 219) * (yCbCr.y - 16) + (255 / 224) * 1.772 * (yCbCr.cb - 128);

        *(p_out + 0) = rgb.r;
        *(p_out + 1) = rgb.g;
        *(p_out + 2) = rgb.b;
    }
}

/**
 * @brief subtract two pixel to seperate the object from background.
 * @param yCbCr_1 pixel 1 with yCbCr-values.
 * @param yCbCr_2 pixel 2 with yCbCr-values.
 * @return difference between two pixels.
 */
void difference_imaging(unsigned char *ycbcr_1, unsigned char *ycbcr_2, unsigned char *ycbcr_out)
{
    printLine("L0", __FILE__, __func__, NULL, NULL, NULL, NULL);
    struct yCbCrModel ycbcr_model_1, ycbcr_model_2, ycbcr_model_out;

    for (unsigned char *p_1 = ycbcr_1, *p_2 = ycbcr_2, *p_out = ycbcr_out;
         p_1 != ycbcr_1 + rgb_image_size;
         p_1 += channels, p_2 += channels, p_out += channels)
    {
        ycbcr_model_1.y = *(p_1 + 0);
        ycbcr_model_1.cb = *(p_1 + 1);
        ycbcr_model_1.cr = *(p_1 + 2);

        ycbcr_model_2.y = *(p_2 + 0);
        ycbcr_model_2.cb = *(p_2 + 1);
        ycbcr_model_2.cr = *(p_2 + 2);

        // > implement diff
        // int diff = abs(ycbcr_model_1.cb) - abs(ycbcr_model_2.cb);

        // if the images do not differ: y = 0, cb = 0, cr = 0
        if ((ycbcr_model_2.cb - ycbcr_model_1.cb > DETECTION_THRESHOLD))
        {
            ycbcr_model_out.y = ycbcr_model_2.y;
            ycbcr_model_out.cb = ycbcr_model_2.cb;
            ycbcr_model_out.cr = ycbcr_model_2.cr;
        }
        // detect blue, because its cb-value is bigger than gray
        else if ((ycbcr_model_1.cb - ycbcr_model_2.cb > DETECTION_THRESHOLD))
        {
            ycbcr_model_out.y = ycbcr_model_2.y;
            ycbcr_model_out.cb = ycbcr_model_2.cb;
            ycbcr_model_out.cr = ycbcr_model_2.cr;
        }
        else
        {
            ycbcr_model_out.y = 0;
            ycbcr_model_out.cb = 0;
            ycbcr_model_out.cr = 0;
        }

        *(p_out + 0) = ycbcr_model_out.y;
        *(p_out + 1) = ycbcr_model_out.cb;
        *(p_out + 2) = ycbcr_model_out.cr;
    }

    // return yCbCr_output;
}

/**
 * @brief Get the clustered values for the thresholds of one color
 * @note (yCbCr_output.y != 0) means if background is not black
 * @note Generates a grid of the YCbCr-scale
 * @param yCbCr_output subtracted pixel with yCbCr-values.
 * @param colorsValues representation of the colors as a struct of integers.
 * @return 0 if succeded
 */
int fill_histogram(unsigned char *ycbcr_image, int *histogram)
{
    printLine("L0", __FILE__, __func__, NULL, NULL, NULL, NULL);
    struct yCbCrModel yCbCr;

    for (unsigned char *p = ycbcr_image;
         p != ycbcr_image + rgb_image_size;
         p += channels)
    {
        yCbCr.y = *(p + 0);
        yCbCr.cb = *(p + 1);
        yCbCr.cr = *(p + 2);

        if (yCbCr.y != 0)
        {
            int value_x = yCbCr.cb / 25;
            int value_y = ((255 - yCbCr.cr) / 25);
            int field_index = value_y * 10 + value_x;
            histogram[field_index]++;
        }
    }
    return 0;
}

/**
 * @brief Sums up the values for all identified colors together
 * This is how to recognize how many pixels of which color are present
 * @param histogram is an array containing all values of the matrix
 * @return color_proportions, an array containing the summed values of all colors
 */
int *get_color_proportions(int *histogram)
{
    t = clock();
    printLine("L3", __FILE__, __func__, NULL, NULL, NULL, NULL);

    // iterate over all 100 fields of the grid
    for (int histogram_index = 0; histogram_index < COLOR_MATRIX_SIZE; histogram_index++)
    {
        // continue only if values are entered in the current field
        if (*(histogram + histogram_index) > 0)
        {
            // printf("value of current histogram field(%i): %i\n", histogram_index, *(histogram + histogram_index));
            // iterate over all 6 colors
            for (int color_index = 0; color_index < NUMBER_OF_COLORS; color_index++)
            {
                // iterate over each detection field of the current color
                for (int field_index = 0; field_index < COLOR_FIELDS_SIZE; field_index++)
                {
                    // break, if a color has less than 4 detection fields
                    // to avoid segmentation faults
                    if (color_masks[color_index][field_index] == 0)
                    {
                        // printf("break\n");
                        break;
                    }

                    // if a detection field of a color matches the current field add the value of the field to the color_proportions
                    if (color_masks[color_index][field_index] == histogram_index)
                    {
                        color_proportions[color_index] += (*(histogram + histogram_index) * color_multiplier[color_index][field_index]);
                    }
                }
            }
        }
    }

    t = clock() - t;
    time_taken = (int)(1000000 * (((double)t) / CLOCKS_PER_SEC)); // in seconds
    // printLine("L2", __FILE__, __func__, "time taken (us)", NULL, &time_taken, NULL);

    return &color_proportions[0];
}

/**
 * @brief searches for the highest color value and returns the corresponding name of this color
 * @param color_proportions is an array containing the summed values of all colors
 * @return the name of the color with the highest value
 */
// char *detect_color(int *color_proportions)
int detect_color(int *color_proportions)
{
    t = clock();
    printLine("L3", __FILE__, __func__, NULL, NULL, NULL, NULL);

    // initialize max-value and color-index to the values of the first color
    // int highest_proportion_value = *color_proportions;
    int highest_proportion_value = 0;
    int highest_proportion_index = 6;

    // iterate over all 6 colors
    for (int current_proportion_index = 0; current_proportion_index < NUMBER_OF_COLORS; current_proportion_index++)
    {
        // determine the value of the current color
        int current_proportion_value = *(color_proportions + current_proportion_index);

        // if value of current color exceeds current max value
        if (current_proportion_value > highest_proportion_value)
        {
            highest_proportion_value = current_proportion_value;
            highest_proportion_index = current_proportion_index;
        }
    }

    // Detected color is only valid if amount of counted pixels reaches minimum
    // threshold of pixels; otherwise return "idle"
    if (*(color_proportions + highest_proportion_index) < COUNTED_PIXELS_THRESHOLD)
    {
        highest_proportion_index = 6;
    }

    // print values of each color for debugging
    printLine("L0", __FILE__, __func__, "Color Proportions", "", NULL, NULL);
    for (int i = 0; i < 6; i++)
    {
        int value = *(color_proportions + i);
        printLine("L1", __FILE__, __func__, color_names[i], NULL, &value, NULL);
    }

    printLine("L1", __FILE__, __func__, "Detected color", color_names[highest_proportion_index], NULL, NULL);
    printLine("L2", __FILE__, __func__, "Pixels counted", NULL, &color_proportions[highest_proportion_index], NULL);

    // return color_names[highest_proportion_index];

    t = clock() - t;
    time_taken = (int)(1000000 * (((double)t) / CLOCKS_PER_SEC)); // in seconds
    // printLine("L2", __FILE__, __func__, "time taken (us)", NULL, &time_taken, NULL);

    return highest_proportion_index;
}

/**
 * @brief generates new image and prints matrix
 * saves the separated object as a new image and prints the matrix with values
 * @param color_proportions is an array containing the summed values of all colors
 * @return void
 */
void generate_output(int *colorValues, int detected_color)
{
    t = clock();
    printLine("L3", __FILE__, __func__, NULL, NULL, NULL, NULL);

    write_image(detected_color);
    printMatrix(colorValues);

    t = clock() - t;
    time_taken = (int)(1000000 * (((double)t) / CLOCKS_PER_SEC)); // in seconds
    printLine("L2", __FILE__, __func__, "time taken (us)", NULL, &time_taken, NULL);
}

void write_image(int detected_color)
{
    t = clock();
    printLine("L3", __FILE__, __func__, NULL, NULL, NULL, NULL);
    char output_image[30] = PATH_IMAGE_OUTPUT;
    // char img_name[8] = "output_";
    char s_img_nr[2];
    char file_suffix[5] = ".jpg";

    sprintf(s_img_nr, "%d", i_img_nr);
    // sprintf(img_name, "%d", detected_color);

    strcat(output_image, s_img_nr);
    strcat(output_image, color_names[detected_color]);
    strcat(output_image, file_suffix);

    // stbi_write_jpg(PATH_IMAGE_OUTPUT, width, height, channels, output_img_memory, 100);
    stbi_write_jpg(output_image, width, height, channels, rgb_image_out, 100);
    printLine("L2", __FILE__, __func__, "New image saved", output_image, NULL, NULL);

    i_img_nr++;

    t = clock() - t;
    time_taken = (int)(1000000 * (((double)t) / CLOCKS_PER_SEC)); // in seconds
    printLine("L2", __FILE__, __func__, "time taken (us)", NULL, &time_taken, NULL);
}

/**
 * @brief outputs the entire matrix
 * in the matrix it can be seen which areas of the ycbcr color space are particularly strongly represented
 * @param color_proportions is an array containing the summed values of all colors
 * @return void
 */
void printMatrix(int *colorValues)
{
    t = clock();
    printLine("L3", __FILE__, __func__, NULL, NULL, NULL, NULL);

    char *color_type;

    printLine("L0", __FILE__, __func__, "Histogram of object", "", NULL, NULL);
    for (int k = 0; k < COLOR_MATRIX_SIZE; k++)
    {
        // print all zeros in gray and values in white text color
        color_type = (*(colorValues + k) == 0) ? "\x1b[90m" : "\x1b[37m";

        printf("%s%i\t", color_type, *(colorValues + k));
        if ((k + 1) % (int)sqrt(COLOR_MATRIX_SIZE) == 0)
            printf("\n");
    }
    printf("%s", "\x1b[37m");

    t = clock() - t;
    time_taken = (int)(1000000 * (((double)t) / CLOCKS_PER_SEC)); // in seconds
    printLine("L2", __FILE__, __func__, "time taken (us)", NULL, &time_taken, NULL);
}

void color_detection_ISR(int gpio, int level, unsigned int tick)
{
    struct message_buffer m;
    m.msg_type = 1;

    unsigned long interrupt_time = millis();
    signed int diff = interrupt_time - last_interrupt_time_lg1;
    // printf("diff COLOR_de: %li - %li = %i\n", interrupt_time, last_interrupt_time_lg1, diff);

    // if (diff > ISR_DEBOUNCE_TIME && last_interrupt_time_lg1 != 0)
    if (diff > ISR_DEBOUNCE_TIME)
    {
        // printf("diff COLOR_de: %li - %li = %i\n", interrupt_time, last_interrupt_time_lg1, diff);
        // printLine("L0", __FILE__, __func__, "inside ", "colordet", NULL, NULL);
        object_counter++;
        printf("object counter = %i\n", object_counter);

        // look for objects in the next seconds
        // man könnte hier eine while (t_now - t_before < t_Aufnahme) machen, damit auf Zeit kontrolliert wird
        for (size_t i = 0; i < 10; i++)
        {
            clock_t dt_Cd = clock();

            // reset arrays
            memset(color_proportions, 0, sizeof(color_proportions));
            memset(histogram, 0, sizeof(histogram));
            memset(rgb_image_out, 0, sizeof(*rgb_image_out));

            // reset msg_text
            if (memset(m.msg_text, 0, sizeof(m.msg_text)) < 0)
            {
                perror("memset 1");
                printLine("L0", __FILE__, __func__, "ERROR!", "NULL", NULL, NULL);

                exit(1);
            }

            // do the setup
            get_object_image();

            rgb_to_ycbcr(rgb_image_2, ycbcr_image_2);

            difference_imaging(ycbcr_image_1, ycbcr_image_2, ycbcr_image_out);

            ycbcr_to_rgb(ycbcr_image_out, rgb_image_out);

            fill_histogram(ycbcr_image_out, histogram);

            get_color_proportions(histogram);

            detected_color = detect_color(color_proportions);

            stbi_image_free(rgb_image_1);
            stbi_image_free(rgb_image_2);

            // >>
            generate_output(histogram, detected_color);
            // <<

            // if (strcmp(detected_color, "idle") != 0)
            if (detected_color != 6)
            {
                // generate_output(histogram, detected_color);

                // preparation for sending message
                m.msg_color = detected_color;
                size_t length = sizeof(m.msg_color);

                // send message
                if (msgsnd(msg_id, &m, length, 0) == -1)
                {
                    perror("msgsnd 1");
                    printLine("L0", __FILE__, __func__, "ERROR!", "NULL", NULL, NULL);

                    exit(1);
                }
                printLine("L0", __FILE__, __func__, "Color", NULL, &m.msg_color, NULL);

                dt_Cd = clock() - dt_Cd;
                time_taken = (int)(1000000 * (((double)dt_Cd) / CLOCKS_PER_SEC)); // in seconds
                printLine("L2", __FILE__, __func__, "dt_Cd: time taken (us)", NULL, &time_taken, NULL);

                break;
            }
        }
    }
    last_interrupt_time_lg1 = interrupt_time;
}
