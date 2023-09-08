#include "util.h"
#include "benchdefs.h"
#include "check.h"
#include "math.h"
#include "stopwatch.h"

struct benchmark_options_t options;
struct bad_usage_t bad_usage;
int verify = 1;

int benchmark_options(int argc, char* argv[]) {

	static struct option long_options[] = {
	    {"window-size", required_argument, 0, 'w'},
	    {"help", no_argument, 0, 'h'},
	    {"min-message-size", required_argument, 0, 's'},
	    {"max-message-size", required_argument, 0, 'e'},
	    {"iterations", required_argument, 0, 'i'},
	    {"csv", no_argument, 0, 0},
	    {"raw_csv", no_argument, 0, 1},
	    {"verify", no_argument, 0, 'v'},
	    {"single-buffer", no_argument, 0, 'b'},
	    {"timer", required_argument, 0, 't'},
	    {"warmup-iterations", required_argument, 0, 'u'}};

	int option_index = 0;
	int c;
	char* optstring = NULL;
	if (options.type == PASSIVE) {
		optstring = "hi:w:s:e:u:vt:";
	}
	else if (options.type == ONESIDED) {
		if (options.subtype == LAT) {
			optstring = "hi:w:s:e:u:vt:";
		}
		else {
			optstring = "hi:w:s:e:u:vbt:";
		}
	}
	else if (options.type == ATOMIC) {
		optstring = "hi:u:t:";
	}
	else if (options.type == COLLECTIVE) {
		if (options.subtype == ALLREDUCE) {
			optstring = "hi:w:s:e:u:vt:";
		}
		else if (options.subtype == BARRIER) {
			optstring = "hi:u:t:";
		}
	}
	else if (options.type == NOTIFY) {
		if (options.subtype == RATE)
			optstring = "hi:u:w:t:";
		else
			optstring = "hi:u:t:";
	}

	// set default values
	options.window_size = DEFAULT_WINDOW_SIZE;
	options.min_message_size = DEFAULT_MIN_MESSAGE_SIZE;
	options.max_message_size = DEFAULT_MAX_MESSAGE_SIZE;
	options.iterations = DEFAULT_ITERATIONS;
	options.skip = DEFAULT_WARMUP_ITERATIONS;
	options.format = PLAIN;
	options.verify = 0;
	options.single_buffer = 0;
	options.memory_mode =
	    options.subtype == LAT ? "single_buffer" : "multiple_buffer";
	options.gaspi_timer = 0;

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
			case 'v':
				options.verify = 1;
				break;
			case 'b':
				options.single_buffer = 1;
				options.memory_mode = "single_buffer";
				break;
			case 't':
				options.gaspi_timer = atoi(optarg);
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
	init_timer(&benchmark_timer);
	return OPTIONS_OKAY;
}

void print_bad_usage() {
	fprintf(stderr, "%s [-%c]\n\n", bad_usage.message, (char) bad_usage.opt);
	fflush(stderr);
}

void print_help_message() {
	fprintf(stdout, "GASPI Micro Benchmark:\n");
	fprintf(stdout, "%s:\n", options.name);
	fprintf(stdout, "\n");
	fprintf(stdout, "\t -h [--help]\tDisplay this help message.\n");

	if (options.subtype != BARRIER && options.type != ATOMIC &&
	    options.type != NOTIFY) {
		fprintf(stdout,
		        "\t -w [--window_size] arg\tNumber of messages sent per "
		        "iteration. Default 64.\n");
		fprintf(stdout,
		        "\t -s [--min_message_size] arg\t Minimum message size. "
		        "Default 1 byte.\n");
		fprintf(stdout,
		        "\t -e [--max_message_size] arg\t Maximum message size. "
		        "Default (1 << 22) byte.\n");
		fprintf(stdout,
		        "\t -v [--verify]\tCheck results of the performed "
		        "operation.\n");
		if (options.subtype != LAT) {
			fprintf(stdout,
			        "\t -b [--single-buffer]\tUse a single memory allocation "
			        "for the measurements.\n");
		}
	}
	else if (options.type == NOTIFY && options.subtype == RATE) {
		fprintf(stdout,
		        "\t -w [--window_size] arg\tNumber of messages sent per "
		        "iteration. Default 64.\n");
	}

	fprintf(stdout,
	        "\t -i [--iterations] arg\tNumber of iterations. Default 10.\n");
	fprintf(stdout,
	        "\t -u [--warmup-iterations] arg\tNumber of warmup iterations. "
	        "Default 10.\n");
	fprintf(stdout, "\t --csv\tPrint output in csv format with statistics.\n");
	fprintf(stdout,
	        "\t --raw_csv\tPrint the collected raw data without statistics.\n");
	fprintf(stdout,
	        "\t -t [--timer] arg\t 0: clock_gettime | 1: gaspi_time_get | 2: "
	        "gaspi_time_ticks.\n");
	fprintf(stdout, "\n\n");
	fflush(stdout);
}

void print_header(const gaspi_rank_t id) {
	if (id == 0) {
		if (options.type == ATOMIC) {
			if (options.format == PLAIN) {
				fprintf(stdout,
				        "%-*s%*s%*s%*s%*s%*s%*s\n",
				        10,
				        "#iterations",
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
			}
			else if (options.format == CSV) {
				fprintf(stdout,
				        "#iterations,min_lat,max_lat,avg_lat,median_"
				        "lat,var_lat,"
				        "std_lat\n");
			}
			else if (options.format == RAW_CSV) {
				fprintf(stdout, "old,new,count,lat\n");
			}
		}
		else if (options.type == NOTIFY) {
			if (options.subtype == RATE) {
				if (options.format == PLAIN) {
					fprintf(stdout,
					        "%-*s%*s%*s%*s%*s%*s\n",
					        10,
					        "min_rate",
					        FIELD_WIDTH,
					        "max_rate",
					        FIELD_WIDTH,
					        "avg_rate",
					        FIELD_WIDTH,
					        "median_rate",
					        FIELD_WIDTH,
					        "var_rate",
					        FIELD_WIDTH,
					        "std_rate");
				}
				else if (options.format == CSV) {
					fprintf(stdout,
					        "min_rate,max_rate,avg_"
					        "rate,median_rate,var_rate,std_rate\n");
				}
				else if (options.format == RAW_CSV) {
					fprintf(stdout, "count,lat\n");
				}
			}
			else if (options.subtype == PINGPONG) {
				if (options.format == PLAIN) {
					fprintf(stdout,
					        "%-*s%*s%*s%*s%*s%*s\n",
					        10,
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
				}
				else if (options.format == CSV) {
					fprintf(stdout,
					        "min_lat,max_lat,avg_"
					        "lat,median_lat,var_lat,std_lat\n");
				}
				else if (options.format == RAW_CSV) {
					fprintf(stdout, "count,lat\n");
				}
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
				        "%-*s%*s%*s%*s%*s%*s%*s%*s\n",
				        10,
				        "memory_mode",
				        FIELD_WIDTH,
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
				fprintf(stdout,
				        "memory_mode,msg_size,min_bw,max_bw,avg_bw,median_bw,"
				        "var_bw,std_bw\n");
			else if (options.format == RAW_CSV) {
				fprintf(stdout, "msg_size,count,bw\n");
			}
		}
		else if (options.subtype == ALLREDUCE) {
			if (options.format == PLAIN)
				fprintf(stdout,
				        "%-*s%*s%*s%*s%*s%*s%*s%*s%*s\n",
				        10,
				        "#elements",
				        FIELD_WIDTH,
				        "#ranks",
				        FIELD_WIDTH,
				        "#iterations",
				        FIELD_WIDTH,
				        "min_lat",
				        FIELD_WIDTH,
				        "max_lat",
				        FIELD_WIDTH,
				        "avg_lat",
				        FIELD_WIDTH,
				        "media_lat",
				        FIELD_WIDTH,
				        "var_lat",
				        FIELD_WIDTH,
				        "std_lat");
			else if (options.format == CSV)
				fprintf(stdout,
				        "elements,ranks,iterations,min_lat,max_lat,avg_lat,"
				        "median_lat,var_"
				        "lat,std_"
				        "lat\n");
			else if (options.format == RAW_CSV) {
				fprintf(stdout, "ranks,elements,count,lat\n");
			}
		}
		else if (options.subtype == BARRIER) {
			if (options.format == PLAIN)
				fprintf(stdout,
				        "%-*s%*s%*s%*s%*s%*s%*s%*s\n",
				        10,
				        "#ranks",
				        FIELD_WIDTH,
				        "#iterations",
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
				        "ranks,iterations,min_lat,max_lat,avg_lat,median_lat,"
				        "var_lat,std_"
				        "lat\n");
			else if (options.format == RAW_CSV) {
				fprintf(stdout, "ranks,count,lat\n");
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
	else if (options.type == NOTIFY && options.subtype == RATE) {
		for (i = 0; i < n; ++i) {
			t[i] = options.window_size / (t[i] * 1e-9);
		}
	}
	else if (options.subtype == LAT || options.type == COLLECTIVE ||
	         options.subtype == PINGPONG) {
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
			        "%-*s%*d%*.*f%*.*f%*.*f%*.*f%*.*f%*.*f\n",
			        10,
			        options.memory_mode,
			        FIELD_WIDTH,
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
			        "%s,%d,%.*f,%.*f,%.*f,%.*f,%.*f,%.*f\n",
			        options.memory_mode,
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
				fprintf(stdout,
				        "%d,%d,%.*f\n",
				        size,
				        i,
				        FLOAT_PRECISION,
				        measurements.time[i]);
			}
		}
		fflush(stdout);
	}
}

void print_result_coll(const gaspi_rank_t id,
                       const int num_pes,
                       const size_t size,
                       struct measurements_t measurements) {
	int i;
	if (id == 0) {
		struct statistics_t statistics;
		compute_statistics(measurements, &statistics, 0);
		if (options.subtype == ALLREDUCE) {
			if (options.format == PLAIN) {
				fprintf(stdout,
				        "%-*d%*d%*d%*.*f%*.*f%*.*f%*.*f%*.*f%*.*f\n",
				        10,
				        size,
				        FIELD_WIDTH,
				        num_pes,
				        FIELD_WIDTH,
				        options.iterations,
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
				        "%d,%d,%d,%.*f,%.*f,%.*f,%.*f,%.*f,%.*f\n",
				        size,
				        num_pes,
				        options.iterations,
				        FLOAT_PRECISION,
				        statistics.min,
				        FLOAT_PRECISION,
				        statistics.max,
				        FLOAT_PRECISION,
				        statistics.avg,
				        FLOAT_PRECISION,
				        statistics.median,
				        FLOAT_PRECISION,
				        statistics.var,
				        FLOAT_PRECISION,
				        statistics.std);
			}
			else if (options.format == RAW_CSV) {
				for (i = 0; i < measurements.n; ++i) {
					fprintf(stdout,
					        "%d,%d,%d,%.*f\n",
					        size,
					        num_pes,
					        i,
					        FLOAT_PRECISION,
					        measurements.time[i]);
				}
			}
		}
		else if (options.subtype == BARRIER) {
			if (options.format == PLAIN) {
				fprintf(stdout,
				        "%-*d%*d%*.*f%*.*f%*.*f%*.*f%*.*f%*.*f\n",
				        10,
				        num_pes,
				        FIELD_WIDTH,
				        options.iterations,
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
				        "%d,%d,%.*f,%.*f,%.*f,%.*f,%.*f,%.*f\n",
				        num_pes,
				        options.iterations,
				        FLOAT_PRECISION,
				        statistics.min,
				        FLOAT_PRECISION,
				        statistics.max,
				        FLOAT_PRECISION,
				        statistics.avg,
				        FLOAT_PRECISION,
				        statistics.median,
				        FLOAT_PRECISION,
				        statistics.var,
				        FLOAT_PRECISION,
				        statistics.std);
			}
			else if (options.format == RAW_CSV) {
				for (i = 0; i < measurements.n; ++i) {
					fprintf(stdout,
					        "%d,%d,%.*f\n",
					        num_pes,
					        i,
					        FLOAT_PRECISION,
					        measurements.time[i]);
				}
			}
		}
		fflush(stdout);
	}
}

void print_notify_lat(const gaspi_rank_t id,
                      struct measurements_t measurements) {
	struct statistics_t statistics;
	int i;
	if (id == 0) {
		compute_statistics(measurements, &statistics, 0);
		if (options.format == PLAIN) {
			fprintf(stdout,
			        "%-*.*f%*.*f%*.*f%*.*f%*.*f%*.*f\n",
			        10,
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
			        "%.*f,%.*f,%.*f,%.*f,%.*f,%.*f\n",
			        FLOAT_PRECISION,
			        statistics.min,
			        FLOAT_PRECISION,
			        statistics.max,
			        FLOAT_PRECISION,
			        statistics.avg,
			        FLOAT_PRECISION,
			        statistics.median,
			        FLOAT_PRECISION,
			        statistics.var,
			        FLOAT_PRECISION,
			        statistics.std);
		}
		else if (options.format == RAW_CSV) {
			for (i = 0; i < measurements.n; ++i) {
				fprintf(stdout,
				        "%d,%.*f\n",
				        i,
				        FLOAT_PRECISION,
				        measurements.time[i]);
			}
		}
		fflush(stdout);
	}
}

void print_atomic_lat(const gaspi_rank_t id,
                      struct measurements_t measurements) {
	struct statistics_t statistics;
	int i, n;
	if (id == 0) {
		compute_statistics(measurements, &statistics, 0);
		if (options.format == PLAIN) {
			fprintf(stdout,
			        "%-*d%*.*f%*.*f%*.*f%*.*f%*.*f%*.*f\n",
			        10,
			        options.iterations,
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
			        options.iterations,
			        FLOAT_PRECISION,
			        statistics.min,
			        FLOAT_PRECISION,
			        statistics.max,
			        FLOAT_PRECISION,
			        statistics.avg,
			        FLOAT_PRECISION,
			        statistics.median,
			        FLOAT_PRECISION,
			        statistics.var,
			        FLOAT_PRECISION,
			        statistics.std);
		}
		else if (options.format == RAW_CSV) {
			for (i = 0; i < measurements.n; ++i) {
				fprintf(stdout,
				        "%d,%.*f\n",
				        i,
				        FLOAT_PRECISION,
				        measurements.time[i]);
			}
		}
		fflush(stdout);
	}
}
