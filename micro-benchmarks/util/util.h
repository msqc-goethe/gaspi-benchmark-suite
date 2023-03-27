#ifndef __UTIL_H__
#define __UTIL_H__

#include <GASPI.h>
#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef FIELD_WIDTH
#define FIELD_WIDTH 20
#endif

#ifndef FLOAT_PRECISION
#define FLOAT_PRECISION 2
#endif

enum options_ret_type { OPTIONS_OKAY = 0, OPTIONS_HELP, OPTIONS_BAD_USAGE };

enum benchmark_type { COLLECTIVE = 0, PASSIVE, ONESIDED, ATOMIC };

enum benchmark_subtype { BW = 0, LAT, ALLREDUCE, BARRIER };

enum output_format { PLAIN = 0, CSV };

struct bad_usage_t {
	char const* message;
	char const* optarg;
	int opt;
};

struct benchmark_options_t {
	enum benchmark_type type;
	enum benchmark_subtype subtype;
	enum output_format format;

	int num_threads;
	int sender_thread;
	int num_processes;
	int segment_size;
	int verbose;
	int window_size;
	int iterations;
	int skip;

	size_t min_message_size;
	size_t max_message_size;
	size_t max_mem_limit;
	size_t warmup_validation;

	char* src;
	char* dst;
};

int benchmark_options(int argc, char* argv[]);
void print_header(const gaspi_rank_t id);
void print_bad_usage(const gaspi_rank_t id);
void print_help_message(const gaspi_rank_t id);
void print_result_bw(const gaspi_rank_t id,
                     const double time,
                     const size_t size);
void print_result_lat(const gaspi_rank_t id,
                      const double time,
                      const size_t size);
void print_result_coll(const gaspi_rank_t id,
                       const int num_pes,
                       const size_t size,
                       const double min_time,
                       const double avg_time,
                       const double max_time);
void print_atomic_lat(const gaspi_rank_t id,
                         const char old_value,
                         const char new_value,
                         const double time);
extern struct benchmark_options_t options;
extern struct bad_usage_t bad_usage;
#endif