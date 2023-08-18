/* -*- mode: C; tab-width: 2; indent-tabs-mode: nil; -*- */

/*
 * This code has been contributed by the DARPA HPCS program.  Contact
 * David Koester <dkoester@mitre.org> or Bob Lucas <rflucas@isi.edu>
 * if you have questions.
 *
 *
 * GUPS (Giga UPdates per Second) is a measurement that profiles the memory
 * architecture of a system and is a measure of performance similar to MFLOPS.
 * The HPCS HPCchallenge RandomAccess benchmark is intended to exercise the
 * GUPS capability of a system, much like the LINPACK benchmark is intended to
 * exercise the MFLOPS capability of a computer.  In each case, we would
 * expect these benchmarks to achieve close to the "peak" capability of the
 * memory system. The extent of the similarities between RandomAccess and
 * LINPACK are limited to both benchmarks attempting to calculate a peak system
 * capability.
 *
 * GUPS is calculated by identifying the number of memory locations that can be
 * randomly updated in one second, divided by 1 billion (1e9). The term "randomly"
 * means that there is little relationship between one address to be updated and
 * the next, except that they occur in the space of one half the total system
 * memory.  An update is a read-modify-write operation on a table of 64-bit words.
 * An address is generated, the value at that address read from memory, modified
 * by an integer operation (add, and, or, xor) with a literal value, and that
 * new value is written back to memory.
 *
 * We are interested in knowing the GUPS performance of both entire systems and
 * system subcomponents --- e.g., the GUPS rating of a distributed memory
 * multiprocessor the GUPS rating of an SMP node, and the GUPS rating of a
 * single processor.  While there is typically a scaling of FLOPS with processor
 * count, a similar phenomenon may not always occur for GUPS.
 *
 *
 */

#define _GNU_SOURCE //needed to get sched_getcpu to work since glibc 2.14
#include <hpcc.h>
#include <sched.h>
#include <stdio.h>
#include "GASPI.h"
#include "RandomAccess.h"
#include "check.h"

#define MAXTHREADS 256
#define BARRIER_ALL() GASPI_CHECK(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK))
#define FLUSH() GASPI_CHECK(gaspi_wait(gaspi_queue_id, GASPI_BLOCK))
void do_abort(char* f) {
	fprintf(stderr, "%s\n", f);
}

u64Int srcBuf[] = {0xb1ffd1da};
u64Int targetBuf[sizeof(srcBuf) / sizeof(u64Int)];

/* Allocate main table (in global memory) */
u64Int* HPCC_Table;

int main(int argc, char** argv) {
	int debug = 0;

	s64Int i;
	gaspi_rank_t NumProcs, logNumProcs, MyProc;
	const gaspi_segment_id_t gaspi_segment_id_table = 0;
	const gaspi_queue_id_t gaspi_queue_id = 0;

	u64Int GlobalStartMyProc;
	u64Int Top; /* Number of table entries in top of Table */
	s64Int LocalTableSize; /* Local table width */
	u64Int MinLocalTableSize; /* Integer ratio TableSize/NumProcs */
	u64Int logTableSize, TableSize;

	double CPUTime; /* CPU  time to update table */
	double RealTime; /* Real time to update table */

	double TotalMem;
	int PowerofTwo;

	double timeBound = -1; /* OPTIONAL time bound for execution time */
	u64Int
	    NumUpdates_Default; /* Number of updates to table (suggested: 4x number of table entries) */
	u64Int
	    NumUpdates; /* actual number of updates to table - may be smaller than
                       * NumUpdates_Default due to execution time bounds */
	s64Int ProcNumUpdates; /* number of updates per processor */
#ifdef RA_TIME_BOUND
	s64Int GlbNumUpdates; /* for reduction */
#endif

	FILE* outFile = NULL;
	double* GUPs;

	int numthreads;
	int *sAbort, *rAbort;

	GASPI_CHECK(gaspi_proc_init(GASPI_BLOCK));
	GASPI_CHECK(gaspi_proc_rank(&MyProc));
	GASPI_CHECK(gaspi_proc_num(&NumProcs));

	/*Allocate symmetric memory*/
	sAbort = (int*) malloc(sizeof(int));
	rAbort = (int*) malloc(sizeof(int));
	GUPs = (double*) malloc(sizeof(double));

	*GUPs = -1;

	if (0 == MyProc) {
		outFile = stdout;
		setbuf(outFile, NULL);
	}

	TotalMem = 20000000; /* max single node memory */
	TotalMem *= NumProcs; /* max memory in NumProcs nodes */

	TotalMem /= sizeof(u64Int);

	/* calculate TableSize --- the size of update array (must be a power of 2) */
	for (TotalMem *= 0.5, logTableSize = 0, TableSize = 1; TotalMem >= 1.0;
	     TotalMem *= 0.5, logTableSize++, TableSize <<= 1)
		; /* EMPTY */

	MinLocalTableSize = (TableSize / NumProcs);
	LocalTableSize = MinLocalTableSize;
	GlobalStartMyProc = (MinLocalTableSize * MyProc);

	*sAbort = 0;

	/*alloc HPCC_Table for RMA*/
	GASPI_CHECK(gaspi_segment_create(gaspi_segment_id_table,
	                                 sizeof(u64Int) * LocalTableSize,
	                                 GASPI_GROUP_ALL,
	                                 GASPI_BLOCK,
	                                 GASPI_MEM_INITIALIZED));
	GASPI_CHECK(
	    gaspi_segment_ptr(gaspi_segment_id_table, (void**) &HPCC_Table));

	if (!HPCC_Table)
		*sAbort = 1;
	BARRIER_ALL();
	GASPI_CHECK(gaspi_allreduce(sAbort,
	                            rAbort,
	                            1,
	                            GASPI_OP_SUM,
	                            GASPI_TYPE_INT,
	                            GASPI_GROUP_ALL,
	                            GASPI_BLOCK));
	BARRIER_ALL();
	if (*rAbort > 0) {
		if (MyProc == 0)
			fprintf(outFile, "Failed to allocate memory for the main table.\n");
		/* check all allocations in case there are new added and their order changes */
		if (HPCC_Table)
			GASPI_CHECK(gaspi_segment_delete(gaspi_segment_id_table));
		goto failed_table;
	}

	/* Default number of global updates to table: 4x number of table entries */
	NumUpdates_Default = 4 * TableSize;
	ProcNumUpdates = 4 * LocalTableSize;
	NumUpdates = NumUpdates_Default;

	if (MyProc == 0) {
		fprintf(outFile,
		        "Running on %d processors%s\n",
		        NumProcs,
		        PowerofTwo ? " (PowerofTwo)" : "");
		fprintf(outFile,
		        "Total Main table size = 2^" FSTR64 " = " FSTR64 " words\n",
		        logTableSize,
		        TableSize);
		if (PowerofTwo)
			fprintf(outFile,
			        "PE Main table size = 2^" FSTR64 " = " FSTR64 " words/PE\n",
			        (logTableSize - logNumProcs),
			        TableSize / NumProcs);
		else
			fprintf(outFile,
			        "PE Main table size = (2^" FSTR64 ")/%d  = " FSTR64
			        " words/PE MAX\n",
			        logTableSize,
			        NumProcs,
			        LocalTableSize);

		fprintf(outFile,
		        "Default number of updates (RECOMMENDED) = " FSTR64
		        "\tand actually done = %d\n",
		        NumUpdates_Default,
		        ProcNumUpdates * NumProcs);
	}

	/* Initialize main table */
	for (i = 0; i < LocalTableSize; i++)
		HPCC_Table[i] = MyProc;

	BARRIER_ALL();

	int j, k;
	int logTableLocal, ipartner, iterate, niterate;
	int ndata, nkeep, nsend, nrecv, index, nlocalm1;
	int numthrds;
	u64Int datum, procmask;
	u64Int *data, *send;
	void* tstatus;
	int remote_proc, offset;
	u64Int* tb;
	s64Int remotecount;
	int thisPeId;
	int numNodes;
	const gaspi_segment_id_t gaspi_segment_id_updates = 1;
	const gaspi_segment_id_t gaspi_segment_id_remote_val = 2;
	s64Int* count;
	s64Int* updates;
	s64Int old_val;
	s64Int* ran;

	thisPeId = MyProc;
	numNodes = NumProcs;
	/* An array of length npes to avoid overwrites*/
	GASPI_CHECK(gaspi_segment_create(gaspi_segment_id_updates,
	                                 sizeof(s64Int) * numNodes,
	                                 GASPI_GROUP_ALL,
	                                 GASPI_BLOCK,
	                                 GASPI_MEM_INITIALIZED));
	GASPI_CHECK(gaspi_segment_ptr(gaspi_segment_id_updates, (void**) &updates));

	count = (s64Int*) malloc(sizeof(s64Int));
	s64Int* all_updates = (s64Int*) malloc(numNodes * sizeof(s64Int));
	ran = (s64Int*) malloc(sizeof(s64Int));
	*ran = starts(4 * GlobalStartMyProc);

	niterate = ProcNumUpdates;
	logTableLocal = logTableSize - logNumProcs;
	nlocalm1 = LocalTableSize - 1;
	// Likely not needless due to GASPI_MEM_INITIALIZED
	for (j = 0; j < numNodes; j++) {
		updates[j] = 0;
		all_updates[j] = 0;
	}
	int verify = 1;

	u64Int* remote_val;
	GASPI_CHECK(gaspi_segment_create(gaspi_segment_id_remote_val,
	                                 sizeof(u64Int),
	                                 GASPI_GROUP_ALL,
	                                 GASPI_BLOCK,
	                                 GASPI_MEM_INITIALIZED));
	GASPI_CHECK(
	    gaspi_segment_ptr(gaspi_segment_id_remote_val, (void**) &remote_val));

	BARRIER_ALL();
	/* Begin timed section */
	RealTime = -RTSEC();
	for (iterate = 0; iterate < niterate; iterate++) {
		*ran = (*ran << 1) ^ ((s64Int) *ran < ZERO64B ? POLY : ZERO64B);
		remote_proc = (*ran >> logTableLocal) & (numNodes - 1);

		/*Forces updates to remote PE only*/
		if (remote_proc == MyProc)
			remote_proc = (remote_proc + 1) / numNodes;
		GASPI_CHECK(gaspi_read(gaspi_segment_id_remote_val,
		                       0,
		                       remote_proc,
		                       gaspi_segment_id_table,
		                       *ran & (LocalTableSize - 1),
		                       1,
		                       gaspi_queue_id,
		                       GASPI_BLOCK));
		*remote_val ^= *ran;
		GASPI_CHECK(gaspi_write(gaspi_segment_id_table,
		                        *ran & (LocalTableSize - 1),
		                        remote_proc,
		                        gaspi_segment_id_remote_val,
		                        0,
		                        1,
		                        gaspi_queue_id,
		                        GASPI_BLOCK));
		FLUSH();
		if (verify) {
			GASPI_CHECK(gaspi_atomic_fetch_add(gaspi_segment_id_updates,
			                                   thisPeId * sizeof(s64Int),
			                                   remote_proc,
			                                   1,
			                                   (gaspi_atomic_value_t*) &old_val,
			                                   GASPI_BLOCK));
		}
	}

	BARRIER_ALL();
	/* End timed section */
	RealTime += RTSEC();

	/* Print timing results */
	if (MyProc == 0) {
		*GUPs = 1e-9 * NumUpdates / RealTime;
		fprintf(outFile, "Real time used = %.6f seconds\n", RealTime);
		fprintf(outFile,
		        "%.9f Billion(10^9) Updates    per second [GUP/s]\n",
		        *GUPs);
		fprintf(outFile,
		        "%.9f Billion(10^9) Updates/PE per second [GUP/s]\n",
		        *GUPs / NumProcs);
	}

	if (verify) {
		s64Int loc_updates = 0;
		for (j = 1; j < numNodes; j++)
			loc_updates += updates[j];
		int cpu = sched_getcpu();
		printf("PE%d CPU%d  updates:%d\n", MyProc, cpu, loc_updates);
		GASPI_CHECK(gaspi_allreduce(updates,
		                            all_updates,
		                            numNodes,
		                            GASPI_OP_SUM,
		                            GASPI_TYPE_LONG,
		                            GASPI_GROUP_ALL,
		                            GASPI_BLOCK));
		if (MyProc == 0) {
			for (j = 1; j < numNodes; j++)
				all_updates[0] += all_updates[j];
			if (ProcNumUpdates * NumProcs == all_updates[0])
				printf("Verification passed!\n");
			else
				printf("Verification failed!\n");
		}
	}
	BARRIER_ALL();
	/* End verification phase */
	free(count);
	free(ran);
	/* Deallocate memory (in reverse order of allocation which should
 *      help fragmentation) */
	GASPI_CHECK(gaspi_segment_delete(gaspi_segment_id_remote_val));
	GASPI_CHECK(gaspi_segment_delete(gaspi_segment_id_updates));
	GASPI_CHECK(gaspi_segment_delete(gaspi_segment_id_table));
	BARRIER_ALL();
failed_table:

	if (0 == MyProc)
		if (outFile != stderr)
			fclose(outFile);
	free(sAbort);
	free(rAbort);

	return 0;
}

/* Utility routine to start random number generator at Nth step */
s64Int starts(u64Int n) {
	/* s64Int i, j; */
	int i, j;
	u64Int m2[64];
	u64Int temp, ran;

	while (n < 0)
		n += PERIOD;
	while (n > PERIOD)
		n -= PERIOD;
	if (n == 0)
		return 0x1;

	temp = 0x1;
	for (i = 0; i < 64; i++) {
		m2[i] = temp;
		temp = (temp << 1) ^ ((s64Int) temp < 0 ? POLY : 0);
		temp = (temp << 1) ^ ((s64Int) temp < 0 ? POLY : 0);
	}

	for (i = 62; i >= 0; i--)
		if ((n >> i) & 1)
			break;

	ran = 0x2;

	while (i > 0) {
		temp = 0;
		for (j = 0; j < 64; j++)
			if ((ran >> j) & 1)
				temp ^= m2[j];
		ran = temp;
		i -= 1;
		if ((n >> i) & 1)
			ran = (ran << 1) ^ ((s64Int) ran < 0 ? POLY : 0);
	}

	return ran;
}
