#include "convolution/parallel_dispatch.h"
#include "filters/filter.h"
#include "queue_mode/queue_dispatch.h"
#include "queue_mode/threads.h"
#include "utils/args.h"

#include <errno.h>
#include <sys/stat.h>

#define PATH_PREFIX_LEN 7	  // Length "images/"
#define UNDERSCORE_COUNT 2	  // Number of underscores
#define NULL_TERMINATOR_LEN 1 // Terminating null character '\0'
#define DIR_ACCESS_RIGHTS 0755

static int default_mode(program_args args, struct filter image_filter) {
	int width, height, channels;
	unsigned char *image = NULL;
	struct image_rgb channel_image = {NULL, NULL, NULL};
	struct image_rgb result_channel_image = {NULL, NULL, NULL};
	unsigned char *result_image = NULL;
	char *output_file_path = NULL;

	// Load image
	image = stbi_load(args.img_path, &width, &height, &channels, 3);
	if (!image) {
		error("Could not open or find the image!\n");
		goto cleanup_and_err;
	}

	// Initialize RGB channels
	channel_image = initialize_image_rgb(width, height);
	if (channel_image.red == NULL || channel_image.green == NULL ||
		channel_image.blue == NULL) {
		error("Memory allocation error for channel_image.\n");
		goto cleanup_and_err;
	}

	split_image_into_rgb_channels(image, channel_image, width, height);

	// Initialize result channels
	result_channel_image = initialize_image_rgb(width, height);
	if (result_channel_image.red == NULL || result_channel_image.green == NULL ||
		result_channel_image.blue == NULL) {
		error("Memory allocation error for result_channel_image.\n");
		goto cleanup_and_err;
	}

	// Apply convolution
	double start_time = get_time_in_seconds();
	if (start_time == -1) {
		error("Error in clock_gettime().\n");
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
		error("Unknown mode name: %s\n", args.mode);
		goto cleanup_and_err;
	}

	double end_time = get_time_in_seconds();
	if (end_time == -1) {
		error("Error in clock_gettime().\n");
		goto cleanup_and_err;
	}

	if (return_value != 0) {
		error("Failed to create thread.\n");
		goto cleanup_and_err;
	}

	// Assemble and save result
	result_image = malloc((size_t)width * (size_t)height * 3);
	if (!result_image) {
		error("Memory allocation error for result_image.\n");
		goto cleanup_and_err;
	}

	assemble_image_from_rgb_channels(result_image, result_channel_image, width,
									 height);

	const char *file_name = extract_filename(args.img_path);
	output_file_path =
		malloc(PATH_PREFIX_LEN + UNDERSCORE_COUNT + strlen(file_name) +
			   strlen(args.mode) + strlen(args.filter_name) + NULL_TERMINATOR_LEN);
	if (!output_file_path) {
		error("Memory allocation error for output_file_path.\n");
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
	free(result_image);
	free(output_file_path);

	return 0;

cleanup_and_err:
	if (image) {
		stbi_image_free(image);
	}
	free_image_rgb(&channel_image);
	free_image_rgb(&result_channel_image);
	free(result_image);
	free(output_file_path);

	return -1;
}

static int queue_mode(program_args args, struct filter image_filter) {
	if (mkdir(QUEUE_DIR_NAME, DIR_ACCESS_RIGHTS) == -1) {
		if (errno != EEXIST) {
			error("Error creating directory.\n");
			return 1;
		}
	}

	img_queue input_queue, output_queue;
	if (queue_init(&input_queue, args.memory_lim, true) != 0) {
		error("Memory allocation error for input_queue.\n");
	}

	if (queue_init(&output_queue, args.memory_lim, false) != 0) {
		error("Memory allocation error for output_queue.\n");
		queue_destroy(&input_queue);
	}

	char **file_paths = get_file_paths(args.img_path, args.img_count);
	if (!file_paths) {
		return -1;
	}

	qthreads_info info = {
		.readers = NULL,
		.workers = NULL,
		.writers = NULL,
		.pargs = &args,
		.file_paths = file_paths,
		.img_filter = &image_filter,
		.input_q = &input_queue,
		.output_q = &output_queue,
	};

	if (start_threads(&info) != 0) {
		return -1;
	}

	free_file_paths(file_paths, args.img_count);
	queue_destroy(&input_queue);
	queue_destroy(&output_queue);

	return 0;
}

int main(int argc, char *argv[]) {
	program_args args = {NULL, NULL, NULL, 1, 0, 0, 0, 0, 0};
	if (!parse_args(argc, argv, &args)) {
		return -1;
	}

	// Create filter
	struct filter image_filter = {0, 0, 0, NULL};

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
	} else if (strcmp(args.filter_name, "bl+gbl") == 0) {
		image_filter = compose_filters_from_params(
			BLUR_SIZE, BLUR_FACTOR, BLUR_BIAS, blur, GAUS_BLUR_SIZE,
			GAUS_BLUR_FACTOR, GAUS_BLUR_BIAS, gaus_blur);
	} else if (strcmp(args.filter_name, "fbl+mbl") == 0) {
		image_filter = compose_filters_from_params(
			FAST_BLUR_SIZE, FAST_BLUR_FACTOR, FAST_BLUR_BIAS, fast_blur,
			MOTION_BLUR_SIZE, MOTION_BLUR_FACTOR, MOTION_BLUR_BIAS, motion_blur);
	} else {
		error("Unknown filter name: %s\n", args.filter_name);
		if (image_filter.kernel != NULL) {
			free_filter(&image_filter);
		}
	}

	if (image_filter.kernel == NULL) {
		error("Memory allocation error for filter.\n");
		return -1;
	}

	if (strcmp(args.mode, "queue") == 0) {
		if (queue_mode(args, image_filter) != 0) {
			return -1;
		}
	} else {
		if (default_mode(args, image_filter) != 0) {
			return -1;
		}
	}

	free_filter(&image_filter);

	return 0;
}
