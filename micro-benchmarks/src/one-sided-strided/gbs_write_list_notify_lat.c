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

	options.type = ONESIDED;
	options.subtype = LAT;
	options.name = "gbs_write_notify_lat";

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

	size_t begin = 0, end = 0;
	size_t min_message_size = options.min_message_size;
	size_t max_message_size = options.max_message_size;

	while (min_message_size >>= 1)
		begin++;
	while (max_message_size >>= 1)
		end++;
	size_t stride_count = end - begin;
	gaspi_segment_id_t* segment_ids =
	    malloc(stride_count * sizeof(gaspi_segment_id_t));
	gaspi_size_t* sizes = malloc(stride_count * sizeof(gaspi_size_t));
	gaspi_offset_t* local_offsets =
	    malloc(stride_count * sizeof(gaspi_offset_t));
	gaspi_offset_t* remote_offsets =
	    malloc(stride_count * sizeof(gaspi_offset_t));
	
	const gaspi_notification_id_t notification_id = 0;
	const gaspi_notification_t notification_val = 1;
	const gaspi_segment_id_t notify_segment = 0;
	const gaspi_queue_id_t q_id = 0;
	gaspi_pointer_t ptr;

	print_header(my_id);
	size_t segment_idx = 0;

	for (int i = begin; i <= end; ++i, ++segment_idx) {
		segment_ids[segment_idx] = segment_idx;
		sizes[segment_idx] = (1 << i) * sizeof(char);
		local_offsets[segment_idx] = 0;
		remote_offsets[segment_idx] = 0;
		allocate_gaspi_memory(segment_ids[segment_idx],
		                      sizes[segment_idx],
		                      my_id == 0 ? 'a' : 'b');
	}

	if (my_id == 0) {
		for (i = 0; i < options.iterations + options.skip; ++i) {
			if (i >= options.skip) {
				time = stopwatch_start();
			}
			GASPI_CHECK(gaspi_write_list_notify(stride_count,
			                             segment_ids,
			                             local_offsets,
			                             1,
			                             segment_ids,
			                             remote_offsets,
			                             sizes,
										 notify_segment,
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

	if (my_id == 1 && options.verify) {
		GASPI_CHECK(gaspi_notify_waitsome(0,0,1,0,GASPI_BLOCK));
		for (int i = 0; i < stride_count; ++i) {
			GASPI_CHECK(gaspi_segment_ptr(segment_ids[i], &ptr));
			for (int u = 0; u < sizes[i]; ++u) {
				if (((char*) ptr)[u] != 'a') {
					fprintf(stderr,
					        "Verification failed. Result is invalid!\n");
					return EXIT_FAILURE;
				}
			}
		}
	}
	GASPI_CHECK(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK));
	print_list_lat(my_id, measurements, stride_count);
	for (int i = 0; i < stride_count; ++i) {
		free_gaspi_memory(segment_ids[i]);
	}
	free(segment_ids);
	free(local_offsets);
	free(remote_offsets);
	free(sizes);
	free(measurements.time);
	GASPI_CHECK(gaspi_proc_term(GASPI_BLOCK));
	return EXIT_SUCCESS;
}
