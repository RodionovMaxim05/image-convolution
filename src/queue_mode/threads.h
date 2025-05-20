#pragma once

#include "../convolution/parallel_dispatch.h"
#include "../utils/args.h"
#include "queue.h"
#include <dirent.h>
#include <stdatomic.h>
#include <stdint.h>

/**
 * Contains all necessary data shared between threads during queue-based processing.
 *
 * @param readers Array of pthread IDs for reader threads.
 * @param workers Array of pthread IDs for worker threads.
 * @param writers Array of pthread IDs for writer threads.
 * @param pargs Parsed command-line arguments.
 * @param file_paths Array of file paths to be processed.
 * @param img_filter Pointer to the filter to be applied.
 * @param input_q Input queue containing images read by readers and processed by
 * workers.
 * @param output_q Output queue containing filtered images ready to be saved by
 * writers.
 */
typedef struct qthreads_info {
	pthread_t *readers;
	pthread_t *workers;
	pthread_t *writers;
	program_args *pargs;
	char **file_paths;
	struct filter *img_filter;
	img_queue *input_q;
	img_queue *output_q;
} qthreads_info;

/**
 * Reads `.bmp` files from the provided list and pushes them into the input queue.
 *
 * @param arg Pointer to a `qthreads_info` structure.
 * @return Always returns `NULL`.
 */
void *reader_thread(void *arg);

/**
 * Dequeues images from the input queue, applies the selected filter,  and enqueues
 * the result to the output queue.
 *
 * @param arg Pointer to a `qthreads_info` structure.
 * @return Always returns `NULL`.
 */
void *worker_thread(void *arg);

/**
 * @brief Thread function for saving processed images to disk.
 *
 * Dequeues filtered images from the output queue and saves them as `.bmp` files.
 *
 * @param arg Pointer to a `qthreads_info` structure.
 * @return Always returns `NULL`.
 */
void *writer_thread(void *arg);
