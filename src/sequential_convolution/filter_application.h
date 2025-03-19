#include <math.h>

#include "../filters/filter.h"
#include "../utils/utils.h"

void apply_filter(struct image_rgb image, struct image_rgb result_image,
				  int width, int height, struct filter filter);
