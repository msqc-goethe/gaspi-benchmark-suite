#include "check.h"
#include "stopwatch.h"
#include "util.h"
#include "util_memory.h"

int main(int argc, char* argv[]) {
	gaspi_rank_t my_id, num_pes;
	size_t size;
	int i;
	int bo_ret = OPTIONS_OKAY;
	double time;
	struct measurements_t measurements;

	options.type = PASSIVE;
	options.subtype = LAT;
	options.name = "gbs_passive_lat";

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

	gaspi_size_t max_transfer_size;
	GASPI_CHECK(gaspi_passive_transfer_size_max(&max_transfer_size));

	if (options.max_message_size > max_transfer_size) {
		if (my_id == 0) {
			fprintf(stderr,
			        "Message size was truncated to %d!\n",
			        max_transfer_size);
		}
		options.max_message_size = max_transfer_size;
	}

	measurements.time = malloc(options.iterations * sizeof(double));
	measurements.n = options.iterations;

	const gaspi_segment_id_t segment_id_a = 0;
	const gaspi_segment_id_t segment_id_b = 1;
	gaspi_rank_t remote_id = my_id == 0 ? 1 : 0;
	gaspi_pointer_t ptr_a, ptr_b;

	print_header(my_id);

	if (options.single_buffer) {
		allocate_gaspi_memory(segment_id_a,
		                      options.max_message_size * sizeof(char),
		                      my_id == 0 ? 'a' : 'b');
		allocate_gaspi_memory(segment_id_b,
		                      options.max_message_size * sizeof(char),
		                      my_id == 0 ? 'a' : 'b');
		GASPI_CHECK(gaspi_segment_ptr(segment_id_a, &ptr_a));
		GASPI_CHECK(gaspi_segment_ptr(segment_id_b, &ptr_b));

		for (size = options.min_message_size; size <= options.max_message_size;
		     size *= 2) {
			for (i = 0; i < options.iterations + options.skip; ++i) {
				if (i >= options.skip) {
					time = stopwatch_start();
				}
				if (my_id == 0) {
					GASPI_CHECK(gaspi_passive_send(
					    segment_id_a, 0, 1, size, GASPI_BLOCK));
					GASPI_CHECK(gaspi_passive_receive(
					    segment_id_b, 0, &remote_id, size, GASPI_BLOCK));
				}
				else if (my_id == 1) {
					GASPI_CHECK(gaspi_passive_receive(
					    segment_id_a, 0, &remote_id, size, GASPI_BLOCK));
					GASPI_CHECK(gaspi_passive_send(
					    segment_id_b, 0, 0, size, GASPI_BLOCK));
				}
				if (i >= options.skip) {
					measurements.time[i - options.skip] = stopwatch_stop(time);
					measurements.time[i - options.skip] /=
					    2.0; // send-receive special case
				}
			}
			if (options.verify) {
				const gaspi_pointer_t ptr = my_id == 0 ? ptr_b : ptr_a;
				const char check_val = my_id == 0 ? 'b' : 'a';
				for (i = 0; i < size; ++i) {
					if (((char*) ptr)[i] != check_val) {
						fprintf(stderr,
						        "Verification failed. Result is invalid!\n");
						return EXIT_FAILURE;
					}
				}
			}
			print_result(my_id, measurements, size);
		}
		free_gaspi_memory(segment_id_a);
		free_gaspi_memory(segment_id_b);
	}
	else {
		for (size = options.min_message_size; size <= options.max_message_size;
		     size *= 2) {
			allocate_gaspi_memory(
			    segment_id_a, size * sizeof(char), my_id == 0 ? 'a' : 'b');
			allocate_gaspi_memory(
			    segment_id_b, size * sizeof(char), my_id == 0 ? 'a' : 'b');
			GASPI_CHECK(gaspi_segment_ptr(segment_id_a, &ptr_a));
			GASPI_CHECK(gaspi_segment_ptr(segment_id_b, &ptr_b));
			for (i = 0; i < options.iterations + options.skip; ++i) {
				if (i >= options.skip) {
					time = stopwatch_start();
				}
				if (my_id == 0) {
					GASPI_CHECK(gaspi_passive_send(
					    segment_id_a, 0, 1, size, GASPI_BLOCK));
					GASPI_CHECK(gaspi_passive_receive(
					    segment_id_b, 0, &remote_id, size, GASPI_BLOCK));
				}
				else if (my_id == 1) {
					GASPI_CHECK(gaspi_passive_receive(
					    segment_id_a, 0, &remote_id, size, GASPI_BLOCK));
					GASPI_CHECK(gaspi_passive_send(
					    segment_id_b, 0, 0, size, GASPI_BLOCK));
				}
				if (i >= options.skip) {
					measurements.time[i - options.skip] = stopwatch_stop(time);
				}
			}
			if (options.verify) {
				const gaspi_pointer_t ptr = my_id == 0 ? ptr_b : ptr_a;
				const char check_val = my_id == 0 ? 'b' : 'a';
				for (i = 0; i < size; ++i) {
					if (((char*) ptr)[i] != check_val) {
						fprintf(stderr,
						        "Verification failed. Result is invalid!\n");
						return EXIT_FAILURE;
					}
				}
			}
			print_result(my_id, measurements, size);
			free_gaspi_memory(segment_id_a);
			free_gaspi_memory(segment_id_b);
		}
	}
	free(measurements.time);
	GASPI_CHECK(gaspi_proc_term(GASPI_BLOCK));
	return EXIT_SUCCESS;
}
