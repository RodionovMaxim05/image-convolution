#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "queue.h"

int queue_init(struct img_queue *img_q, size_t max_mem, bool input_queue) {
	img_q->input_queue = input_queue;
	img_q->head = NULL;
	img_q->tail = NULL;
	atomic_store(&img_q->current_mem_usage, 0);
	img_q->max_mem_usage = max_mem;

	pthread_mutex_init(&img_q->push_mutex, NULL);
	pthread_mutex_init(&img_q->pop_mutex, NULL);

	if (pthread_cond_init(&img_q->cond_not_full, NULL) != 0) {
		pthread_mutex_destroy(&img_q->push_mutex);
		pthread_mutex_destroy(&img_q->pop_mutex);
		return -1;
	}

	if (pthread_cond_init(&img_q->cond_not_empty, NULL) != 0) {
		pthread_mutex_destroy(&img_q->push_mutex);
		pthread_mutex_destroy(&img_q->pop_mutex);
		pthread_cond_destroy(&img_q->cond_not_full);
		return -1;
	}

	return 0;
}

void queue_destroy(struct img_queue *img_q) {
	img_info_node_t *current = img_q->head;
	while (current) {
		img_info_node_t *next = current->next;

		free_image_rgb(&current->image);
		free(current);

		current = next;
	}

	pthread_mutex_destroy(&img_q->push_mutex);
	pthread_mutex_destroy(&img_q->pop_mutex);
	pthread_cond_destroy(&img_q->cond_not_full);
	pthread_cond_destroy(&img_q->cond_not_empty);
}

bool queue_push(img_queue *img_q, struct image_rgb img, int width, int height,
				char *filename) {
	pthread_mutex_lock(&img_q->push_mutex);

	size_t img_weight = (size_t)width * (size_t)height * 3;

	// Waiting for space to become available
	while (atomic_load(&img_q->current_mem_usage) + img_weight >
		   img_q->max_mem_usage) {
		pthread_cond_wait(&img_q->cond_not_full, &img_q->push_mutex);
	}

	img_info_node_t *node = malloc(sizeof(img_info_node_t));
	if (!node) {
		return false;
	}

	node->image = img;
	node->filename = filename;
	node->next = NULL;

	// Load and split image
	if (img_q->input_queue && filename) {
		int channels;
		unsigned char *image_data =
			stbi_load(filename, &width, &height, &channels, 3);
		if (!image_data) {
			error("Failed to load image: %s\n", filename);
			free(node);
			return false;
		}

		// Initialize RGB channels
		struct image_rgb channel_image = initialize_image_rgb(width, height);
		if (channel_image.red == NULL || channel_image.green == NULL ||
			channel_image.blue == NULL) {
			error("Failed to initialize image rgb for %s.\n", filename);
			free(node);
			stbi_image_free(image_data);
			return false;
		}

		split_image_into_rgb_channels(image_data, channel_image, width, height);

		node->image = channel_image;
		stbi_image_free(image_data);
	}

	node->width = width;
	node->height = height;

	atomic_fetch_add(&img_q->current_mem_usage, img_weight);

	// Push to the queue
	if (img_q->tail) {
		img_q->tail->next = node;
		img_q->tail = node;
	} else {
		img_q->tail = node;
		img_q->head = node;
	}

	pthread_cond_signal(&img_q->cond_not_empty);
	pthread_mutex_unlock(&img_q->push_mutex);

	return true;
}

img_info_node_t *queue_pop(img_queue *img_q) {
	pthread_mutex_lock(&img_q->pop_mutex);

	// Waiting for an item in the queue
	while (!img_q->head) {
		pthread_cond_wait(&img_q->cond_not_empty, &img_q->pop_mutex);
	}

	img_info_node_t *out_node = img_q->head;

	// Pop from the queue
	img_q->head = out_node->next;
	if (!img_q->head) {
		img_q->tail = NULL;
	}

	// Checking for a termination signal
	if (!out_node->filename) {
		pthread_mutex_unlock(&img_q->pop_mutex);
		return out_node;
	}

	atomic_fetch_sub(&img_q->current_mem_usage,
					 ((size_t)out_node->width * (size_t)out_node->height * 3));

	pthread_cond_signal(&img_q->cond_not_full);
	pthread_mutex_unlock(&img_q->pop_mutex);

	return out_node;
}
