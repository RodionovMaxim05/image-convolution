#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define error(...) (fprintf(stderr, __VA_ARGS__))

struct image_rgb {
	unsigned char *red;
	unsigned char *green;
	unsigned char *blue;
};

struct image_rgb initialize_image_rgb(int width, int height);

void free_image_rgb(struct image_rgb *image);

void split_image_into_rgb_channels(const unsigned char *image,
								   struct image_rgb channel_image, int width,
								   int height);

void assemble_image_from_rgb_channels(unsigned char *image,
									  struct image_rgb channel_image, int width,
									  int height);

const char *extract_filename(const char *path);

double get_time_in_seconds(void);
