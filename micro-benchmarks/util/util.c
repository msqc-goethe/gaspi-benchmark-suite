#include "util.h"
#include "check.h"
#include "math.h"

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
	    {"raw_csv", no_argument, 0, 1},
	    {"warmup-iterations", required_argument, 0, 'u'},
	    {"verbose", no_argument, &options.verbose, 'v'}};

	int option_index = 0;
	int c;
	char* optstring = NULL;

	if (options.type == PASSIVE) {
		optstring = "chi:w:s:e:u";
	}
	else if (options.type == ONESIDED) {
		optstring = "chi:w:s:e:u";
	}
	else if (options.type == ATOMIC) {
		optstring = "chi:w:s:e:u";
	}
	else if (options.type == COLLECTIVE) {
		optstring = "chi:w:s:e:u";
	}

	// set default values
	options.window_size = 64;
	options.min_message_size = 1ULL;
	options.max_message_size = (1ULL << 22);
	options.iterations = 10;
	options.skip = 10;
	options.format = PLAIN;

	if (options.subtype == ALLREDUCE) {
		gaspi_allreduce_elem_max(&c);
		options.max_message_size = (size_t) c;
	}

	while (1) {
		c = getopt_long(argc, argv, optstring, long_options, &option_index);
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
			case 1:
				options.format = RAW_CSV;
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
				        "%-*s%*s%*s%*s%*s%*s%*s\n",
				        10,
				        "old_value",
				        FIELD_WIDTH,
				        "new_value",
				        FIELD_WIDTH,
				        "min_lat",
				        FIELD_WIDTH,
				        "max_lat",
				        FIELD_WIDTH,
				        "avg_lat",
				        FIELD_WIDTH,
				        "var_lat",
				        FIELD_WIDTH,
				        "std_lat");
			else if (options.format == CSV)
				fprintf(stdout,
				        "old_value,new_value,min_lat,max_lat,avg_lat,var_lat,"
				        "std_lat\n");
			else if (options.format == RAW_CSV) {
				fprintf(stdout, "old,new,count,lat\n");
			}
		}
		else if (options.subtype == LAT) {
			if (options.format == PLAIN)
				fprintf(stdout,
				        "%-*s%*s%*s%*s%*s%*s%*s\n",
				        10,
				        "msg_size",
				        FIELD_WIDTH,
				        "min_lat",
				        FIELD_WIDTH,
				        "max_lat",
				        FIELD_WIDTH,
				        "avg_lat",
				        FIELD_WIDTH,
				        "median_lat",
				        FIELD_WIDTH,
				        "var_lat",
				        FIELD_WIDTH,
				        "std_lat");
			else if (options.format == CSV)
				fprintf(stdout,
				        "msg_size,min_lat,max_lat,avg_lat,median_lat,var_lat,"
				        "std_lat\n");
			else if (options.format == RAW_CSV) {
				fprintf(stdout, "msg_size,count,lat\n");
			}
		}
		else if (options.subtype == BW) {
			if (options.format == PLAIN)
				fprintf(stdout,
				        "%-*s%*s%*s%*s%*s%*s%*s\n",
				        10,
				        "msg_size",
				        FIELD_WIDTH,
				        "min_bw",
				        FIELD_WIDTH,
				        "max_bw",
				        FIELD_WIDTH,
				        "avg_bw",
				        FIELD_WIDTH,
				        "median_bw",
				        FIELD_WIDTH,
				        "var_bw",
				        FIELD_WIDTH,
				        "std_bw");
			else if (options.format == CSV)
				fprintf(
				    stdout,
				    "msg_size,min_bw,max_bw,avg_bw,median_bw,var_bw,std_bw\n");
			else if (options.format == RAW_CSV) {
				fprintf(stdout, "msg_size,count,bw\n");
			}
		}
		else if (options.subtype == ALLREDUCE) {
			if (options.format == PLAIN)
				fprintf(stdout,
				        "%-*s%*s%*s%*s%*s%*s%*s\n",
				        10,
				        "#elements",
				        FIELD_WIDTH,
				        "#ranks",
				        FIELD_WIDTH,
				        "min_lat",
				        FIELD_WIDTH,
				        "max_lat",
				        FIELD_WIDTH,
				        "avg_lat",
				        FIELD_WIDTH,
				        "var_lat",
				        FIELD_WIDTH,
				        "std_lat");
			else if (options.format == CSV)
				fprintf(stdout,
				        "elements,ranks,min_lat,max_lat,avg_lat,var_lat,std_"
				        "lat\n");
			else if (options.format == RAW_CSV) {
				fprintf(stdout, "elements,ranks,count,lat\n");
			}
		}
		else if (options.subtype == BARRIER) {
			if (options.format == PLAIN)
				fprintf(stdout,
				        "%-*s%*s%*s%*s%*s%*s\n",
				        10,
				        "#ranks",
				        FIELD_WIDTH,
				        "min_lat",
				        FIELD_WIDTH,
				        "max_lat",
				        FIELD_WIDTH,
				        "avg_lat",
				        FIELD_WIDTH,
				        "var_lat",
				        FIELD_WIDTH,
				        "std_lat");
			else if (options.format == CSV)
				fprintf(stdout,
				        "elements,ranks,min_lat,max_lat,avg_lat,var_lat,std_"
				        "lat\n");
			else if (options.format == RAW_CSV) {
				fprintf(stdout, "elements,ranks,count,lat\n");
			}
		}
		fflush(stdout);
	}
}

static int double_cmp(const void* a, const void* b) {
	return *((const double*) (a)) > *((const double*) (b));
}

void compute_statistics(struct measurements_t measurements,
                        struct statistics_t* statistics,
                        const size_t size) {
	int n, i;
	double sum = 0;
	double* t;

	n = measurements.n;
	t = measurements.time;

	if (options.subtype == BW) {
		for (i = 0; i < n; ++i) {
			t[i] = (double) size / t[i];
			t[i] *= 1e3; // MB/s
		}
	}
	else if (options.subtype == LAT || options.subtype == ALLREDUCE ||
	         options.subtype == BARRIER) {
		for (i = 0; i < n; ++i) {
			t[i] *= 1e-3; // ns to us
		}
	}

	qsort(t, n, sizeof *t, double_cmp);

	statistics->min = t[0];
	statistics->max = t[n - 1];

	for (i = 0; i < n; ++i) {
		sum += t[i];
	}

	statistics->avg = sum / n;
	statistics->median = t[n / 2];

	sum = 0;

	for (i = 0; i < n; ++i) {
		sum += (t[i] - statistics->avg) * (t[i] - statistics->avg);
	}

	statistics->var = sum / n;
	statistics->std = sqrt(statistics->var);
}

void print_result(const gaspi_rank_t id,
                  struct measurements_t measurements,
                  const size_t size) {
	struct statistics_t statistics;
	size_t bytes;
	int i;
	if (id == 0) {
		bytes = size * options.window_size;
		compute_statistics(measurements, &statistics, bytes);
		if (options.format == PLAIN) {
			fprintf(stdout,
			        "%-*d%*.*f%*.*f%*.*f%*.*f%*.*f%*.*f\n",
			        10,
			        size,
			        FIELD_WIDTH,
			        FLOAT_PRECISION,
			        statistics.min,
			        FIELD_WIDTH,
			        FLOAT_PRECISION,
			        statistics.max,
			        FIELD_WIDTH,
			        FLOAT_PRECISION,
			        statistics.avg,
			        FIELD_WIDTH,
			        FLOAT_PRECISION,
			        statistics.median,
			        FIELD_WIDTH,
			        FLOAT_PRECISION,
			        statistics.var,
			        FIELD_WIDTH,
			        FLOAT_PRECISION,
			        statistics.std);
		}
		else if (options.format == CSV) {
			fprintf(stdout,
			        "%d,%.*f,%.*f,%.*f,%.*f,%.*f,%.*f\n",
			        size,
			        FLOAT_PRECISION,
			        statistics.min,
			        FLOAT_PRECISION,
			        statistics.max,
			        FLOAT_PRECISION,
			        statistics.avg,
			        FLOAT_PRECISION,
			        statistics.median,
			        FLOAT_PRECISION,
			        statistics.std,
			        FLOAT_PRECISION,
			        statistics.var);
		}
		else if (options.format == RAW_CSV) {
			for (i = 0; i < measurements.n; ++i) {
				fprintf(stdout, "%d,%d,%.*f\n", size, i, measurements.time[i]);
			}
		}
		fflush(stdout);
	}
}

void print_result_coll(const gaspi_rank_t id,
                       const int num_pes,
                       const size_t size,
                       struct measurements_t measurements) {
	if (id == 0) {
		struct statistics_t statistics;
		compute_statistics(measurements, &statistics, 0);
		if (options.subtype == ALLREDUCE) {
			if (options.format == PLAIN) {
				fprintf(stdout,
				        "%-*d%*d%*.*f%*.*f%*.*f%*.*f%*.*f\n",
				        10,
				        size,
				        FIELD_WIDTH,
				        num_pes,
				        FIELD_WIDTH,
				        FLOAT_PRECISION,
				        statistics.min,
				        FIELD_WIDTH,
				        FLOAT_PRECISION,
				        statistics.max,
				        FIELD_WIDTH,
				        FLOAT_PRECISION,
				        statistics.avg,
				        FIELD_WIDTH,
				        FLOAT_PRECISION,
				        statistics.var,
				        FIELD_WIDTH,
				        FLOAT_PRECISION,
				        statistics.std);
			}
			else if (options.format == CSV) {
				fprintf(stdout,
				        "%d,%d,%*.*f,%*.*f,%*.*f,%*.*f,%*.*f\n",
				        size,
				        num_pes,
				        FLOAT_PRECISION,
				        statistics.min,
				        FLOAT_PRECISION,
				        statistics.max,
				        FLOAT_PRECISION,
				        statistics.avg,
				        FLOAT_PRECISION,
				        statistics.var,
				        FLOAT_PRECISION,
				        statistics.std);
			}
		}
		else if (options.subtype == BARRIER) {
			if (options.format == PLAIN) {
				fprintf(stdout,
				        "%-*d%*.*f%*.*f%*.*f%*.*f%*.*f\n",
				        10,
				        num_pes,
				        FIELD_WIDTH,
				        FLOAT_PRECISION,
				        statistics.min,
				        FIELD_WIDTH,
				        FLOAT_PRECISION,
				        statistics.max,
				        FIELD_WIDTH,
				        FLOAT_PRECISION,
				        statistics.avg,
				        FIELD_WIDTH,
				        FLOAT_PRECISION,
				        statistics.var,
				        FIELD_WIDTH,
				        FLOAT_PRECISION,
				        statistics.std);
			}
			else if (options.format == CSV) {
				fprintf(stdout,
				        "%d,%*.*f,%*.*f,%*.*f,%*.*f,%*.*f\n",
				        num_pes,
				        FLOAT_PRECISION,
				        statistics.min,
				        FLOAT_PRECISION,
				        statistics.max,
				        FLOAT_PRECISION,
				        statistics.avg,
				        FLOAT_PRECISION,
				        statistics.var,
				        FLOAT_PRECISION,
				        statistics.std);
			}
		}
		fflush(stdout);
	}
}

void print_atomic_lat(const gaspi_rank_t id,
                      char const* old,
                      char const* new,
                      struct measurements_t measurements) {
	struct statistics_t statistics;
	int i, n;
	if (id == 0) {
		compute_statistics(measurements, &statistics, 0);
		if (options.format == PLAIN) {
			fprintf(stdout,
			        "%-*c%*c%*.*f%*.*f%*.*f%*.*f%*.*f\n",
			        10,
			        *old,
			        FIELD_WIDTH,
			        *new,
			        FIELD_WIDTH,
			        FLOAT_PRECISION,
			        statistics.min,
			        FIELD_WIDTH,
			        FLOAT_PRECISION,
			        statistics.max,
			        FIELD_WIDTH,
			        FLOAT_PRECISION,
			        statistics.avg,
			        FIELD_WIDTH,
			        FLOAT_PRECISION,
			        statistics.var,
			        FIELD_WIDTH,
			        FLOAT_PRECISION,
			        statistics.std);
		}
		else if (options.format == CSV) {
			fprintf(stdout,
			        "%c,%c,%.*f,%.*f,%.*f,%.*f,%.*f\n",
			        *old,
			        *new,
			        FLOAT_PRECISION,
			        statistics.min,
			        FLOAT_PRECISION,
			        statistics.max,
			        FLOAT_PRECISION,
			        statistics.avg,
			        FLOAT_PRECISION,
			        statistics.var,
			        FLOAT_PRECISION,
			        statistics.std);
		}
		else if (options.format == RAW_CSV) {
			for (i = 0; i < measurements.n; ++i) {
				fprintf(stdout,
				        "%c,%c,%d,%.*f\n",
				        old[i],
				        new[i],
				        i,
				        FLOAT_PRECISION,
				        measurements.time[i]);
			}
		}
		fflush(stdout);
	}
}
