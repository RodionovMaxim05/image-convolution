#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define error(...) (fprintf(stderr, __VA_ARGS__))

/**
 * Represents an image split into its red, green, and blue channels.
 *
 * @param red Pointer to the red channel data.
 * @param green Pointer to the green channel data.
 * @param blue Pointer to the blue channel data.
 */
struct image_rgb {
	unsigned char *red;
	unsigned char *green;
	unsigned char *blue;
};

/**
 * Allocates memory for the red, green, and blue channels of an image with the
 * specified dimensions.
 */
struct image_rgb initialize_image_rgb(int width, int height);

/**
 * Frees the memory allocated for the red, green, and blue channels of an image.
 */
void free_image_rgb(struct image_rgb *image);

/**
 * Splits an input image into its red, green, and blue channels.
 */
void split_image_into_rgb_channels(const unsigned char *image,
								   struct image_rgb channel_image, int width,
								   int height);

/**
 * Combines the red, green, and blue channels of an image into a single image.
 */
void assemble_image_from_rgb_channels(unsigned char *image,
									  struct image_rgb channel_image, int width,
									  int height);

/**
 * Extracts the filename from a given file path.
 */
const char *extract_filename(const char *path);

/**
 * Retrieves the current time in seconds using the monotonic clock.
 */
double get_time_in_seconds(void);
