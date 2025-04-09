#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "parallel_tests.c"
#include "sequential_tests.c"
#include "unit_tests.c"

/**
 * The main function initializes the random number generator using the current time
 * and runs all test groups sequentially.
 */
int main(void) {
	srand((unsigned int)time(NULL));

	const struct CMUnitTest core_tests[] = {
		cmocka_unit_test(test_create_filter),
		cmocka_unit_test(test_split_assemble_channels),
		cmocka_unit_test(test_identity_filter),
	};

	const struct CMUnitTest sequential_tests[] = {
		cmocka_unit_test(test_filter_inverse_with_random_image),
		cmocka_unit_test(test_filter_inverse_with_default_image),
		cmocka_unit_test(test_filter_zero_padding_with_random_image),
		cmocka_unit_test(test_filter_zero_padding_with_default_image),
	};

	const struct CMUnitTest parallel_tests[] = {
		cmocka_unit_test(test_parallel_pixel_with_default_image),
		cmocka_unit_test(test_parallel_pixel_with_random_image),
		cmocka_unit_test(test_parallel_row_with_default_image),
		cmocka_unit_test(test_parallel_row_with_random_image),
		cmocka_unit_test(test_parallel_column_with_default_image),
		cmocka_unit_test(test_parallel_column_with_random_image),
		cmocka_unit_test(test_parallel_block_with_default_image),
		cmocka_unit_test(test_parallel_block_with_random_image),
	};

	int status = 0;
	status |= cmocka_run_group_tests_name("Core Functionality Tests", core_tests,
										  NULL, NULL);
	status |= cmocka_run_group_tests_name("Sequential Application Tests",
										  sequential_tests, NULL, NULL);
	status |= cmocka_run_group_tests_name("Parallel Application Tests",
										  parallel_tests, NULL, NULL);

	return status;
}
