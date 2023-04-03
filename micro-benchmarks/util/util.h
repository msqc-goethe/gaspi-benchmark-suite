#ifndef __UTIL_H__
#define __UTIL_H__

#include <GASPI.h>
#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef FIELD_WIDTH
#define FIELD_WIDTH 20
#endif

#ifndef FLOAT_PRECISION
#define FLOAT_PRECISION 2
#endif

enum options_ret_type { OPTIONS_OKAY = 0, OPTIONS_HELP, OPTIONS_BAD_USAGE };

enum benchmark_type { COLLECTIVE = 0, PASSIVE, ONESIDED, ATOMIC };

enum benchmark_subtype { BW = 0, LAT, ALLREDUCE, BARRIER };

enum output_format { PLAIN = 0, CSV, RAW_CSV };

struct measurements_t {
	double* time;
	int n;
};

struct statistics_t {
	double min;
	double max;
	double avg;
	double median;
	double std;
	double var;
};

struct bad_usage_t {
	char const* message;
	char const* optarg;
	int opt;
};

struct benchmark_options_t {
	enum benchmark_type type;
	enum benchmark_subtype subtype;
	enum output_format format;

	int verbose;
	int window_size;
	int iterations;
	int skip;

	size_t min_message_size;
	size_t max_message_size;
};

int benchmark_options(int argc, char* argv[]);
void print_header(const gaspi_rank_t id);
void print_bad_usage(const gaspi_rank_t id);
void print_help_message(const gaspi_rank_t id);
void print_result(const gaspi_rank_t id,
                  struct measurements_t timings,
                  const size_t size);
void print_result_coll(const gaspi_rank_t id,
                       const int num_pes,
                       const size_t size,
                       struct measurements_t measurements);
void print_atomic_lat(const gaspi_rank_t id,
                      char const* old_value,
                      char const* new_value,
                      struct measurements_t measurements);

extern struct benchmark_options_t options;
extern struct bad_usage_t bad_usage;
#endif
