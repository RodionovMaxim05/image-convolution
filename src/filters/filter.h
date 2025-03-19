#pragma once

#include <stdlib.h>

struct filter {
	int size;
	double factor;
	double bias;
	double **kernel;
};

extern const double fast_blur[3][3];
extern const double blur[5][5];
extern const double motion_blur[9][9];

struct filter create_filter(int size, double factor, double bias,
							const double values[size][size]);

void free_filter(struct filter *f);
