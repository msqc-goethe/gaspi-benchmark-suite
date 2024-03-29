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
	char check_val;
	struct measurements_t measurements;

	options.type = ONESIDED;
	options.subtype = BW;
	options.name = "gbs_write_bibw";

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

	measurements.time = malloc(options.iterations * sizeof(double));
	measurements.n = options.iterations;

	const gaspi_segment_id_t segment_id_send = 0;
	const gaspi_segment_id_t segment_id_recv = 1;
	const gaspi_queue_id_t q_id = 0;
	gaspi_pointer_t ptr;

	print_header(my_id);

	int window_size = options.window_size;
	if (options.single_buffer) {
		allocate_gaspi_memory(segment_id_send,
		                      options.max_message_size * sizeof(char),
		                      my_id == 0 ? 'a' : 'b');
		allocate_gaspi_memory(
		    segment_id_recv, options.max_message_size * sizeof(char), 'y');
		GASPI_CHECK(gaspi_segment_ptr(segment_id_recv, &ptr));

		for (size = options.min_message_size; size <= options.max_message_size;
		     size *= 2) {
			GASPI_CHECK(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK));
			if (my_id == 0) {
				for (i = 0; i < options.iterations + options.skip; ++i) {
					if (i >= options.skip) {
						time = stopwatch_start();
					}
					for (j = 0; j < window_size; ++j) {
						GASPI_CHECK(gaspi_write(segment_id_send,
						                        0,
						                        1,
						                        segment_id_recv,
						                        0,
						                        size,
						                        q_id,
						                        GASPI_BLOCK));
					}
					GASPI_CHECK(gaspi_wait(q_id, GASPI_BLOCK));
					GASPI_CHECK(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK));
					if (i >= options.skip) {
						measurements.time[i - options.skip] =
						    stopwatch_stop(time);
					}
				}
			}
			else if (my_id == 1) {
				for (i = 0; i < options.iterations + options.skip; ++i) {
					for (j = 0; j < window_size; ++j) {
						GASPI_CHECK(gaspi_write(segment_id_send,
						                        0,
						                        0,
						                        segment_id_recv,
						                        0,
						                        size,
						                        q_id,
						                        GASPI_BLOCK));
					}
					GASPI_CHECK(gaspi_wait(q_id, GASPI_BLOCK));
					GASPI_CHECK(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK));
				}
			}
			GASPI_CHECK(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK));
			if (options.verify) {
				check_val = my_id == 0 ? 'b' : 'a';
				for (i = 0; i < size; ++i) {
					if (((char*) ptr)[i] != check_val) {
						fprintf(stderr,
						        "Verification failed. Result is invalid!\n");
						return EXIT_FAILURE;
					}
				}
			}
			print_result(my_id, measurements, size * 2);
		}
		free_gaspi_memory(segment_id_send);
		free_gaspi_memory(segment_id_recv);
	}
	else {
		for (size = options.min_message_size; size <= options.max_message_size;
		     size *= 2) {
			allocate_gaspi_memory(segment_id_send,
			                      size * window_size * sizeof(char),
			                      my_id == 0 ? 'a' : 'b');
			allocate_gaspi_memory(
			    segment_id_recv, size * window_size * sizeof(char), 'y');
			GASPI_CHECK(gaspi_segment_ptr(segment_id_recv, &ptr));

			GASPI_CHECK(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK));
			if (my_id == 0) {
				for (i = 0; i < options.iterations + options.skip; ++i) {
					if (i >= options.skip) {
						time = stopwatch_start();
					}
					for (j = 0; j < window_size; ++j) {
						GASPI_CHECK(gaspi_write(segment_id_send,
						                        j * size,
						                        1,
						                        segment_id_recv,
						                        j * size,
						                        size,
						                        q_id,
						                        GASPI_BLOCK));
					}
					GASPI_CHECK(gaspi_wait(q_id, GASPI_BLOCK));
					GASPI_CHECK(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK));
					if (i >= options.skip) {
						measurements.time[i - options.skip] =
						    stopwatch_stop(time);
					}
				}
			}
			else if (my_id == 1) {
				for (i = 0; i < options.iterations + options.skip; ++i) {
					for (j = 0; j < window_size; ++j) {
						GASPI_CHECK(gaspi_write(segment_id_send,
						                        j * size,
						                        0,
						                        segment_id_recv,
						                        j * size,
						                        size,
						                        q_id,
						                        GASPI_BLOCK));
					}
					GASPI_CHECK(gaspi_wait(q_id, GASPI_BLOCK));
					GASPI_CHECK(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK));
				}
			}
			GASPI_CHECK(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK));
			if (options.verify) {
				check_val = my_id == 0 ? 'b' : 'a';
				for (i = 0; i < size * window_size; ++i) {
					if (((char*) ptr)[i] != check_val) {
						fprintf(stderr,
						        "Verification failed. Result is invalid!\n");
						return EXIT_FAILURE;
					}
				}
			}
			print_result(my_id, measurements, size * 2);
			free_gaspi_memory(segment_id_send);
			free_gaspi_memory(segment_id_recv);
		}
	}
	free(measurements.time);
	GASPI_CHECK(gaspi_proc_term(GASPI_BLOCK));
	return EXIT_SUCCESS;
}
