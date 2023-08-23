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

	gaspi_config_t conf;

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

	const gaspi_segment_id_t segment_id_a = 0;
	const gaspi_segment_id_t segment_id_b = 1;
	const gaspi_queue_id_t q_id = 0;
	gaspi_pointer_t ptr_a;
	gaspi_pointer_t ptr_b;
	char* ptr_check;

	print_header(my_id);

	int window_size = options.window_size;
	for (size = options.min_message_size; size <= options.max_message_size;
	     size *= 2) {
		allocate_gaspi_memory(
		    segment_id_a, size * window_size * sizeof(char), 'a');
		allocate_gaspi_memory(
		    segment_id_b, size * window_size * sizeof(char), 'b');
		GASPI_CHECK(gaspi_segment_ptr(segment_id_a, &ptr_a));
		GASPI_CHECK(gaspi_segment_ptr(segment_id_b, &ptr_b));

		GASPI_CHECK(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK));
		if (my_id == 0) {
			for (i = 0; i < options.iterations + options.skip; ++i) {
				GASPI_CHECK(gaspi_barrier(q_id, GASPI_BLOCK));
				if (i >= options.skip) {
					time = stopwatch_start();
				}
				for (j = 0; j < window_size; ++j) {
					GASPI_CHECK(gaspi_write(segment_id_b,
					                        j * size,
					                        1,
					                        segment_id_a,
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
		else if (my_id == 1) {
			for (i = 0; i < options.iterations + options.skip; ++i) {
				GASPI_CHECK(gaspi_barrier(q_id, GASPI_BLOCK));
				for (j = 0; j < window_size; ++j) {
					GASPI_CHECK(gaspi_write(segment_id_a,
					                        j * size,
					                        0,
					                        segment_id_b,
					                        j * size,
					                        size,
					                        q_id,
					                        GASPI_BLOCK));
				}
				GASPI_CHECK(gaspi_wait(q_id, GASPI_BLOCK));
			}
		}
		GASPI_CHECK(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK));
		if (verify) {
			ptr_check = my_id == 0 ? ptr_b : ptr_a;
			check_val = my_id == 0 ? 'a' : 'b';
			for (i = 0; i < size * window_size; ++i) {
				if (ptr_check[i] != check_val) {
					fprintf(stderr,
					        "Verification failed. Result is invalid!\n");
					return EXIT_FAILURE;
				}
			}
		}
		print_result(my_id, measurements, size * 2);
		free_gaspi_memory(segment_id_a);
		free_gaspi_memory(segment_id_b);
	}
	free(measurements.time);
	return EXIT_SUCCESS;
}
