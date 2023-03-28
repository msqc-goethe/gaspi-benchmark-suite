#include "util.h"
#include "check.h"

struct benchmark_options_t options;
struct bad_usage_t bad_usage;

int benchmark_options(int argc, char* argv[]) {

	static struct option long_options[] = {
	    {"window-size", required_argument, 0, 'w'},
	    {"help", no_argument, 0, 'h'},
	    {"min-message-size", required_argument, 0, 's'},
	    {"max-message-size", required_argument, 0, 'e'},
	    {"iterations", required_argument, 0, 'i'},
	    {"csv", no_argument, 0, 0},
	    {"warmup-iterations", required_argument, 0, 'u'},
	    {"verbose", no_argument, &options.verbose, 'v'}};

	int option_index = 0;
	int c;

	switch (options.type) {
		case COLLECTIVE:
			break;
		case PASSIVE:
			break;
		case ONESIDED:
			break;
	}

	// set default values
	options.window_size = 64;
	options.min_message_size = 1ULL;
	options.max_message_size = (1ULL << 22);
	options.iterations = 10;
	options.skip = 10;
	options.format = PLAIN;

	while (1) {
		c = getopt_long(
		    argc, argv, "chvi:w:s:e:u:", long_options, &option_index);
		if (c == -1)
			break;
		switch (c) {
			case 'w':
				options.window_size = atoi(optarg);
				break;
			case 'h':
				return OPTIONS_HELP;
			case 's':
				options.min_message_size = atoll(optarg);
				break;
			case 'e':
				options.max_message_size = atoll(optarg);
				break;
			case 'i':
				options.iterations = atoi(optarg);
				break;
			case 0:
				options.format = CSV;
				break;
			case 'u':
				options.skip = atoi(optarg);
				break;
			default:
				bad_usage.message = "Invalid option";
				bad_usage.opt = optopt;
				return OPTIONS_BAD_USAGE;
		}
	}
	return OPTIONS_OKAY;
}

void print_bad_usage(const gaspi_rank_t id) {
	if (id == 0) {
		fprintf(
		    stderr, "%s [-%c]\n\n", bad_usage.message, (char) bad_usage.opt);
		fflush(stderr);
	}
	gaspi_proc_term(GASPI_BLOCK);
}

void print_help_message(const gaspi_rank_t id) {
	if (id == 0) {
		fprintf(stdout, "Help Message\n");
		fflush(stdout);
	}
	gaspi_proc_term(GASPI_BLOCK);
}

void print_header(const gaspi_rank_t id) {
	if (id == 0) {
		if (options.type == ATOMIC) {
			if (options.format == PLAIN)
				fprintf(stdout,
				        "%-*s%*s%*s\n",
				        10,
				        "old_value",
				        FIELD_WIDTH,
				        "new_value",
				        FIELD_WIDTH,
				        "Latency[us]");
			else if (options.format == CSV)
				fprintf(stdout, "old_value,new_value,Latency[us]\n");
		}
		else if (options.subtype == LAT) {
			if (options.format == PLAIN)
				fprintf(stdout,
				        "%-*s%*s\n",
				        10,
				        "message_size[B]",
				        FIELD_WIDTH,
				        "Latency[ns]");
			else if (options.format == CSV)
				fprintf(stdout, "message_size[B],Latency[us]\n");
		}
		else if (options.subtype == BW) {
			if (options.format == PLAIN)
				fprintf(stdout,
				        "%-*s%*s\n",
				        10,
				        "message_size[B]",
				        FIELD_WIDTH,
				        "Bandwidth[MB/s]");
			else if (options.format == CSV)
				fprintf(stdout, "message_size[B],Bandwidth[MB/s]\n");
		}
		else if (options.subtype == ALLREDUCE) {
			if (options.format == PLAIN)
				fprintf(stdout,
				        "%-*s%*s%*s%*s%*s\n",
				        10,
				        "#elements",
				        FIELD_WIDTH,
				        "#ranks",
				        FIELD_WIDTH,
				        "min_time[us]",
				        FIELD_WIDTH,
				        "avg_time[us]",
				        FIELD_WIDTH,
				        "max_time[us]");
			else if (options.format == CSV)
				fprintf(stdout,
				        "#elements,#ranks,min_time[us],avg_time[us],max_time["
				        "us]\n");
		}
		else if (options.subtype == BARRIER) {
			if (options.format == PLAIN)
				fprintf(stdout,
				        "%-*s%*s%*s%*s\n",
				        10,
				        "#ranks",
				        FIELD_WIDTH,
				        "min_time[us]",
				        FIELD_WIDTH,
				        "avg_time[us]",
				        FIELD_WIDTH,
				        "max_time[us]");
			else if (options.format == CSV)
				fprintf(stdout,
				        "#elements,#ranks,min_time[us],avg_time[us],max_time["
				        "us]\n");
		}
		fflush(stdout);
	}
}

void print_result_bw(const gaspi_rank_t id,
                     const double time,
                     const size_t size) {
	if (id == 0) {
		size_t bytes = size * options.iterations * options.window_size;
		double bw = (double) bytes / time;
		bw *= 1e3;
		if (options.format == PLAIN) {
			fprintf(stdout,
			        "%-*d%*.*f\n",
			        10,
			        size,
			        FIELD_WIDTH,
			        FLOAT_PRECISION,
			        bw);
		}
		else if (options.format == CSV) {
			fprintf(stdout, "%d,%.*f\n", size, FLOAT_PRECISION, bw);
		}
		fflush(stdout);
	}
}

void print_result_lat(const gaspi_rank_t id,
                      const double time,
                      const size_t size) {
	if (id == 0) {
		double tmp = time / (double) (options.iterations * options.window_size);

		if (options.format == PLAIN) {
			fprintf(stdout,
			        "%-*d%*.*f\n",
			        10,
			        size,
			        FIELD_WIDTH,
			        FLOAT_PRECISION,
			        tmp);
		}
		else if (options.format == CSV) {
			fprintf(stdout, "%d,%.*f\n", size, FLOAT_PRECISION, tmp);
		}
		fflush(stdout);
	}
}

void print_result_coll(const gaspi_rank_t id,
                       const int num_pes,
                       const size_t size,
                       const double min_time,
                       const double avg_time,
                       const double max_time) {
	if (id == 0) {
		if (options.subtype == ALLREDUCE) {
			if (options.format == PLAIN) {
				fprintf(stdout,
				        "%-*d%*d%*.*f%*.*f%*.*f\n",
				        10,
				        size,
				        FIELD_WIDTH,
				        num_pes,
				        FIELD_WIDTH,
				        FLOAT_PRECISION,
				        min_time,
				        FIELD_WIDTH,
				        FLOAT_PRECISION,
				        avg_time,
				        FIELD_WIDTH,
				        FLOAT_PRECISION,
				        max_time);
			}
			else if (options.format == CSV) {
				fprintf(stdout,
				        "%d,%d,%*.*f,%*.*f,%*.*f\n",
				        size,
				        num_pes,
				        FLOAT_PRECISION,
				        min_time,
				        FLOAT_PRECISION,
				        avg_time,
				        FLOAT_PRECISION,
				        min_time);
			}
		}
		else if (options.subtype == BARRIER) {
			if (options.format == PLAIN) {
				fprintf(stdout,
				        "%-*d%*.*f%*.*f%*.*f\n",
				        10,
				        num_pes,
				        FIELD_WIDTH,
				        FLOAT_PRECISION,
				        min_time,
				        FIELD_WIDTH,
				        FLOAT_PRECISION,
				        avg_time,
				        FIELD_WIDTH,
				        FLOAT_PRECISION,
				        max_time);
			}
			else if (options.format == CSV) {
				fprintf(stdout,
				        "%d,%*.*f,%*.*f,%*.*f\n",
				        num_pes,
				        FLOAT_PRECISION,
				        min_time,
				        FLOAT_PRECISION,
				        avg_time,
				        FLOAT_PRECISION,
				        min_time);
			}
		}
		fflush(stdout);
	}
}

void print_atomic_lat(const gaspi_rank_t id,
                      const char old_value,
                      const char new_value,
                      const double time) {
	double tmp = time / options.iterations;
	if (id == 0) {
		if (options.format == PLAIN) {
			fprintf(stdout,
			        "%-*c%*c%*.*f\n",
			        10,
			        old_value,
			        FIELD_WIDTH,
			        new_value,
			        FIELD_WIDTH,
			        FLOAT_PRECISION,
			        tmp);
		}
		else if (options.format == CSV) {
			fprintf(stdout,
			        "%c,%c,%*.*f\n",
			        old_value,
			        new_value,
			        FLOAT_PRECISION,
			        tmp);
		}
		fflush(stdout);
	}
}
