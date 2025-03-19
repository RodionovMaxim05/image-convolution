#include "utils.h"

void free_image_rgb(struct image_rgb *image) {
	free(image->red);
	free(image->green);
	free(image->blue);
}

struct image_rgb initialize_image_rgb(int width, int height) {
	struct image_rgb channel_image;

	channel_image.red = malloc(width * height);
	channel_image.green = malloc(width * height);
	channel_image.blue = malloc(width * height);

	if (channel_image.red == NULL || channel_image.green == NULL ||
		channel_image.blue == NULL) {
		free_image_rgb(&channel_image);

		struct image_rgb empty = {NULL, NULL, NULL};
		return empty;
	}

	return channel_image;
}

void split_image_into_rgb_channels(unsigned char *image,
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
