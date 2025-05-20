#pragma once

#include "threads.h"
#include <pthread.h>

/**
 * Manages the full lifecycle of reader, worker, and writer threads in "queue" mode:
 * - Allocates thread storage
 * - Creates threads for each group
 * - Waits for all threads to finish
 * - Frees allocated memory
 *
 * @param info Pointer to a `qthreads_info` structure containing thread
 * configuration.
 * @return `0` on success, `-1` on error during thread creation or allocation.
 */
int start_threads(qthreads_info *info);
