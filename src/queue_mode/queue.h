#pragma once

#include "../utils/utils.h"
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>

#include "stb_image.h"
#include "stb_image_write.h"

typedef struct queue_img_info {
	struct image_rgb image;
	char *filename;
	int width;
	int height;
	struct queue_img_info *next;
} img_info_node_t;

typedef struct img_queue {
	bool input_queue;

	img_info_node_t *head;
	img_info_node_t *tail;
	atomic_size_t current_mem_usage;
	size_t max_mem_usage;

	pthread_mutex_t pop_mutex;
	pthread_mutex_t push_mutex;
	pthread_cond_t cond_not_full;
	pthread_cond_t cond_not_empty;
} img_queue;

int queue_init(struct img_queue *q, size_t max_mem, bool input_queue);

void queue_destroy(struct img_queue *img_q);

bool queue_push(struct img_queue *img_q, struct image_rgb img, int width, int height,
				char *filename);

img_info_node_t *queue_pop(struct img_queue *img_q);
