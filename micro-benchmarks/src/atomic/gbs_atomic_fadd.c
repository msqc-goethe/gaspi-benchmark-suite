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

	if (num_pes > 2) {
		fprintf(stderr, "Benchmark requires exactly two processes!\n");
		return EXIT_FAILURE;
	}

	old = malloc(options.iterations * sizeof(char));
	new = malloc(options.iterations * sizeof(char));
	measurements.time = malloc(options.iterations * sizeof(double));
	measurements.n = options.iterations;

	const gaspi_segment_id_t segment_id = 0;
	const gaspi_queue_id_t q_id = 0;
	size_t old_value = 0;
	gaspi_pointer_t ptr;

	print_header(my_id);

	allocate_gaspi_memory_initialized(segment_id, sizeof(size_t));
	GASPI_CHECK(gaspi_segment_ptr(segment_id, &ptr));
	for (i = 0; i < options.iterations + options.skip; ++i) {
		if (my_id == 0) {
			if (i >= options.skip) {
				time = stopwatch_start();
			}
			GASPI_CHECK(
			    gaspi_atomic_fetch_add(segment_id,
			                           0,
			                           1,
			                           1,
			                           (gaspi_atomic_value_t*) &old_value,
			                           GASPI_BLOCK));
			if (i >= options.skip) {
				measurements.time[i - options.skip] = stopwatch_stop(time);
			}
		}
	}

	if (my_id == 0 && options.verify) {
		GASPI_CHECK(
		    gaspi_read(segment_id, 0, 1, segment_id, 0, sizeof(size_t), 0, GASPI_BLOCK));
		GASPI_CHECK(gaspi_wait(0, GASPI_BLOCK));
		size_t expected_counter_val = options.iterations + options.skip;
		size_t actual_counter_val = *((size_t*) ptr);

		if (actual_counter_val != expected_counter_val) {
			fprintf(stderr,
			        "Error: expected result is %d but actual result is %d\n",
			        expected_counter_val,
			        actual_counter_val);
			return EXIT_FAILURE;
		}
	}

	GASPI_CHECK(gaspi_barrier(q_id, GASPI_BLOCK));

	print_atomic_lat(my_id, measurements);
	free_gaspi_memory(segment_id);
	free(measurements.time);
	GASPI_CHECK(gaspi_proc_term(GASPI_BLOCK));
	return EXIT_SUCCESS;
}
