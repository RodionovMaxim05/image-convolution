#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>

#include "unit_tests.c"

int main(void) {
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_create_filter),
		cmocka_unit_test(test_split_assemble_channels),
		cmocka_unit_test(test_identity_filter),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
