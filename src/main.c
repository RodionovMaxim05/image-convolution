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

/**
 * A structure to store the parsed command-line arguments.
 *
 * @param image_path Path to the input image file or "images/cat.bmp" if
--default-image is specified.
 * @param filter_name Name of the filter to apply.
 * @param mode Execution mode ("seq", "row", "column", "block", or "pixel").
 * @param threads_num Number of threads to use in parallel modes (ignored for "seq").
 */
typedef struct {
	const char *image_path;
	const char *filter_name;
	const char *mode;
	int threads_num;
} ProgramArgs;

/**
 * Handles runtime errors by printing an error message (using error) if the specified
 * condition is true.
 *
 * @param condition A boolean value indicating whether an error has occurred.
 * @param message String describing the error.
 * @param ... Additional arguments for formatting the error message.
 *
 * @return `true` if the condition is true (indicating an error occurred), `false`
 * otherwise.
 */
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

/**
 * Parses the command-line arguments provided to the program. It validates the input
 * and populates the ProgramArgs structure with the parsed values.
 *
 * @param argc The number of command-line arguments.
 * @param argv An array of strings containing the command-line arguments.
 * @param args A pointer to a `ProgramArgs` structure where the parsed arguments will
 * be stored.
 *
 * @return `true` if parsing is successful, `false` if there is an error.
 */
static bool parse_args(int argc, char *argv[], ProgramArgs *args) {
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
		return false;
	}

	args->image_path =
		strcmp(argv[1], "--default-image") == 0 ? "images/cat.bmp" : argv[1];
	args->filter_name = argv[2];

	if (strncmp(argv[3], "--mode=", MODE_PREFIX_LEN) != 0) {
		handle_error(true, "Missing --mode argument\n");
		return false;
	}
	args->mode = argv[3] + MODE_PREFIX_LEN;

	if (strcmp(args->mode, "seq") != 0) {
		if (strncmp(argv[4], "--thread=", THREAD_PREFIX_LEN) != 0) {
			handle_error(true, "Missing --thread argument\n");
			return false;
		}
		args->threads_num = atoi(argv[4] + THREAD_PREFIX_LEN);
	}

	return true;
}

/**
 * The entry point of the program. It organizes the entire process of loading an
 * image, applying a convolution filter, and saving the result.
 */
int main(int argc, char *argv[]) {
	ProgramArgs args = {NULL, NULL, NULL, 1};
	if (!parse_args(argc, argv, &args)) {
		return -1;
	}

	int width, height, channels;
	unsigned char *image = NULL;
	struct image_rgb channel_image = {NULL, NULL, NULL};
	struct image_rgb result_channel_image = {NULL, NULL, NULL};
	struct filter image_filter = {0, 0, 0, NULL};
	unsigned char *result_image = NULL;
	char *output_file_path = NULL;

	// Load image
	image = stbi_load(args.image_path, &width, &height, &channels, 3);
	if (handle_error(!image, "Could not open or find the image!\n")) {
		goto cleanup_and_err;
	}

	// Initialize RGB channels
	channel_image = initialize_image_rgb(width, height);
	if (handle_error(channel_image.red == NULL || channel_image.green == NULL ||
						 channel_image.blue == NULL,
					 "Memory allocation error for channel_image.\n")) {
		goto cleanup_and_err;
	}

	split_image_into_rgb_channels(image, channel_image, width, height);

	// Create filter
	if (strcmp(args.filter_name, "id") == 0) {
		image_filter = create_filter(ID_SIZE, ID_FACTOR, ID_BIAS, id);
	} else if (strcmp(args.filter_name, "fbl") == 0) {
		image_filter = create_filter(FAST_BLUR_SIZE, FAST_BLUR_FACTOR,
									 FAST_BLUR_BIAS, fast_blur);
	} else if (strcmp(args.filter_name, "bl") == 0) {
		image_filter = create_filter(BLUR_SIZE, BLUR_FACTOR, BLUR_BIAS, blur);
	} else if (strcmp(args.filter_name, "gbl") == 0) {
		image_filter = create_filter(GAUS_BLUR_SIZE, GAUS_BLUR_FACTOR,
									 GAUS_BLUR_BIAS, gaus_blur);
	} else if (strcmp(args.filter_name, "mbl") == 0) {
		image_filter = create_filter(MOTION_BLUR_SIZE, MOTION_BLUR_FACTOR,
									 MOTION_BLUR_BIAS, motion_blur);
	} else if (strcmp(args.filter_name, "ed") == 0) {
		image_filter = create_filter(EDGE_DETECTION_SIZE, EDGE_DETECTION_FACTOR,
									 EDGE_DETECTION_BIAS, edge_detection);
	} else if (strcmp(args.filter_name, "em") == 0) {
		image_filter =
			create_filter(EMBOSS_SIZE, EMBOSS_FACTOR, EMBOSS_BIAS, emboss);
	} else {
		if (handle_error(1, "Unknown filter name: %s\n", args.filter_name)) {
			goto cleanup_and_err;
		}
	}

	if (handle_error(image_filter.kernel == NULL,
					 "Memory allocation error for filter.\n")) {
		goto cleanup_and_err;
	}

	// Initialize result channels
	result_channel_image = initialize_image_rgb(width, height);
	if (handle_error(result_channel_image.red == NULL ||
						 result_channel_image.green == NULL ||
						 result_channel_image.blue == NULL,
					 "Memory allocation error for result_channel_image.\n")) {
		goto cleanup_and_err;
	}

	// Apply convolution
	double start_time = get_time_in_seconds();
	if (handle_error(start_time == -1, "Error in clock_gettime().\n")) {
		goto cleanup_and_err;
	}

	int return_value = 0;
	if (strcmp(args.mode, "row") == 0) {
		return_value = parallel_row(&channel_image, &result_channel_image, width,
									height, image_filter, args.threads_num);
	} else if (strcmp(args.mode, "column") == 0) {
		return_value = parallel_column(&channel_image, &result_channel_image, width,
									   height, image_filter, args.threads_num);
	} else if (strcmp(args.mode, "block") == 0) {
		return_value = parallel_block(&channel_image, &result_channel_image, width,
									  height, image_filter, args.threads_num);
	} else if (strcmp(args.mode, "pixel") == 0) {
		return_value = parallel_pixel(&channel_image, &result_channel_image, width,
									  height, image_filter, args.threads_num);
	} else if (strcmp(args.mode, "seq") == 0) {
		sequential_application(&channel_image, &result_channel_image, width, height,
							   image_filter);
	} else {
		if (handle_error(1, "Unknown mode name: %s\n", args.mode)) {
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

	// Assemble and save result
	result_image = malloc((size_t)width * (size_t)height * (size_t)channels);
	if (handle_error(result_image == NULL,
					 "Memory allocation error for result_image.\n")) {
		goto cleanup_and_err;
	}

	assemble_image_from_rgb_channels(result_image, result_channel_image, width,
									 height);

	const char *file_name = extract_filename(args.image_path);
	output_file_path = malloc(PATH_PREFIX_LENGTH + UNDERSCORE_COUNT +
							  strlen(file_name) + strlen(args.mode) +
							  strlen(args.filter_name) + NULL_TERMINATOR_LENGTH);
	if (handle_error(output_file_path == NULL,
					 "Memory allocation error for output_file_path.\n")) {
		goto cleanup_and_err;
	}

	sprintf(output_file_path, "images/%s_%s_%s", args.filter_name, args.mode,
			file_name);

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
	free(result_image);
	free(output_file_path);

	return -1;
}
