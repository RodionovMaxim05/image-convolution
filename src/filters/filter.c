#include "filter.h"

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

void free_filter(struct filter *f) {
	for (int i = 0; i < f->size; i++) {
		free(f->kernel[i]);
	}
	free(f->kernel);
}

struct filter create_filter(int size, double factor, double bias,
							const double values[size][size]) {
	struct filter f;
	f.size = size;
	f.factor = factor;
	f.bias = bias;

	f.kernel = malloc(size * sizeof(double *));
	if (f.kernel == NULL) {
		struct filter empty = {0, 0.0, 0.0, NULL};
		return empty;
	}

	for (int i = 0; i < size; i++) {
		f.kernel[i] = malloc(size * sizeof(double));
		if (f.kernel[i] == NULL) {
			free_filter(&f);

			struct filter empty = {0, 0.0, 0.0, NULL};
			return empty;
		}
	}

	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			f.kernel[i][j] = values[i][j];
		}
	}

	return f;
}
