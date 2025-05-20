#pragma once

#include "../filters/filter.h"
#include "utils.h"
#include <dirent.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define MAX_PATH_LEN 264
#define QUEUE_DIR_NAME "output_queue_mode"

/**
 * A structure to store the parsed command-line arguments.
 *
 * @param image_path Path to the input image file or "images/cat.bmp" if
 * --default-image is specified.
 * @param filter_name Name of the filter to apply.
 * @param mode Execution mode ("seq", "row", "column", "block", "pixel" or "queue").
 * @param threads_num Number of threads to use for parallel convolution (ignored for
 * "seq").
 * @param img_count Number of images to process in "queue" mode.
 * @param readers_num Number of reader threads in "queue" mode.
 * @param workers_num Number of worker threads in "queue" mode.
 * @param writers_num Number of writer threads in "queue" mode.
 * @param memory_lim Memory limit for queues in bytes (converted from MiB) in "queue"
 * mode.
 */
typedef struct {
	const char *img_path;
	const char *filter_name;
	const char *mode;
	int threads_num;

	uint8_t img_count;
	uint8_t readers_num;
	uint8_t workers_num;
	uint8_t writers_num;
	size_t memory_lim;
} program_args;

/**
 * Parses the command-line arguments provided to the program. It validates the input
 * and populates the `ProgramArgs` structure with the parsed values.
 *
 * @param argc The number of command-line arguments.
 * @param argv An array of strings containing the command-line arguments.
 * @param args A pointer to a `ProgramArgs` structure where the parsed arguments will
 * be stored.
 *
 * @return `true` if parsing is successful, `false` if there is an error.
 */
bool parse_args(int argc, char *argv[], program_args *args);

/**
 * Extracts the filename from a given file path.
 *
 * @param path The full file path.
 *
 * @return A pointer to the start of the filename in the path. If no / is found, the
 * entire path is returned.
 */
const char *extract_filename(const char *path);

/**
 * Scans the specified directory and collects paths of `.bmp` files up to the
 * specified count.
 *
 * @param dir_path Path to the directory to scan.
 * @param file_count Maximum number of file paths to return.
 *
 * @return An allocated array of file paths, or `NULL` on failure.
 */
char **get_file_paths(const char *dir_path, size_t file_count);

/**
 * Deallocates each individual file path and the array itself.
 *
 * @param file_paths The array of file paths to free.
 * @param file_count Number of elements in the array.
 */
void free_file_paths(char **file_paths, size_t file_count);
