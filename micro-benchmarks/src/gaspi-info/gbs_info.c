#include "check.h"
#include "util.h"

int main(int argc, char* argv[]) {
	float version = 0.0f;
	gaspi_rank_t my_id, num_pes;
	gaspi_number_t group_max, segment_max, queue_num, queue_size_max, queue_max,
	    notification_num, allreduce_max_elem;
	gaspi_size_t transfer_size_max, passive_transfer_size_max,
	    allreduce_user_buf_size;
	gaspi_atomic_value_t atomic_max_value;
	gaspi_network_t network;

	GASPI_CHECK(gaspi_proc_init(GASPI_BLOCK));
	GASPI_CHECK(gaspi_proc_rank(&my_id));
	GASPI_CHECK(gaspi_proc_num(&num_pes));
	GASPI_CHECK(gaspi_group_max(&group_max));
	GASPI_CHECK(gaspi_segment_max(&segment_max));
	GASPI_CHECK(gaspi_queue_num(&queue_num));
	GASPI_CHECK(gaspi_queue_size_max(&queue_size_max));
	GASPI_CHECK(gaspi_queue_max(&queue_max));
	GASPI_CHECK(gaspi_transfer_size_max(&transfer_size_max));
	GASPI_CHECK(gaspi_notification_num(&notification_num));
	GASPI_CHECK(gaspi_passive_transfer_size_max(&passive_transfer_size_max));
	GASPI_CHECK(gaspi_atomic_max(&atomic_max_value));
	GASPI_CHECK(gaspi_allreduce_buf_size(&allreduce_user_buf_size));
	GASPI_CHECK(gaspi_allreduce_elem_max(&allreduce_max_elem));
	GASPI_CHECK(gaspi_network_type(&network));
	GASPI_CHECK(gaspi_version(&version));

	if (my_id == 0) {
		fprintf(
		    stdout,
		    "Maximum number of groups: %u\nMaximum number of permissible "
		    "segments: %u\nNumber of available Queues: %u\nMaximum number of "
		    "simultaneous requests allowed: %u\nMaximum number of allowed "
		    "Queues: %u\nMaximum transfet size for a single request: "
		    "%u\nNumber of available notifications: %u\nMaximum transfer size "
		    "per single passive communication request: %u\nMaximum value for "
		    "atomic value: %u\nAllreduce internal user buffer size: "
		    "%u\nMaximum number of elements allowed in gaspi_allreduce: %u\n",
		    group_max,
		    segment_max,
		    queue_num,
		    queue_size_max,
		    queue_max,
		    transfer_size_max,
		    notification_num,
		    passive_transfer_size_max,
		    atomic_max_value,
		    allreduce_user_buf_size,
		    allreduce_max_elem);
		fprintf(stdout, "Network Type: ");
		switch (network) {
			case 0:
				fprintf(stdout, "GASBPI_IB\n");
				break;
			case 1:
				fprintf(stdout, "GASBPI_ROCE\n");
				break;
			case 2:
				fprintf(stdout, "GASBPI_ETHERNET\n");
				break;
			case 3:
				fprintf(stdout, "GASBPI_GEMINI\n");
				break;
			case 4:
				fprintf(stdout, "GASBPI_ARIES\n");
				break;
			default:
				fprintf(stdout, "Unknown Network Type!\n");
				break;
		}
		fprintf(stdout, "Version: %f\n", version);
	}
	GASPI_CHECK(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK));
	GASPI_CHECK(gaspi_proc_term(GASPI_GROUP_ALL));
	return EXIT_SUCCESS;
}
