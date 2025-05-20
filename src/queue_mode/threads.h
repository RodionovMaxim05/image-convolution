#pragma once

#include "../convolution/parallel_dispatch.h"
#include "../utils/args.h"
#include "queue.h"
#include <dirent.h>
#include <stdatomic.h>
#include <stdint.h>

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

void *reader_thread(void *arg);

void *worker_thread(void *arg);

void *writer_thread(void *arg);
