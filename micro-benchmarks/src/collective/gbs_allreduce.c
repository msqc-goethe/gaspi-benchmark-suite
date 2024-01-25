#include "check.h"
#include "stopwatch.h"
#include "util.h"
#include "util_memory.h"

int main(int argc, char* argv[]) {
	gaspi_rank_t my_id, num_pes;
	gaspi_number_t max_elem;
	size_t size;
	int i;
	int bo_ret = OPTIONS_OKAY;
	float* send_buffer;
	float* recv_buffer;
	double timer = 0;
	double t0;

	options.type = COLLECTIVE;
	options.subtype = ALLREDUCE;
	options.name = "gbs_allreduce";

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

	print_header(my_id);

	GASPI_CHECK(gaspi_allreduce_elem_max(&max_elem));
	if (options.max_message_size > max_elem) {
		if (my_id == 0) {
			fprintf(stdout,
			        "limit for allreduce is %d requested was %ld\n",
			        max_elem,
			        options.max_message_size);
		}
		options.max_message_size = max_elem;
	}

	if (options.single_buffer) {
		allocate_memory((void**) &send_buffer,
		                options.max_message_size * sizeof(float));
		allocate_memory((void**) &recv_buffer,
		                options.max_message_size * sizeof(float));
		for (size = options.min_message_size; size <= options.max_message_size;
		     size *= 2) {
			GASPI_CHECK(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK));

			for (i = 0; i < options.iterations + options.skip; ++i) {
				if (i >= options.skip) {
					t0 = stopwatch_start();
				}
				GASPI_CHECK(gaspi_allreduce(send_buffer,
				                            recv_buffer,
				                            size,
				                            GASPI_OP_SUM,
				                            GASPI_TYPE_FLOAT,
				                            GASPI_GROUP_ALL,
				                            GASPI_BLOCK));
				if (i >= options.skip) {
					timer += stopwatch_stop(t0);
				}
			}
			double latency = timer / options.iterations;
			double min_time, max_time, avg_time;

			GASPI_CHECK(gaspi_allreduce(&latency,
			                            &min_time,
			                            1,
			                            GASPI_OP_MIN,
			                            GASPI_TYPE_DOUBLE,
			                            GASPI_GROUP_ALL,
			                            GASPI_BLOCK));
			GASPI_CHECK(gaspi_allreduce(&latency,
			                            &max_time,
			                            1,
			                            GASPI_OP_MAX,
			                            GASPI_TYPE_DOUBLE,
			                            GASPI_GROUP_ALL,
			                            GASPI_BLOCK));
			GASPI_CHECK(gaspi_allreduce(&latency,
			                            &avg_time,
			                            1,
			                            GASPI_OP_SUM,
			                            GASPI_TYPE_DOUBLE,
			                            GASPI_GROUP_ALL,
			                            GASPI_BLOCK));

			avg_time /= num_pes;
			avg_time *= 1e-3; //us
			min_time *= 1e-3; //us
			max_time *= 1e-3; //us

			print_allreduce_result(
			    my_id, num_pes, size, min_time, max_time, avg_time);
		}
		free_memory(send_buffer);
		free_memory(recv_buffer);
	}
	else {
		for (size = options.min_message_size; size <= options.max_message_size;
		     size *= 2) {
			allocate_memory((void**) &send_buffer, size * sizeof(float));
			allocate_memory((void**) &recv_buffer, size * sizeof(float));

			GASPI_CHECK(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK));

			for (i = 0; i < options.iterations + options.skip; ++i) {
				if (i >= options.skip) {
					t0 = stopwatch_start();
				}
				GASPI_CHECK(gaspi_allreduce(send_buffer,
				                            recv_buffer,
				                            size,
				                            GASPI_OP_SUM,
				                            GASPI_TYPE_FLOAT,
				                            GASPI_GROUP_ALL,
				                            GASPI_BLOCK));
				if (i >= options.skip) {
					timer += stopwatch_stop(t0);
				}
			}
			double latency = timer / options.iterations;
			double min_time, max_time, avg_time;

			GASPI_CHECK(gaspi_allreduce(&latency,
			                            &min_time,
			                            1,
			                            GASPI_OP_MIN,
			                            GASPI_TYPE_DOUBLE,
			                            GASPI_GROUP_ALL,
			                            GASPI_BLOCK));
			GASPI_CHECK(gaspi_allreduce(&latency,
			                            &max_time,
			                            1,
			                            GASPI_OP_MAX,
			                            GASPI_TYPE_DOUBLE,
			                            GASPI_GROUP_ALL,
			                            GASPI_BLOCK));
			GASPI_CHECK(gaspi_allreduce(&latency,
			                            &avg_time,
			                            1,
			                            GASPI_OP_SUM,
			                            GASPI_TYPE_DOUBLE,
			                            GASPI_GROUP_ALL,
			                            GASPI_BLOCK));

			avg_time /= num_pes;
			avg_time *= 1e-3; //us
			min_time *= 1e-3; //us
			max_time *= 1e-3; //us

			print_allreduce_result(
			    my_id, num_pes, size, min_time, max_time, avg_time);
			free_memory(send_buffer);
			free_memory(recv_buffer);
		}
	}
	GASPI_CHECK(gaspi_proc_term(GASPI_BLOCK));
	return EXIT_SUCCESS;
}
