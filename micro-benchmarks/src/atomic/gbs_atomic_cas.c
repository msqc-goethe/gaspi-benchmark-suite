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

	options.type = ATOMIC;
	options.subtype = LAT;

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
			break;
	}

	if (num_pes > 2) {
		fprintf(stderr, "Benchmark requires exactly two processes!\n");
		gaspi_proc_term(GASPI_BLOCK);
		return EXIT_FAILURE;
	}

	const gaspi_segment_id_t segment_id = 0;
	const gaspi_atomic_value_t new_value = 'z';
	gaspi_atomic_value_t old_value;
	gaspi_atomic_value_t comparator = 'a';

	print_header(my_id);

	allocate_gaspi_memory(segment_id, sizeof(char), 'a');
	for (i = 0; i < options.iterations + options.skip; ++i) {
		if (i == options.skip)
			time = stopwatch_start();
		if (my_id == 0) {
			GASPI_CHECK(gaspi_atomic_compare_swap(segment_id,
			                                      0,
			                                      1,
			                                      comparator,
			                                      new_value,
			                                      &old_value,
			                                      GASPI_BLOCK));
		}
	}
	time = stopwatch_stop(time);
	print_atomic_lat(my_id, old_value, new_value, time/1e3);
	free_gaspi_memory(segment_id);

	return EXIT_SUCCESS;
}
