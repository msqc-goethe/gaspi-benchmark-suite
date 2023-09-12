#include "check.h"
#include "stopwatch.h"
#include "util.h"
#include "util_memory.h"

int main(int argc, char* argv[]) {
	gaspi_rank_t my_id, num_pes;
	int i;
	int bo_ret = OPTIONS_OKAY;
	struct measurements_t measurements;
	double time;

	options.type = COLLECTIVE;
	options.subtype = BARRIER;
	options.name = "gbs_barrier";

	bo_ret = benchmark_options(argc, argv);

	switch (bo_ret) {
		case OPTIONS_BAD_USAGE:
			print_bad_usage();
			return EXIT_FAILURE;
		case OPTIONS_HELP:
			print_help_message();
			return EXIT_SUCCESS;
	}

	GASPI_CHECK(gaspi_proc_init(GASPI_BLOCK));
	GASPI_CHECK(gaspi_proc_rank(&my_id));
	GASPI_CHECK(gaspi_proc_num(&num_pes));

	if (num_pes < 2) {
		fprintf(stderr, "Benchmark requires >= 2 processes!\n");
		return EXIT_FAILURE;
	}

	measurements.time = malloc(options.iterations * sizeof(double));
	measurements.n = options.iterations;

	print_header(my_id);

	for (i = 0; i < options.iterations + options.skip; ++i) {
		if (i >= options.skip) {
			time = stopwatch_start();
		}
		GASPI_CHECK(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK));
		if (i >= options.skip) {
			measurements.time[i - options.skip] = stopwatch_stop(time);
		}
	}
	print_result_coll(my_id, num_pes, 0, measurements);
	free(measurements.time);
	GASPI_CHECK(gaspi_proc_term(GASPI_BLOCK));
	return EXIT_SUCCESS;
}
