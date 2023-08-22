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
	struct measurements_t measurements;

	options.type = NOTIFY;
	options.subtype = LAT;
	options.name = "gbs_notification_rate";

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

	const gaspi_segment_id_t segment_id = 0;
	const gaspi_queue_id_t q_id = 0;
	gaspi_notification_t notification_val = 1;
	gaspi_notification_id_t notification_id = 0;

	print_header(my_id);

	allocate_gaspi_memory(segment_id, sizeof(char), 'a');
	for (i = 0; i < options.iterations + options.skip; ++i) {
		if (my_id == 0) {
			if (i >= options.skip) {
				time = stopwatch_start();
			}
			GASPI_CHECK(gaspi_notify(segment_id,
			                         1,
			                         notification_id,
			                         notification_val,
			                         q_id,
			                         GASPI_BLOCK));
			GASPI_CHECK(gaspi_wait(q_id, GASPI_BLOCK));
			if (i >= options.skip) {
				measurements.time[i - options.skip] = stopwatch_stop(time);
			}
		}
	}
	print_notify_lat(my_id, measurements);
	free_gaspi_memory(segment_id);
	free(measurements.time);
	return EXIT_SUCCESS;
}
