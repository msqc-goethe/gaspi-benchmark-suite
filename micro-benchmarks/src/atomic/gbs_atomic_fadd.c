#include "check.h"
#include "stopwatch.h"
#include "util.h"
#include "util_memory.h"

int main(int argc, char* argv[]) {
	gaspi_rank_t my_id, num_pes;
	size_t size;
	int i, j;
	int bo_ret = OPTIONS_OKAY;
	struct measurements_t measurements;
	double time;
	char *old, *new;
	char* segment_ptr;

	options.type = ATOMIC;
	options.subtype = LAT;
	options.name = "gbs_atomic_fadd";

	GASPI_CHECK(gaspi_proc_init(GASPI_BLOCK));
	GASPI_CHECK(gaspi_proc_rank(&my_id));
	GASPI_CHECK(gaspi_proc_num(&num_pes));

	bo_ret = benchmark_options(argc, argv);

	switch (bo_ret) {
		case OPTIONS_BAD_USAGE:
			print_bad_usage(my_id);
			break;
		case OPTIONS_HELP:
			print_help_message(my_id);
			return EXIT_SUCCESS;
	}

	if (num_pes > 2) {
		fprintf(stderr, "Benchmark requires exactly two processes!\n");
		return EXIT_FAILURE;
	}

	old = malloc(options.iterations * sizeof(char));
	new = malloc(options.iterations * sizeof(char));
	measurements.time = malloc(options.iterations * sizeof(double));
	measurements.n = options.iterations;

	const gaspi_segment_id_t segment_id = 0;
	gaspi_atomic_value_t old_value;

	print_header(my_id);

	allocate_gaspi_memory(segment_id, sizeof(char), 'a');
	GASPI_CHECK(gaspi_segment_ptr(segment_id, (void**) &segment_ptr));
	for (i = 0; i < options.iterations + options.skip; ++i) {
		if (my_id == 0) {
			if (i >= options.skip) {
				time = stopwatch_start();
			}
			GASPI_CHECK(gaspi_atomic_fetch_add(
			    segment_id, 0, 1, 1, &old_value, GASPI_BLOCK));
			if (i >= options.skip) {
				measurements.time[i - options.skip] = stopwatch_stop(time);
				old[i - options.skip] = old_value;
				GASPI_CHECK(gaspi_read(
				    segment_id, 0, 1, segment_id, 0, 1, 0, GASPI_BLOCK));
				GASPI_CHECK(gaspi_wait(0, GASPI_BLOCK));
				new[i - options.skip] = *segment_ptr;
			}
		}
	}
	print_atomic_lat(my_id, old, new, measurements);
	free_gaspi_memory(segment_id);
	free(measurements.time);
	free(old);
	free(new);
	return EXIT_SUCCESS;
}
