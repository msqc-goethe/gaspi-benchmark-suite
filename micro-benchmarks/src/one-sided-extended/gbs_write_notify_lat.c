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

	options.type = ONESIDED;
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
	const gaspi_queue_id_t q_id = 0;
	const gaspi_notification_t notification_val = 1;
	const gaspi_notification_id_t notification_id = 0;

	print_header(my_id);

	int window_size = options.window_size;
	for (size = options.min_message_size; size <= options.max_message_size;
	     size *= 2) {
		allocate_gaspi_memory(segment_id, size * window_size * sizeof(char),'a');
		if (my_id == 0) {
			for (i = 0; i < options.iterations + options.skip; ++i) {
				if (i == options.skip)
					time = stopwatch_start();
				for (j = 0; j < window_size; ++j) {
					GASPI_CHECK(gaspi_write_notify(segment_id,
					                        j * size,
					                        1,
					                        segment_id,
					                        j * size,
					                        size,
											notification_id,
											notification_val,
					                        q_id,
					                        GASPI_BLOCK));
					GASPI_CHECK(gaspi_wait(q_id, GASPI_BLOCK));
				}
			}
			time = stopwatch_stop(time);
		}
		print_result_lat(my_id, time/1e3, size);
		free_gaspi_memory(segment_id);
	}
	return EXIT_SUCCESS;
}
