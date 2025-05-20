#include "queue_dispatch.h"

/**
 * Allocates separate arrays of `pthread_t` for each group: readers, workers, and
 * writers, based on the number specified in the program arguments.
 */
int allocate_threads(qthreads_info *info) {
	info->readers = malloc(info->pargs->readers_num * sizeof(pthread_t));
	if (!info->readers) {
		return -1;
	}

	info->workers = malloc(info->pargs->workers_num * sizeof(pthread_t));
	if (!info->workers) {
		free(info->readers);
		return -1;
	}

	info->writers = malloc(info->pargs->writers_num * sizeof(pthread_t));
	if (!info->writers) {
		free(info->readers);
		free(info->workers);
		return -1;
	}

	return 0;
}

/**
 * Deallocates memory previously allocated by `allocate_threads`.
 */
void free_threads(qthreads_info *info) {
	free(info->readers);
	free(info->workers);
	free(info->writers);
}

/**
 * Starts `num` threads that execute the function pointed to by `start_routine`.
 * If thread creation fails, already created threads are cancelled.
 */
int create_thread_group(pthread_t *threads, uint8_t num,
						void *(*start_routine)(void *), void *arg) {
	for (uint8_t i = 0; i < num; ++i) {
		if (pthread_create(&threads[i], NULL, start_routine, arg) != 0) {
			// Cancel successfully created threads
			for (uint8_t j = 0; j < i; ++j) {
				pthread_cancel(threads[j]);
			}

			return -1;
		}
	}

	return 0;
}

/**
 * Joins each thread in the provided array using `pthread_join()`.
 */
void join_thread_group(pthread_t *threads, uint8_t num) {
	for (uint8_t i = 0; i < num; ++i) {
		pthread_join(threads[i], NULL);
	}
}

int start_threads(qthreads_info *info) {
	if (allocate_threads(info) != 0) {
		error("Failed to allocate thread memory\n");
		return -1;
	}

	// Create thread groups

	if (create_thread_group(info->readers, info->pargs->readers_num, reader_thread,
							info) != 0) {
		error("Failed to create thread.\n");
		return -1;
	}

	if (create_thread_group(info->workers, info->pargs->workers_num, worker_thread,
							info) != 0) {
		error("Failed to create thread.\n");
		return -1;
	}

	if (create_thread_group(info->writers, info->pargs->writers_num, writer_thread,
							info) != 0) {
		error("Failed to create thread.\n");
		return -1;
	}

	// Join threads

	join_thread_group(info->readers, info->pargs->readers_num);
	join_thread_group(info->workers, info->pargs->workers_num);
	join_thread_group(info->writers, info->pargs->writers_num);
	free_threads(info);

	return 0;
}
