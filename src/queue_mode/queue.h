#pragma once

#include "../utils/utils.h"
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>

#include "stb_image.h"
#include "stb_image_write.h"

/**
 * Stores image data, metadata and pointer to next node in the linked list.
 *
 * @param image RGB channels of the image (`struct image_rgb`).
 * @param filename Name of the source file (or NULL for termination signal).
 * @param width Width of the image.
 * @param height Height of the image.
 * @param next Pointer to the next node in the queue.
 */
typedef struct queue_img_info {
	struct image_rgb image;
	char *filename;
	int width;
	int height;
	struct queue_img_info *next;
} img_info_node_t;

/**
 * Manages a linked list of images with atomic memory tracking and synchronization
 * primitives for concurrent access from multiple threads.
 *
 * @param input_queue Boolean indicating whether this is an input queue.
 * @param head Head of the queue (oldest item).
 * @param tail Tail of the queue (newest item).
 * @param current_mem_usage Current memory usage of all items in bytes.
 * @param max_mem_usage Maximum allowed memory usage in bytes.
 * @param push_mutex Mutex for protecting push operations.
 * @param pop_mutex Mutex for protecting pop operations.
 * @param cond_not_full Condition variable for signaling when queue is not full.
 * @param cond_not_empty Condition variable for signaling when queue is not empty.
 */
typedef struct img_queue {
	bool input_queue;

	img_info_node_t *head;
	img_info_node_t *tail;
	atomic_size_t current_mem_usage;
	size_t max_mem_usage;

	pthread_mutex_t push_mutex;
	pthread_mutex_t pop_mutex;
	pthread_cond_t cond_not_full;
	pthread_cond_t cond_not_empty;
} img_queue;

/**
 * Sets up internal synchronization primitives and prepares queue for use.
 *
 * @param img_q Pointer to the queue structure to initialize.
 * @param max_mem Maximum memory usage allowed for the queue in bytes.
 * @param input_queue Boolean indicating whether this is an input queue.
 * @return `0` on success, `-1` on error during mutex/condition initialization.
 */
int queue_init(struct img_queue *img_q, size_t max_mem, bool input_queue);

/**
 * Frees all queued images and destroys synchronization primitives.
 *
 * @param img_q Pointer to the queue to destroy.
 */
void queue_destroy(struct img_queue *img_q);

/**
 * Creates and pushes an `img_info_node_t` to the queue, downloading and splitting
 * image data if it is an input queue. Blocks the thread if the queue's memory usage
 * exceeds the limit until space becomes available.
 *
 * @param img_q Pointer to the queue.
 * @param img RGB image data to enqueue (`struct image_rgb`).
 * @param width Width of the image.
 * @param height Height of the image.
 * @param filename Path to image file or NULL for termination signal.
 *
 * @return `0` on success, `-1` on memory allocation failure or image load
 * error.
 */
int queue_push(struct img_queue *img_q, struct image_rgb img, int width, int height,
			   char *filename);

/**
 * Removes `img_info_node_t` from the queue. Blocks the thread if the queue is empty
 * until a new element arrives.
 *
 * @param img_q Pointer to the queue.
 * @return Pointer to image info node, or NULL on error.
 */
img_info_node_t *queue_pop(struct img_queue *img_q);
