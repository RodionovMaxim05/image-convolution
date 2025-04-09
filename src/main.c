#include "convolution/parallel_dispatch.h"
#include "filters/filter.h"
#include "utils/utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define MODE_PREFIX_LEN 7
#define THREAD_PREFIX_LEN 9
#define PATH_PREFIX_LENGTH 7	 // Length "images/"
#define UNDERSCORE_COUNT 2		 // Number of underscores
#define NULL_TERMINATOR_LENGTH 1 // Terminating null character '\0'

static inline bool handle_error(bool condition, const char *message, ...) {
	if (condition) {
		va_list args;
		va_start(args, message);
		error(message, args);
		va_end(args);
		return true;
	}
	return false;
}

int main(int argc, char *argv[]) {
	if (argc < 4) {
		error("Usage:\n"
			  "  %s <image_path | --default-image> <filter_name> --mode=<mode> "
			  "--thread=<num>\n"
			  "  %s <image_path | --default-image> <filter_name> --mode=seq\n\n"
			  "Options:\n"
			  "  <image_path>           Path to the input image file.\n"
			  "  --default-image        Use a predefined default image.\n"
			  "  <filter_name>          Name of the filter to apply ().\n"
			  "  --mode=<mode>          Execution mode: 'seq' for sequential or "
			  "'row', 'column', 'block', 'pixel' for parallel.\n"
			  "  --thread=<num>         Number of threads to use in parallel mode "
			  "(ignored if --mode=seq).\n\n"
			  "Available Filters:\n",
			  argv[0], argv[0]);
		for (int i = 0; i < NUM_OF_FILTERS; i++) {
			error("  %-22s %s\n", filters_info[i].name, filters_info[i].description);
		}
		return -1;
	}

	int width, height, channels;
	unsigned char *image = NULL;
	struct image_rgb channel_image = {NULL, NULL, NULL};
	struct image_rgb result_channel_image = {NULL, NULL, NULL};
	struct filter image_filter = {0, 0, 0, NULL};
	unsigned char *result_image = NULL;
	char *output_file_path = NULL;

	const char *image_path =
		strcmp(argv[1], "--default-image") == 0 ? "images/cat.bmp" : argv[1];

	image = stbi_load(image_path, &width, &height, &channels, 3);
	if (handle_error(!image, "Could not open or find the image!\n")) {
		goto cleanup_and_err;
	}

	const char *file_name = extract_filename(image_path);
	const char *filter_name = argv[2];

	char *mode_str = NULL;
	int threads_num = 0;
	if (strncmp(argv[3], "--mode=", MODE_PREFIX_LEN) == 0) {
		mode_str = argv[3] + MODE_PREFIX_LEN;

		if (strcmp(mode_str, "seq") != 0) {
			if (handle_error(strncmp(argv[4], "--thread=", THREAD_PREFIX_LEN) != 0,
							 "Missing --thread argument\n")) {
				goto cleanup_and_err;
			};
			threads_num = atoi(argv[4] + THREAD_PREFIX_LEN);
		}
	} else {
		if (handle_error(1, "Missing --mode argument\n")) {
			goto cleanup_and_err;
		}
	}

	channel_image = initialize_image_rgb(width, height);
	if (handle_error(channel_image.red == NULL || channel_image.green == NULL ||
						 channel_image.blue == NULL,
					 "Memory allocation error for channel_image.\n")) {
		goto cleanup_and_err;
	}

	split_image_into_rgb_channels(image, channel_image, width, height);

	if (strcmp(filter_name, "id") == 0) {
		image_filter = create_filter(ID_SIZE, ID_FACTOR, ID_BIAS, id);
	} else if (strcmp(filter_name, "fbl") == 0) {
		image_filter = create_filter(FAST_BLUR_SIZE, FAST_BLUR_FACTOR,
									 FAST_BLUR_BIAS, fast_blur);
	} else if (strcmp(filter_name, "bl") == 0) {
		image_filter = create_filter(BLUR_SIZE, BLUR_FACTOR, BLUR_BIAS, blur);
	} else if (strcmp(filter_name, "gbl") == 0) {
		image_filter = create_filter(GAUS_BLUR_SIZE, GAUS_BLUR_FACTOR,
									 GAUS_BLUR_BIAS, gaus_blur);
	} else if (strcmp(filter_name, "mbl") == 0) {
		image_filter = create_filter(MOTION_BLUR_SIZE, MOTION_BLUR_FACTOR,
									 MOTION_BLUR_BIAS, motion_blur);
	} else if (strcmp(filter_name, "ed") == 0) {
		image_filter = create_filter(EDGE_DETECTION_SIZE, EDGE_DETECTION_FACTOR,
									 EDGE_DETECTION_BIAS, edge_detection);
	} else if (strcmp(filter_name, "em") == 0) {
		image_filter =
			create_filter(EMBOSS_SIZE, EMBOSS_FACTOR, EMBOSS_BIAS, emboss);
	} else {
		if (handle_error(1, "Unknown filter name: %s\n", filter_name)) {
			goto cleanup_and_err;
		}
	}

	if (handle_error(image_filter.kernel == NULL,
					 "Memory allocation error for filter.\n")) {
		goto cleanup_and_err;
	}

	result_channel_image = initialize_image_rgb(width, height);
	if (handle_error(result_channel_image.red == NULL ||
						 result_channel_image.green == NULL ||
						 result_channel_image.blue == NULL,
					 "Memory allocation error for result_channel_image.\n")) {
		goto cleanup_and_err;
	}

	int return_value = 0;
	double start_time = get_time_in_seconds();
	if (handle_error(start_time == -1, "Error in clock_gettime().\n")) {
		goto cleanup_and_err;
	}

	if (strcmp(mode_str, "row") == 0) {
		return_value = parallel_row(&channel_image, &result_channel_image, width,
									height, image_filter, threads_num);
	} else if (strcmp(mode_str, "column") == 0) {
		return_value = parallel_column(&channel_image, &result_channel_image, width,
									   height, image_filter, threads_num);
	} else if (strcmp(mode_str, "block") == 0) {
		return_value = parallel_block(&channel_image, &result_channel_image, width,
									  height, image_filter, threads_num);
	} else if (strcmp(mode_str, "pixel") == 0) {
		return_value = parallel_pixel(&channel_image, &result_channel_image, width,
									  height, image_filter, threads_num);
	} else if (strcmp(mode_str, "seq") == 0) {
		sequential_application(&channel_image, &result_channel_image, width, height,
							   image_filter);
	} else {
		if (handle_error(1, "Unknown mode name: %s\n", mode_str)) {
			goto cleanup_and_err;
		}
	}
	double end_time = get_time_in_seconds();
	if (handle_error(end_time == -1, "Error in clock_gettime().\n")) {
		goto cleanup_and_err;
	}

	if (handle_error(return_value != 0, "Failed to create thread.\n")) {
		goto cleanup_and_err;
	}

	result_image = malloc((size_t)width * (size_t)height * (size_t)channels);
	if (handle_error(result_image == NULL,
					 "Memory allocation error for result_image.\n")) {
		goto cleanup_and_err;
	}

	assemble_image_from_rgb_channels(result_image, result_channel_image, width,
									 height);

	output_file_path =
		malloc(PATH_PREFIX_LENGTH + UNDERSCORE_COUNT + strlen(file_name) +
			   strlen(mode_str) + strlen(filter_name) + NULL_TERMINATOR_LENGTH);
	if (handle_error(output_file_path == NULL,
					 "Memory allocation error for output_file_path.\n")) {
		goto cleanup_and_err;
	}

	sprintf(output_file_path, "images/%s_%s_%s", filter_name, mode_str, file_name);

	stbi_write_bmp(output_file_path, width, height, 3, result_image);

	printf("The convolution took %.6f. The final image is located at '%s'\n",
		   (end_time - start_time), output_file_path);

	stbi_image_free(image);
	free_image_rgb(&channel_image);
	free_image_rgb(&result_channel_image);
	free_filter(&image_filter);
	free(result_image);
	free(output_file_path);

	return 0;

cleanup_and_err:
	if (image) {
		stbi_image_free(image);
	}
	free_image_rgb(&channel_image);
	free_image_rgb(&result_channel_image);
	free_filter(&image_filter);
	if (result_image) {
		free(result_image);
	}
	if (output_file_path) {
		free(output_file_path);
	}

	return -1;
}
