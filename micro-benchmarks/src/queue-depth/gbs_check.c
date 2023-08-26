#include "check.h"
#include "stopwatch.h"
#include "util.h"
#include "util_memory.h"

int main(int argc, char* argv[]) {
	gaspi_rank_t my_id, num_pes;
	int i, j, k;
	int bo_ret = OPTIONS_OKAY;

	options.type = ONESIDED;
	options.subtype = BW;
	options.name = "gbs_check";

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
	const int bytes = 2;
	const gaspi_segment_id_t segment_id = 0;
	const gaspi_queue_id_t q_id = 0;

	allocate_gaspi_memory(segment_id, 2 * sizeof(char), my_id == 0 ? 'a' : 'b');
	if (my_id == 0)
		printf("Running ITWM Benchmark logic without error checking\n");
	for (int i = 0; i < 23; i++) {
		for (int j = 0; j < 10; j++) {

			for (int k = 0; k < 1000; k++) {
				gaspi_write(segment_id, 0, 1, 0, 0, bytes, q_id, GASPI_BLOCK);
			}
			gaspi_wait(q_id, GASPI_BLOCK);
		}
	}
	if (my_id == 0) {
		printf("Passed!\n");
		printf("Now running same logic with error checking\n");
	}
	for (int i = 0; i < 23; i++) {
		for (int j = 0; j < 10; j++) {

			for (int k = 0; k < 1000; k++) {
				GASPI_CHECK(gaspi_write(
				    segment_id, 0, 1, 0, 0, bytes, q_id, GASPI_BLOCK));
			}
			GASPI_CHECK(gaspi_wait(q_id, GASPI_BLOCK));
		}
	}
	free_gaspi_memory(segment_id);
	GASPI_CHECK(gaspi_proc_term(GASPI_BLOCK));
	return EXIT_SUCCESS;
}
