#include "check.h"
#include "stopwatch.h"
#include "util.h"
#include "util_memory.h"

int main(int argc, char* argv[]) {
	gaspi_rank_t my_id, num_pes;
	size_t size;
	int i, j;
	int bo_ret = OPTIONS_OKAY;
	double time;
	struct measurements_t measurements;

	options.type = ONESIDED;
	options.subtype = BW;
	options.name = "gbs_read_bw";

	bo_ret = benchmark_options(argc, argv);

	switch (bo_ret) {
		case OPTIONS_BAD_USAGE:
			print_bad_usage();
			break;
		case OPTIONS_HELP:
			print_help_message();
			return EXIT_SUCCESS;
	}

	GASPI_CHECK(gaspi_proc_init(GASPI_BLOCK));
	GASPI_CHECK(gaspi_proc_rank(&my_id));
	GASPI_CHECK(gaspi_proc_num(&num_pes));

	measurements.time = malloc(options.iterations * sizeof(double));
	measurements.n = options.iterations;

	if (num_pes > 2) {
		fprintf(stderr, "Benchmark requires exactly two processes!\n");
		return EXIT_FAILURE;
	}

	const gaspi_segment_id_t segment_id = 0;
	const gaspi_queue_id_t q_id = 0;

	print_header(my_id);

	int window_size = options.window_size;
	for (size = options.min_message_size; size <= options.max_message_size;
	     size *= 2) {
		allocate_gaspi_memory(
		    segment_id, size * window_size * sizeof(char), 'a');
		if (my_id == 0) {
			for (i = 0; i < options.iterations + options.skip; ++i) {
				if (i >= options.skip) {
					time = stopwatch_start();
				}
				for (j = 0; j < window_size; ++j) {
					GASPI_CHECK(gaspi_read(segment_id,
					                       j * size,
					                       1,
					                       segment_id,
					                       j * size,
					                       size,
					                       q_id,
					                       GASPI_BLOCK));
				}
				GASPI_CHECK(gaspi_wait(q_id, GASPI_BLOCK));
				if (i >= options.skip) {
					measurements.time[i - options.skip] = stopwatch_stop(time);
				}
			}
		}
		print_result(my_id, measurements, size);
		free_gaspi_memory(segment_id);
	}
	free(measurements.time);
	return EXIT_SUCCESS;
}
