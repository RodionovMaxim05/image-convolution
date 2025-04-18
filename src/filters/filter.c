#include "filter.h"

const FilterInfo filters_info[] = {
	{"id", "Identity filter (no effect)."},
	{"fbl", "Fast blur filter (3x3 kernel)."},
	{"bl", "Standard blur filter (5x5 kernel)."},
	{"gbl", "Gaussian blur filter (5x5 kernel)."},
	{"mbl", "Motion blur filter (9x9 kernel)."},
	{"ed", "Edge detection filter (3x3 kernel)."},
	{"em", "Emboss filter (5x5 kernel)."},
	{"bl+gbl",
	 "Composition of Standard blur (5x5) and Gaussian blur (5x5) filters."},
	{"fbl+mbl", "Composition of Fast blur (3x3) and Motion blur (9x9) filters."}};

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

struct filter compose_filters_from_params(int size1, double factor1, double bias1,
										  const double kernel1[size1][size1],
										  int size2, double factor2, double bias2,
										  const double kernel2[size2][size2]) {
	int new_size = size1 + size2 - 1;

	double **kernel = (double **)malloc(new_size * sizeof(double *));
	if (kernel == NULL) {
		struct filter empty = {0, 0.0, 0.0, NULL};
		return empty;
	}

	for (int i = 0; i < new_size; i++) {
		kernel[i] = malloc(new_size * sizeof(double));
		if (kernel[i] == NULL) {
			for (int j = 0; j < i; j++) {
				free(kernel[j]);
			}
			free((void *)kernel);

			struct filter empty = {0, 0.0, 0.0, NULL};
			return empty;
		}

		for (int j = 0; j < new_size; j++) {

			for (int fi = 0; fi < size1; fi++) {
				for (int fj = 0; fj < size1; fj++) {
					int i2 = i - fi;
					int j2 = j - fj;

					if (i2 >= 0 && i2 < size2 && j2 >= 0 && j2 < size2) {
						kernel[i][j] += kernel1[fi][fj] * kernel2[i2][j2];
					}
				}
			}
		}
	}

	double new_factor = factor1 * factor2;
	double new_bias = bias1 * factor2 + bias2;

	struct filter composed = {
		.size = new_size, .factor = new_factor, .bias = new_bias, .kernel = kernel};

	return composed;
}
