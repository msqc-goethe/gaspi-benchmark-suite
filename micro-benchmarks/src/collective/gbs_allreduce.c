#include "check.h"
#include "stopwatch.h"
#include "util.h"
#include "util_memory.h"

int main(int argc, char* argv[]) {
	gaspi_rank_t my_id, num_pes;
	size_t size;
	int i, j;
	int bo_ret = OPTIONS_OKAY;
	float* send_buffer;
	float* recv_buffer;
	double time, min_time, max_time, avg_time;

	options.type = COLLECTIVE;
	options.subtype = ALLREDUCE;

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

	if (num_pes < 2) {
		fprintf(stderr, "Benchmark requires >= 2 processes!\n");
		return EXIT_FAILURE;
	}

	print_header(my_id);

	for (size = options.min_message_size; size <= options.max_message_size;
	     size *= 2) {
		allocate_memory((void**) &send_buffer, size * sizeof(float));
		allocate_memory((void**) &recv_buffer, size * sizeof(float));
		GASPI_CHECK(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK));
		for (i = 0; i < options.iterations + options.skip; ++i) {
			if (i == options.skip) {
				time = stopwatch_start();
			}
			GASPI_CHECK(gaspi_allreduce(send_buffer,
			                            recv_buffer,
			                            size,
			                            GASPI_OP_SUM,
			                            GASPI_TYPE_FLOAT,
			                            GASPI_GROUP_ALL,
			                            GASPI_BLOCK));
		}
		time = stopwatch_stop(time);
		GASPI_CHECK(gaspi_allreduce(&time,
		                            &min_time,
		                            1,
		                            GASPI_OP_MIN,
		                            GASPI_TYPE_DOUBLE,
		                            GASPI_GROUP_ALL,
		                            GASPI_BLOCK));
		GASPI_CHECK(gaspi_allreduce(&time,
		                            &max_time,
		                            1,
		                            GASPI_OP_MAX,
		                            GASPI_TYPE_DOUBLE,
		                            GASPI_GROUP_ALL,
		                            GASPI_BLOCK));
		GASPI_CHECK(gaspi_allreduce(&time,
		                            &avg_time,
		                            1,
		                            GASPI_OP_SUM,
		                            GASPI_TYPE_DOUBLE,
		                            GASPI_GROUP_ALL,
		                            GASPI_BLOCK));
		min_time /= options.iterations;
		avg_time /= options.iterations * num_pes;
		max_time /= options.iterations;
		print_result_coll(my_id, num_pes, size, min_time, avg_time, max_time);
		free_memory(send_buffer);
		free_memory(recv_buffer);
	}
	return EXIT_SUCCESS;
}