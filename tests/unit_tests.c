#include "../src/filters/filter.h"
#include "../src/sequential_convolution/filter_application.h"
#include "../src/utils/utils.h"

#define IMAGE_WIDTH 2
#define IMAGE_HEIGHT 2

unsigned char test_image[] = {
	255, 0, 0,	 0,	  255, 0,  // red green
	0,	 0, 255, 255, 255, 255 // blue white
};

static void test_create_filter(void **state) {
	(void)state;

	double kernel[3][3] = {{0, 1, 0}, {1, 1, 0}, {1, 1, 1}};

	struct filter filter = create_filter(3, 1.0, 0.0, kernel);

	assert_int_equal(filter.size, 3);
	assert_double_equal(filter.factor, 1.0, 1e-6);
	assert_double_equal(filter.bias, 0.0, 1e-6);

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			assert_double_equal(filter.kernel[i][j], kernel[i][j], 1e-6);
		}
	}

	free_filter(&filter);
}

static void test_split_assemble_channels(void **state) {
	(void)state;

	struct image_rgb channel_image = initialize_image_rgb(IMAGE_WIDTH, IMAGE_HEIGHT);
	split_image_into_rgb_channels(test_image, channel_image, IMAGE_WIDTH,
								  IMAGE_HEIGHT);

	assert_int_equal(channel_image.red[0], 255);
	assert_int_equal(channel_image.green[0], 0);
	assert_int_equal(channel_image.blue[0], 0);

	assert_int_equal(channel_image.red[1], 0);
	assert_int_equal(channel_image.green[1], 255);
	assert_int_equal(channel_image.blue[1], 0);

	assert_int_equal(channel_image.red[2], 0);
	assert_int_equal(channel_image.green[2], 0);
	assert_int_equal(channel_image.blue[2], 255);

	assert_int_equal(channel_image.red[3], 255);
	assert_int_equal(channel_image.green[3], 255);
	assert_int_equal(channel_image.blue[3], 255);

	unsigned char *assembled_image = malloc(IMAGE_WIDTH * IMAGE_HEIGHT * 3);
	assemble_image_from_rgb_channels(assembled_image, channel_image, IMAGE_WIDTH,
									 IMAGE_HEIGHT);

	assert_memory_equal(test_image, assembled_image, IMAGE_WIDTH * IMAGE_HEIGHT * 3);

	free(assembled_image);
	free_image_rgb(&channel_image);
}

static void test_identity_filter(void **state) {
	(void)state;

	struct image_rgb channel_image = initialize_image_rgb(IMAGE_WIDTH, IMAGE_HEIGHT);
	split_image_into_rgb_channels(test_image, channel_image, IMAGE_WIDTH,
								  IMAGE_HEIGHT);

	struct image_rgb result_channel_image =
		initialize_image_rgb(IMAGE_WIDTH, IMAGE_HEIGHT);

	double identity_kernel[3][3] = {{0, 0, 0}, {0, 1, 0}, {0, 0, 0}};
	struct filter filter = create_filter(3, 1.0, 0.0, identity_kernel);

	apply_filter(channel_image, result_channel_image, IMAGE_WIDTH, IMAGE_HEIGHT,
				 filter);

	assert_memory_equal(channel_image.red, result_channel_image.red,
						IMAGE_WIDTH * IMAGE_HEIGHT);
	assert_memory_equal(channel_image.green, result_channel_image.green,
						IMAGE_WIDTH * IMAGE_HEIGHT);
	assert_memory_equal(channel_image.blue, result_channel_image.blue,
						IMAGE_WIDTH * IMAGE_HEIGHT);

	free_filter(&filter);
	free_image_rgb(&channel_image);
	free_image_rgb(&result_channel_image);
}
