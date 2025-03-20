#include <stdbool.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

#include "filters/filter.h"
#include "sequential_convolution/filter_application.h"
#include "utils/utils.h"

int main(int argc, char *argv[]) {
	if (argc < 3) {
		error("Usage: %s <image_path | --default-image> <filter_name>\n", argv[0]);
		return -1;
	}

	const char *image_path =
		strcmp(argv[1], "--default-image") == 0 ? "../images/cat.bmp" : argv[1];

	int width, height, channels;
	unsigned char *image = stbi_load(image_path, &width, &height, &channels, 3);
	if (!image) {
		error("Could not open or find the image!\n");
		return -1;
	}

	const char *file_name = extract_filename(image_path);

	const char *filter_name = argv[2];

	struct image_rgb channel_image = initialize_image_rgb(width, height);
	if (channel_image.red == NULL || channel_image.green == NULL ||
		channel_image.blue == NULL) {
		error("Memory allocation error.\n");
		stbi_image_free(image);
		return -1;
	}
	split_image_into_rgb_channels(image, channel_image, width, height);

	struct image_rgb result_channel_image = initialize_image_rgb(width, height);
	if (result_channel_image.red == NULL || result_channel_image.green == NULL ||
		result_channel_image.blue == NULL) {
		error("Memory allocation error.\n");
		stbi_image_free(image);
		free_image_rgb(&channel_image);
		return -1;
	}

	struct filter image_filter;
	if (strcmp(filter_name, "id") == 0) {
		image_filter = create_filter(3, 1.0, 0.0, id);
	} else if (strcmp(filter_name, "fbl") == 0) {
		image_filter = create_filter(3, 1.0, 0.0, fast_blur);
	} else if (strcmp(filter_name, "bl") == 0) {
		image_filter = create_filter(5, 1.0 / 13.0, 0.0, blur);
	} else if (strcmp(filter_name, "gb") == 0) {
		image_filter = create_filter(5, 1.0 / 256.0, 0.0, gaus_blur);
	} else if (strcmp(filter_name, "mbl") == 0) {
		image_filter = create_filter(9, 1.0 / 9.0, 0.0, motion_blur);
	} else if (strcmp(filter_name, "ed") == 0) {
		image_filter = create_filter(3, 1.0, 0.0, edge_detection);
	} else if (strcmp(filter_name, "em") == 0) {
		image_filter = create_filter(5, 1.0, 128.0, emboss);
	} else {
		error("Unknown filter name: %s\n", filter_name);
		image_filter.kernel = NULL;
	}

	if (image_filter.kernel == NULL) {
		error("Memory allocation error.\n");
		return -1;
	}

	apply_filter(channel_image, result_channel_image, width, height, image_filter);

	unsigned char *result_image = malloc(width * height * channels);
	assemble_image_from_rgb_channels(result_image, result_channel_image, width,
									 height);

	char *output_file_path =
		malloc(12 + strlen(file_name) + strlen(filter_name) + 1);
	sprintf(output_file_path, "../images/%s_%s", filter_name, file_name);
	stbi_write_bmp(output_file_path, width, height, 3, result_image);

	free(output_file_path);
	stbi_image_free(image);
	free_filter(&image_filter);
	free_image_rgb(&channel_image);
	free_image_rgb(&result_channel_image);

	return 0;
}
