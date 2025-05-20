#include "args.h"

#define MODE_PREFIX_LEN 7
#define THREAD_PREFIX_LEN 9
#define QUEUE_ARGS_PREFIX_LEN 10
#define NUM_OF_IMAGES_PREFIX_LEN 6
#define MAX_PATH_LEN 264
#define NUM_OF_ARGS_FOR_QUEUE_MOD 5
#define INITIAL_INDEX_FOR_QUEUE_MOD 5
#define CHECK_NUMBER(num, str)                                                      \
	if ((num) < 1) {                                                                \
		error("Invalid number of %s, required number > 0.\n", str);                 \
		return false;                                                               \
	}

bool parse_args(int argc, char *argv[], program_args *args) {
	char *queue_options =
		"Queue options:\n"
		"  --num=<images>         Number of images to process.\n"
		"  --readers=<num>        Number of reader threads.\n"
		"  --workers=<num>        Number of worker threads.\n"
		"  --writers=<num>        Number of writer threads.\n"
		"  --mem_lim=<MiB>        Memory limit for queues in MiB (e.g., 10).\n\n";

	if (argc < 4) {
		error(
			"Usage:\n"
			"  %s <image_path | --default-image> <filter_name> --mode=seq\n"
			"  %s <image_path | --default-image> <filter_name> "
			"--mode=<parallel_mode> --thread=<num>\n"
			"  %s <image_path | --default-image> <filter_name> --mode=queue "
			"--thread=<num> \\\n"
			"        --num=<images> --readers=<num> --workers=<num> "
			"--writers=<num> --mem_lim=<MiB>\n\n"

			"Options:\n"
			"  <image_path>           Path to the input image file.\n"
			"  --default-image        Use a predefined default image.\n"
			"  <filter_name>          Name of the filter to apply ().\n"
			"  --mode=<mode>          Execution mode:\n"
			"                         'seq'     - sequential processing,\n"
			"                         'row'     - parallel by rows,\n"
			"                         'column'  - parallel by columns,\n"
			"                         'block'   - parallel by blocks,\n"
			"                         'pixel'   - parallel by pixels,\n"
			"                         'queue'   - queue-based parallel processing.\n"
			"  --thread=<num>         Number of threads to use for parallel "
			"convolution.\n"
			"                         (Ignored if --mode=seq)\n\n",
			argv[0], argv[0], argv[0]);
		error("%s", queue_options);
		error("Available Filters:\n");
		for (int i = 0; i < NUM_OF_FILTERS; i++) {
			error("  %-22s %s\n", filters_info[i].name, filters_info[i].description);
		}
		return false;
	}

	args->img_path =
		strcmp(argv[1], "--default-image") == 0 ? "images/cat.bmp" : argv[1];
	args->filter_name = argv[2];

	if (strncmp(argv[3], "--mode=", MODE_PREFIX_LEN) != 0) {
		error("Missing --mode argument\n");
		return false;
	}
	args->mode = argv[3] + MODE_PREFIX_LEN;

	int res_int = 0;

	if (strcmp(args->mode, "seq") != 0) {
		if (strncmp(argv[4], "--thread=", THREAD_PREFIX_LEN) != 0) {
			error("Missing --thread argument\n");
			return false;
		}
		res_int = atoi(argv[4] + THREAD_PREFIX_LEN);
		CHECK_NUMBER(res_int, "threads")
		args->threads_num = res_int;
	}

	if (strcmp(args->mode, "queue") == 0) {
		if (argc < INITIAL_INDEX_FOR_QUEUE_MOD + NUM_OF_ARGS_FOR_QUEUE_MOD) {
			error("Missing queue mode parameters.\n\n");
			error("%s", queue_options);
			return false;
		}

		for (uint8_t i = INITIAL_INDEX_FOR_QUEUE_MOD;
			 i < INITIAL_INDEX_FOR_QUEUE_MOD + NUM_OF_ARGS_FOR_QUEUE_MOD; i++) {
			if (strncmp(argv[i], "--num=", NUM_OF_IMAGES_PREFIX_LEN) == 0) {
				res_int = atoi(argv[i] + NUM_OF_IMAGES_PREFIX_LEN);
				CHECK_NUMBER(res_int, "images")
				args->img_count = res_int;

			} else if (strncmp(argv[i], "--readers=", QUEUE_ARGS_PREFIX_LEN) == 0) {
				res_int = atoi(argv[i] + QUEUE_ARGS_PREFIX_LEN);
				CHECK_NUMBER(res_int, "reader threads")
				args->readers_num = res_int;

			} else if (strncmp(argv[i], "--workers=", QUEUE_ARGS_PREFIX_LEN) == 0) {
				res_int = atoi(argv[i] + QUEUE_ARGS_PREFIX_LEN);
				CHECK_NUMBER(res_int, "worker threads")
				args->workers_num = res_int;

			} else if (strncmp(argv[i], "--writers=", QUEUE_ARGS_PREFIX_LEN) == 0) {
				res_int = atoi(argv[i] + QUEUE_ARGS_PREFIX_LEN);
				CHECK_NUMBER(res_int, "writer threads")
				args->writers_num = res_int;

			} else if (strncmp(argv[i], "--mem_lim=", QUEUE_ARGS_PREFIX_LEN) == 0) {
				res_int = atoi(argv[i] + QUEUE_ARGS_PREFIX_LEN);
				CHECK_NUMBER(res_int, "memory limit")
				args->memory_lim = (size_t)res_int * (size_t)BYTES_IN_MEBIBYTE;

			} else {
				error("Invalid argument '%s' for queue mode.\n", argv[i]);

				return false;
			}
		}
	}

	return true;
}

const char *extract_filename(const char *path) {
	const char *last_slash = strrchr(path, '/');
	if (last_slash == NULL) {
		return path;
	}

	return last_slash + 1;
}

char **get_file_paths(const char *dir_path, size_t file_count) {
	DIR *dir = opendir(dir_path);
	if (!dir) {
		error("Failed to open directory '%s'.\n", dir_path);
		return NULL;
	}

	char **file_paths = (char **)malloc(file_count * sizeof(char *));
	if (!file_paths) {
		error("Memory allocation error for file_paths.\n");
		closedir(dir);
		return NULL;
	}

	struct dirent *entry;
	size_t index = 0;

	while ((entry = readdir(dir)) && index < file_count) {
		if (strstr(entry->d_name, ".bmp")) {
			file_paths[index] = malloc(MAX_PATH_LEN);
			if (!file_paths[index]) {
				for (size_t i = 0; i < index; i++) {
					free(file_paths[i]);
				}
				free((void *)file_paths);
				closedir(dir);
				return NULL;
			}

			snprintf(file_paths[index], MAX_PATH_LEN, "%s/%s", dir_path,
					 entry->d_name);

			index++;
		}
	}

	closedir(dir);
	return file_paths;
}

void free_file_paths(char **file_paths, size_t file_count) {
	for (size_t i = 0; i < file_count; i++) {
		free(file_paths[i]);
	}
	free((void *)file_paths);
}
