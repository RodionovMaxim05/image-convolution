#include "../src/convolution/filter_application.h"
#include "../src/convolution/parallel_dispatch.h"

#include "utils_for_tests.c"

static void test_parallel_row_with_default_image(void **state) {
	(void)state;

	int width, height, channels;
	unsigned char *image =
		stbi_load("../../images/cat.bmp", &width, &height, &channels, 3);
	assert_true(image);

	struct image_rgb channel_image = initialize_and_check_image_rgb(width, height);
	split_image_into_rgb_channels(image, channel_image, width, height);

	struct filter filter = create_filter(9, 1.0 / 9.0, 0.0, motion_blur);
	assert_non_null(filter.kernel);

	struct image_rgb result_seq = initialize_and_check_image_rgb(width, height);
	struct image_rgb result_par = initialize_and_check_image_rgb(width, height);

	sequential_application(&channel_image, &result_seq, width, height, filter);
	parallel_row(&channel_image, &result_par, width, height, filter, 3);

	assert_memory_equal(result_seq.red, result_par.red, width * height);
	assert_memory_equal(result_seq.green, result_par.green, width * height);
	assert_memory_equal(result_seq.blue, result_par.blue, width * height);

	stbi_image_free(image);
	free_image_rgb(&channel_image);
	free_image_rgb(&result_seq);
	free_image_rgb(&result_par);
	free_filter(&filter);
}
