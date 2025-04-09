#include "utils.h"

#define NANOSECONDS_IN_SECOND 1e9

struct image_rgb initialize_image_rgb(int width, int height) {
	struct image_rgb channel_image;

	channel_image.red = malloc((size_t)width * (size_t)height);
	channel_image.green = malloc((size_t)width * (size_t)height);
	channel_image.blue = malloc((size_t)width * (size_t)height);

	if (channel_image.red == NULL || channel_image.green == NULL ||
		channel_image.blue == NULL) {
		free_image_rgb(&channel_image);

		struct image_rgb empty = {NULL, NULL, NULL};
		return empty;
	}

	return channel_image;
}

void free_image_rgb(struct image_rgb *image) {
	free(image->red);
	free(image->green);
	free(image->blue);
}

void split_image_into_rgb_channels(const unsigned char *image,
								   struct image_rgb channel_image, int width,
								   int height) {

	for (int i = 0; i < width * height; i++) {
		channel_image.red[i] = image[i * 3 + 0];
		channel_image.green[i] = image[i * 3 + 1];
		channel_image.blue[i] = image[i * 3 + 2];
	}
}

void assemble_image_from_rgb_channels(unsigned char *image,
									  struct image_rgb channel_image, int width,
									  int height) {

	for (int i = 0; i < width * height; i++) {
		image[i * 3 + 0] = channel_image.red[i];
		image[i * 3 + 1] = channel_image.green[i];
		image[i * 3 + 2] = channel_image.blue[i];
	}
}

const char *extract_filename(const char *path) {
	const char *last_slash = strrchr(path, '/');
	if (last_slash == NULL) {
		return path;
	}

	return last_slash + 1;
}

double get_time_in_seconds(void) {
	struct timespec time;
	if (clock_gettime(CLOCK_MONOTONIC, &time) != 0) {
		return -1;
	}

	return (double)time.tv_sec + (double)time.tv_nsec / NANOSECONDS_IN_SECOND;
}
