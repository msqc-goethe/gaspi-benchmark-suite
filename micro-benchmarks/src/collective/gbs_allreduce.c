#include "check.h"
#include "stopwatch.h"
#include "util.h"
#include "util_memory.h"

int main(int argc, char* argv[]) {
	gaspi_rank_t my_id, num_pes;
	gaspi_number_t max_elem;
	size_t size;
	int i, j;
	int bo_ret = OPTIONS_OKAY;
	float* send_buffer;
	float* recv_buffer;
	struct measurements_t measurements;
	double time;

	options.type = COLLECTIVE;
	options.subtype = ALLREDUCE;
	options.name = "gbs_allreduce";

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

	GASPI_CHECK(
	    gaspi_allreduce_elem_max((gaspi_number_t*) options.max_message_size));

	if (num_pes < 2) {
		fprintf(stderr, "Benchmark requires >= 2 processes!\n");
		return EXIT_FAILURE;
	}

	measurements.time = malloc(options.iterations * sizeof(double));
	measurements.n = options.iterations;

	print_header(my_id);

	GASPI_CHECK(gaspi_allreduce_elem_max(&max_elem));

	for (size = options.min_message_size; size <= options.max_message_size;
	     size *= 2) {
		if (size >= max_elem) {
			fprintf(stderr,
			        "%d elements exceede limit of %d for gaspi_allreduce!\n",
			        size,
			        max_elem);
			return EXIT_FAILURE;
		}
		allocate_memory((void**) &send_buffer, size * sizeof(float));
		allocate_memory((void**) &recv_buffer, size * sizeof(float));

		GASPI_CHECK(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK));

		for (i = 0; i < options.iterations + options.skip; ++i) {
			if (i >= options.skip) {
				time = stopwatch_start();
			}
			GASPI_CHECK(gaspi_allreduce(send_buffer,
			                            recv_buffer,
			                            size,
			                            GASPI_OP_SUM,
			                            GASPI_TYPE_FLOAT,
			                            GASPI_GROUP_ALL,
			                            GASPI_BLOCK));
			if (i >= options.skip) {
				measurements.time[i - options.skip] = stopwatch_stop(time);
			}
		}
		print_result_coll(my_id, num_pes, size, measurements);
		free_memory(send_buffer);
		free_memory(recv_buffer);
	}
	free(measurements.time);
	return EXIT_SUCCESS;
}
