/**
* @file gbs_abench_grid.h
* @brief 2D 5-point stencil Halo-Exchange application benchmark for GASPI.
* @detail This benchmark computes a 5-point stencil with halo-exchange
*		  using GASPI. The heat distribution on a 2D plane is evaluted.
*		  The computation is split over multiple nodes with GASPI and
*		  additionally multithreaded using pthreads.
* @author MSQC
* @version 2020-11-25
*/
#ifndef __GBS_ABENCH_GRID_H__
#define __GBS_ABENCH_GRID_H__

#include <stdio.h>
#include <utility.h>
#include <GASPI.h>
#include "gbs_benchmark.h"


/**
* Data type for one data element in the grid.
* Can be changed via defining '-DGRID_FLOAT'
* during compilation.
*/
#ifdef GRID_FLOAT
typedef float gridData_t;
#else
typedef double gridData_t;
#endif

/**
* Represents a grid with the length of one side (size) and
* a pointer to the data array.
* The grid is allocated larger to include one halo row/column at
* each side of the grid (realSize)
*/
typedef struct grid_s {
    gridData_t *data;
    unsigned int size;
    unsigned int realSize;
} grid_t;

/**
* Represents a Shard (a distributed grid). When distributing
* the grid in shards that are shared over different ranks each
* shard has a constant number of columns which corresponds to the
* number of columns in the grid.
* The number of rows may differ among shards.
* The shard is allocated in the global address space and has
* a memory segment of its own.
*/
typedef struct gridShard_s {
    gridData_t *data;
	gaspi_segment_id_t sid;
    unsigned int rows;
    unsigned int cols;
} gridShard_t;

/**
* Allocates memory for a grid and initializes the meta data. The
* elements on the grid are NOT initialized.
*
* @param gridSize Dimension of the full grid. Number of usable rows/cols.
* @return Pointer to a newly allocated grid.
*/
extern grid_t *grid_allocGrid(unsigned int gridSize);

/**
* Alloziert Speicher fuer einen Shard und initialisiert diesen NICHT. Die Berechnung
* der Groesse erfolgt anhand der Anzahl der Worker-Ranks und der jeweiligen Rank-ID.
*
* @param rank Rank des jeweiligen Prozesses, der eine Zuteilung erhalten soll.
* @param workerProcs Anzahl der an der Berechnung beteiligten Prozessen.
* @return Zeiger auf das Shard-Objekt.
*/
/**
* Allocates a memory segment for a shard and initializes the meta data.
* The data points in the shard are NOT initialized. The shard dimensions depend
* on the rank process number.
*
* @param gridSize Dimension of the full grid. Number of usable rows/cols.
* @param iproc The number of this GASPI rank.
* @param nproc The total number of all GASPI ranks.
* @return Pointer to the allocated shard object.
*/
extern gridShard_t *grid_allocShard(unsigned int gridSize, unsigned int iproc, int nproc);


/**
* Initializes a shard with default values.
* All values are set to zero except for a circle of a certain radius.
* The initialization is done in a way that a "hot spot" in the
* center of the full grid is created that can be visualized nicely.
*
* @param shard The shard object to be initialized.
* @param iproc The number of this GASPI rank.
* @param nproc The total number of all GASPI ranks.
* @param radius The radius on the full grid in which the circle should
*		 be created.
* @param The temperature value that is set in the circle.
*/
extern void grid_initializeShard(gridShard_t *shard, unsigned int iproc, unsigned int nproc, double radius, double value);

/**
* Prints the given grid on stdout.
* All values are separated by a TAB and distinct
* rows are separated by a newline. The first value
* corresponds to the top-right value on the grid.
*
* @param grid Pointer to the grid that shall be printed.
*/
extern void grid_printGrid(grid_t *grid);

/**
* Writes a shard to stdout.
* All values are separated by a TAB and distinct
* rows are separated by a newline.
*
* @param shard Pointer to the shard that shall be printed.
*/
extern void grid_printShard(gridShard_t *shard);

/**
* De-Allocates the resources of the given grid.
*
* @param grid Pointer to the grid that shall be freed.
* @return NULL.
*/
extern grid_t *grid_freeGrid(grid_t *grid);

/**
* De-Allocates the resources of the given shad.
*
* @param shard Pointer to the shard.
* @return NULL.
*/
extern gridShard_t *grid_freeShard(gridShard_t *shard);

/**
* Benchmarking function for the GBS suite. Executes the
* grid benchmark for the given grid sizes, etc. and
* prints the result to the supplied stream.
*
* @param conf The benchmark config with the command line arguments.
*/
extern void gbs_abench_grid_stencil(gbs_bench_config_t *conf);

#endif
