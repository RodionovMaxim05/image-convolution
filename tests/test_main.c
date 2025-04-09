#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "parallel_tests.c"
#include "sequential_tests.c"
#include "unit_tests.c"

int main(void) {
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_create_filter),
		cmocka_unit_test(test_split_assemble_channels),
		cmocka_unit_test(test_identity_filter),

		cmocka_unit_test(test_filter_inverse_with_random_image),
		cmocka_unit_test(test_filter_inverse_with_default_image),
		cmocka_unit_test(test_filter_zero_padding_with_random_image),
		cmocka_unit_test(test_filter_zero_padding_with_default_image),

		cmocka_unit_test(test_parallel_pixel_with_default_image),
		cmocka_unit_test(test_parallel_pixel_with_random_image),
		cmocka_unit_test(test_parallel_row_with_default_image),
		cmocka_unit_test(test_parallel_row_with_random_image),
		cmocka_unit_test(test_parallel_column_with_default_image),
		cmocka_unit_test(test_parallel_column_with_random_image),
		cmocka_unit_test(test_parallel_block_with_default_image),
		cmocka_unit_test(test_parallel_block_with_random_image),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
