/**
* @file gbs_ubench_headers.h
* @brief Header file for all microbenchmarks.
* @author MSQC
* @version 2020-11-26
*/

#ifndef __GBS_UBENCH_HEADERS__
#define __GBS_UBENCH_HEADERS__

#include "gbs_benchmark.h"
#include "gbs_microbenchmarks.h"

void gbs_ubench_allreduce_min(gbs_bench_config_t *conf);
void gbs_ubench_allreduce_max(gbs_bench_config_t *conf);
void gbs_ubench_allreduce_sum(gbs_bench_config_t *conf);

void gbs_ubench_atomic_fetch_add_single(gbs_bench_config_t *conf);
void gbs_ubench_atomic_fetch_add_all(gbs_bench_config_t *conf);
void gbs_ubench_atomic_compare_swap_single(gbs_bench_config_t *conf);
void gbs_ubench_atomic_compare_swap_all(gbs_bench_config_t *conf);

void gbs_ubench_barrier(gbs_bench_config_t *conf);

void gbs_ubench_get_all_bdir(gbs_bench_config_t *conf);
void gbs_ubench_get_all_udir(gbs_bench_config_t *conf);
void gbs_ubench_get_single_bdir(gbs_bench_config_t *conf);
void gbs_ubench_get_single_udir(gbs_bench_config_t *conf);
void gbs_ubench_get_single_udir_benchRun(gbs_ubench_data_t *benchData);

void gbs_ubench_put_all_bdir(gbs_bench_config_t *conf);
void gbs_ubench_put_all_udir(gbs_bench_config_t *conf);
void gbs_ubench_put_single_bdir(gbs_bench_config_t *conf);
void gbs_ubench_put_single_udir(gbs_bench_config_t *conf);
void gbs_ubench_put_single_udir_benchRun(gbs_ubench_data_t *benchData);

void gbs_ubench_put_solo_single_udir(gbs_bench_config_t *conf);
void gbs_ubench_put_aggregate_single_udir(gbs_bench_config_t *conf);

void gbs_ubench_noti_single_udir(gbs_bench_config_t *conf);
void gbs_ubench_noti_single_bdir(gbs_bench_config_t *conf);
void gbs_ubench_noti_all_udir(gbs_bench_config_t *conf);
void gbs_ubench_noti_all_bdir(gbs_bench_config_t *conf);

void gbs_ubench_ping_pong(gbs_bench_config_t *conf);

void gbs_ubench_twosided_ping_pong(gbs_bench_config_t *conf);

void gbs_ubench_put_true_exchange(gbs_bench_config_t *conf);
void gbs_ubench_get_true_exchange(gbs_bench_config_t *conf);


#endif
