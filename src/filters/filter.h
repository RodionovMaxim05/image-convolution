#pragma once

#include <stdlib.h>

#define ID_SIZE 3
#define ID_FACTOR 1.0
#define ID_BIAS 0.0

#define FAST_BLUR_SIZE 3
#define FAST_BLUR_FACTOR 1.0
#define FAST_BLUR_BIAS 0.0

#define BLUR_SIZE 5
#define BLUR_FACTOR (1.0 / 13.0)
#define BLUR_BIAS 0.0

#define GAUS_BLUR_SIZE 5
#define GAUS_BLUR_FACTOR (1.0 / 256.0)
#define GAUS_BLUR_BIAS 0.0

#define MOTION_BLUR_SIZE 9
#define MOTION_BLUR_FACTOR (1.0 / 9.0)
#define MOTION_BLUR_BIAS 0.0

#define EDGE_DETECTION_SIZE 3
#define EDGE_DETECTION_FACTOR 1.0
#define EDGE_DETECTION_BIAS 0.0

#define EMBOSS_SIZE 5
#define EMBOSS_FACTOR 1.0
#define EMBOSS_BIAS 128.0

#define NUM_OF_FILTERS 7

typedef struct {
	const char *name;
	const char *description;
} FilterInfo;

extern const FilterInfo filters_info[];

struct filter {
	int size;
	double factor;
	double bias;
	double **kernel;
};

extern const double id[3][3];
extern const double fast_blur[3][3];
extern const double blur[5][5];
extern const double gaus_blur[5][5];
extern const double motion_blur[9][9];
extern const double edge_detection[3][3];
extern const double emboss[5][5];

struct filter create_filter(int size, double factor, double bias,
							const double values[size][size]);

void free_filter(struct filter *f);
