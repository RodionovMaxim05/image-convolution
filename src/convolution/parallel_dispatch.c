#include "parallel_dispatch.h"

int parallel_filter(struct image_rgb *input_image, struct image_rgb *output_image,
					int width, int height, struct filter filter, int num_threads,
					int block_width, int block_height) {
	pthread_t threads[num_threads];
	struct thread_data thread_data_array[num_threads];

	int num_cols = (width + block_width - 1) / block_width;
	int num_rows = (height + block_height - 1) / block_height;
	int num_blocks = num_cols * num_rows;

	atomic_int next_block;
	atomic_init(&next_block, 0);

	for (int i = 0; i < num_threads; i++) {
		thread_data_array[i].input_image = input_image;
		thread_data_array[i].output_image = output_image;
		thread_data_array[i].width = width;
		thread_data_array[i].height = height;
		thread_data_array[i].filter = filter;
		thread_data_array[i].block_width = block_width;
		thread_data_array[i].block_height = block_height;
		thread_data_array[i].num_cols = num_cols;
		thread_data_array[i].num_blocks = num_blocks;
		thread_data_array[i].next_block = &next_block;

		if (pthread_create(&threads[i], NULL, process_dynamic,
						   (void *)&thread_data_array[i]) != 0) {
			error("Failed to create a thread\n");
			for (int j = 0; j < i; j++) {
				pthread_cancel(threads[j]);
			}
			return -1;
		}
	}

	for (int i = 0; i < num_threads; i++) {
		pthread_join(threads[i], NULL);
	}

	return 0;
}

int parallel_pixel(struct image_rgb *input_image, struct image_rgb *output_image,
				   int width, int height, struct filter filter, int num_threads) {
	return parallel_filter(input_image, output_image, width, height, filter,
						   num_threads, 1, 1);
}

int parallel_row(struct image_rgb *input_image, struct image_rgb *output_image,
				 int width, int height, struct filter filter, int num_threads) {
	return parallel_filter(input_image, output_image, width, height, filter,
						   num_threads, width, 1);
}

int parallel_column(struct image_rgb *input_image, struct image_rgb *output_image,
					int width, int height, struct filter filter, int num_threads) {
	return parallel_filter(input_image, output_image, width, height, filter,
						   num_threads, 1, height);
}

int parallel_block(struct image_rgb *input_image, struct image_rgb *output_image,
				   int width, int height, struct filter filter, int num_threads) {
	int block_width = (width + num_threads - 1) / num_threads;
	int block_height = (height + num_threads - 1) / num_threads;

	return parallel_filter(input_image, output_image, width, height, filter,
						   num_threads, block_width, block_height);
}
