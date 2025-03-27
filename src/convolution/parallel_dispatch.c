#include "parallel_dispatch.h"

void parallel_row(struct image_rgb *input_image, struct image_rgb *output_image,
				  int width, int height, struct filter filter, int num_threads) {
	pthread_t threads[num_threads];
	struct thread_data thread_data_array[num_threads];

	int base_rows_per_thread = height / num_threads;
	int remainder = height % num_threads; // The rest of rows need to be distributed

	int start_y = 0;

	for (int i = 0; i < num_threads; i++) {
		int rows_for_thread = base_rows_per_thread + (i < remainder ? 1 : 0);

		thread_data_array[i].input_image = input_image;
		thread_data_array[i].output_image = output_image;
		thread_data_array[i].width = width;
		thread_data_array[i].height = height;
		thread_data_array[i].filter = filter;
		thread_data_array[i].start_x = 0;
		thread_data_array[i].start_y = start_y;
		thread_data_array[i].end_x = width;
		thread_data_array[i].end_y = start_y + rows_for_thread;

		pthread_create(&threads[i], NULL, process_image_part,
					   (void *)&thread_data_array[i]);

		start_y += rows_for_thread;
	}

	for (int i = 0; i < num_threads; i++) {
		pthread_join(threads[i], NULL);
	}
}

void parallel_column(struct image_rgb *input_image, struct image_rgb *output_image,
					 int width, int height, struct filter filter, int num_threads) {
	pthread_t threads[num_threads];
	struct thread_data thread_data_array[num_threads];

	int base_columns_per_thread = width / num_threads;
	int remainder = width % num_threads; // The rest of column need to be distributed

	int start_x = 0;

	for (int i = 0; i < num_threads; i++) {
		int columns_for_thread = base_columns_per_thread + (i < remainder ? 1 : 0);

		thread_data_array[i].input_image = input_image;
		thread_data_array[i].output_image = output_image;
		thread_data_array[i].width = width;
		thread_data_array[i].height = height;
		thread_data_array[i].filter = filter;
		thread_data_array[i].start_x = start_x;
		thread_data_array[i].start_y = 0;
		thread_data_array[i].end_x = start_x + columns_for_thread;
		thread_data_array[i].end_y = height;

		pthread_create(&threads[i], NULL, process_image_part,
					   (void *)&thread_data_array[i]);

		start_x += columns_for_thread;
	}

	for (int i = 0; i < num_threads; i++) {
		pthread_join(threads[i], NULL);
	}
}
