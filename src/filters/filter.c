#include "filter.h"

const FilterInfo filters_info[] = {{"id", "Identity filter (no effect)."},
								   {"fbl", "Fast blur filter (3x3 kernel)."},
								   {"bl", "Standard blur filter (5x5 kernel)."},
								   {"gbl", "Gaussian blur filter (5x5 kernel)."},
								   {"mbl", "Motion blur filter (9x9 kernel)."},
								   {"ed", "Edge detection filter (3x3 kernel)."},
								   {"em", "Emboss filter (5x5 kernel)."}};

const double id[3][3] = {{0.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 0.0}};

const double fast_blur[3][3] = {{0.0, 0.2, 0.0}, {0.2, 0.2, 0.2}, {0.0, 0.2, 0.0}};

const double blur[5][5] = {{0, 0, 1, 0, 0},
						   {0, 1, 1, 1, 0},
						   {1, 1, 1, 1, 1},
						   {0, 1, 1, 1, 0},
						   {0, 0, 1, 0, 0}};

const double gaus_blur[5][5] = {{1, 4, 6, 4, 1},
								{4, 16, 24, 16, 4},
								{6, 24, 36, 24, 6},
								{4, 16, 24, 16, 4},
								{1, 4, 6, 4, 1}};

const double motion_blur[9][9] = {
	{1, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 1, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 1, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 1, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 1, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 1, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 1, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 1},
};

const double edge_detection[3][3] = {{-1, -1, -1}, {-1, 8, -1}, {-1, -1, -1}};

const double emboss[5][5] = {{-1, -1, -1, -1, 0},
							 {-1, -1, -1, 0, 1},
							 {-1, -1, 0, 1, 1},
							 {-1, 0, 1, 1, 1},
							 {0, 1, 1, 1, 1}};

struct filter create_filter(int size, double factor, double bias,
							const double values[size][size]) {
	struct filter filter;
	filter.size = size;
	filter.factor = factor;
	filter.bias = bias;

	filter.kernel = (double **)malloc(size * sizeof(double *));
	if (filter.kernel == NULL) {
		struct filter empty = {0, 0.0, 0.0, NULL};
		return empty;
	}

	for (int i = 0; i < size; i++) {
		filter.kernel[i] = malloc(size * sizeof(double));
		if (filter.kernel[i] == NULL) {
			free_filter(&filter);

			struct filter empty = {0, 0.0, 0.0, NULL};
			return empty;
		}
	}

	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			filter.kernel[i][j] = values[i][j];
		}
	}

	return filter;
}

void free_filter(struct filter *filter) {
	for (int i = 0; i < filter->size; i++) {
		free(filter->kernel[i]);
	}
	free((void *)filter->kernel);
}
