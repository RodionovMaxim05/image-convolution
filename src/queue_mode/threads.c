#include "threads.h"

#define MAX_PATH_LEN 264

atomic_size_t read_images = 0;
atomic_size_t finished_reader_threads = 0;
atomic_size_t written_images = 0;
atomic_bool input_termination_sent = false;
atomic_bool output_termination_sent = false;

void *reader_thread(void *arg) {
	qthreads_info *info = (qthreads_info *)arg;

	struct image_rgb dummy_img = {NULL, NULL, NULL};
	int width, height, channels;
	double start_time, end_time;
	size_t index;
	char *path;

	while ((index = atomic_fetch_add(&read_images, 1)) < info->pargs->img_count) {
		path = info->file_paths[index];

		start_time = get_time_in_seconds();
		if (start_time == -1) {
			break;
		}

		if (!stbi_info(path, &width, &height, &channels)) {
			error("READER: Failed to read image info from '%s'\n", path);
			continue;
		}

		// Checks that the image size is less than the size limit passed by the user
		double weight_mib = (double)(width * height * 3) / BYTES_IN_MEBIBYTE;
		double memory_lim_mib = (double)info->pargs->memory_lim / BYTES_IN_MEBIBYTE;
		if (info->pargs->memory_lim < ((size_t)width * (size_t)height * 3)) {
			error("'%s' (%.1f MiB) is larger than the maximum specified size - %.1f "
				  "MiB.\n",
				  path, weight_mib, memory_lim_mib);
			continue;
		}

		if (!queue_push(info->input_q, dummy_img, width, height, path)) {
			error("READER: Failed to push '%s' into input queue\n", path);
			continue;
		}

		end_time = get_time_in_seconds();
		if (end_time == -1) {
			break;
		}
		printf("READER: It took %.6f seconds to load and add '%s' to input queue.\n",
			   (end_time - start_time), path);
	}
	atomic_fetch_add(&finished_reader_threads, 1);

	// Sending termination signals
	if (atomic_load(&finished_reader_threads) == info->pargs->readers_num &&
		!atomic_exchange(&input_termination_sent, true)) {
		for (uint8_t i = 0; i < info->pargs->workers_num; i++) {
			queue_push(info->input_q, dummy_img, 0, 0, NULL);
		}
		printf("READER: Sent %d termination signals to workers.\n",
			   info->pargs->workers_num);
	}

	printf("Reader end his work.\n");
	return NULL;
}

void *worker_thread(void *arg) {
	qthreads_info *info = (qthreads_info *)arg;

	double start_time, end_time;
	img_info_node_t *out_node;

	while (1) {
		start_time = get_time_in_seconds();
		if (start_time == -1) {
			break;
		}

		out_node = queue_pop(info->input_q);

		// Checking for a termination signal
		if (!out_node->filename) {
			free(out_node);
			break;
		}

		struct image_rgb result_channel_image =
			initialize_image_rgb(out_node->width, out_node->height);
		if (!result_channel_image.red || !result_channel_image.green ||
			!result_channel_image.blue) {
			error("WORKER: Memory allocation error for result_channel_image.\n");
			free_image_rgb(&out_node->image);
			free(out_node);
			continue;
		}

		parallel_row(&out_node->image, &result_channel_image, out_node->width,
					 out_node->height, *info->img_filter, info->pargs->threads_num);

		if (!queue_push(info->output_q, result_channel_image, out_node->width,
						out_node->height, out_node->filename)) {
			error("WORKER: Failed to push processed image to output queue.\n");
			free_image_rgb(&out_node->image);
			free_image_rgb(&result_channel_image);
			free(out_node);
			break;
		}
		atomic_fetch_add(&written_images, 1);

		end_time = get_time_in_seconds();
		if (end_time == -1) {
			free_image_rgb(&out_node->image);
			free(out_node);
			break;
		}
		printf("WORKER: Getting from input queue, applying the filter, and adding "
			   "'%s' to output queue took %.6f seconds.\n",
			   out_node->filename, (end_time - start_time));

		free_image_rgb(&out_node->image);
		free(out_node);
	}

	// Sending termination signals
	struct image_rgb dummy_img = {NULL, NULL, NULL};
	if (atomic_load(&written_images) == info->pargs->img_count &&
		!atomic_exchange(&output_termination_sent, true)) {
		for (uint8_t i = 0; i < info->pargs->writers_num; i++) {
			queue_push(info->output_q, dummy_img, 0, 0, NULL);
		}
		printf("WORKER: Sent %d termination signals to writers.\n",
			   info->pargs->workers_num);
	}

	printf("Worker end his work.\n");
	return NULL;
}

void *writer_thread(void *arg) {
	struct qthreads_info *info = (struct qthreads_info *)arg;

	double start_time, end_time;
	img_info_node_t *out_node;

	while (1) {
		start_time = get_time_in_seconds();
		if (start_time == -1) {
			break;
		}

		out_node = queue_pop(info->output_q);

		// Checking for a termination signal
		if (out_node->filename == NULL) {
			free(out_node);
			break;
		}

		unsigned char *result_image =
			malloc((size_t)out_node->width * (size_t)out_node->height * 3);
		if (!result_image) {
			error("WRITER: Memory allocation failed for output image\n");
			free_image_rgb(&out_node->image);
			free(out_node);
			break;
		}

		assemble_image_from_rgb_channels(result_image, out_node->image,
										 out_node->width, out_node->height);

		char out_path[MAX_PATH_LEN];
		snprintf(out_path, sizeof(out_path), "%s/%s", QUEUE_DIR_NAME,
				 extract_filename(out_node->filename));

		if (!stbi_write_bmp(out_path, out_node->width, out_node->height, 3,
							result_image)) {
			error("WRITER: Failed to save image '%s'\n", out_path);
			free(result_image);
			free_image_rgb(&out_node->image);
			free(out_node);
			continue;
		}

		end_time = get_time_in_seconds();
		if (end_time == -1) {
			break;
			free(result_image);
			free_image_rgb(&out_node->image);
			free(out_node);
		}
		printf("WRITER: It took %.6f seconds to get the '%s' from output queue and "
			   "save it.\n",
			   (end_time - start_time), out_node->filename);

		free(result_image);
		free_image_rgb(&out_node->image);
		free(out_node);
	}

	printf("Writer end his work.\n");
	return NULL;
}
