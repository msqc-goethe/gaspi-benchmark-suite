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

	options.type = PASSIVE;
	options.subtype = BW;
	options.name = "gbs_passive_bw";

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

	if (num_pes > 2) {
		fprintf(stderr, "Benchmark requires exactly two processes!\n");
		return EXIT_FAILURE;
	}

	measurements.time = malloc(options.iterations * sizeof(double));
	measurements.n = options.iterations;

	const gaspi_segment_id_t segment_id = 0;
	gaspi_rank_t remote_id;

	print_header(my_id);

	int window_size = options.window_size;
	for (size = options.min_message_size; size <= options.max_message_size;
	     size *= 2) {
		allocate_gaspi_memory(
		    segment_id, size * window_size * sizeof(char), 'a');
		for (i = 0; i < options.iterations + options.skip; ++i) {
			if (i >= options.skip) {
				time = stopwatch_start();
			}
			if (my_id == 0) {
				for (j = 0; j < window_size; ++j) {
					GASPI_CHECK(gaspi_passive_send(
					    segment_id, j * size, 1, size, GASPI_BLOCK));
				}
				GASPI_CHECK(gaspi_passive_receive(
				    segment_id, 0, &remote_id, 1, GASPI_BLOCK));
			}
			else if (my_id == 1) {
				for (j = 0; j < window_size; ++j) {
					GASPI_CHECK(gaspi_passive_receive(
					    segment_id, j * size, &remote_id, size, GASPI_BLOCK));
				}
				GASPI_CHECK(
				    gaspi_passive_send(segment_id, 0, 0, 1, GASPI_BLOCK));
			}
			if (i >= options.skip) {
				measurements.time[i - options.skip] = stopwatch_stop(time);
			}
		}
		print_result(my_id, measurements, size);
		free_gaspi_memory(segment_id);
	}
	free(measurements.time);
	return EXIT_SUCCESS;
}
