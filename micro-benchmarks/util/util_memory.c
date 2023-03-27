#include "util_memory.h"
#include "check.h"

void allocate_gaspi_memory(const gaspi_segment_id_t id,
                           const size_t size,
                           const char c) {
	GASPI_CHECK(gaspi_segment_create(
	    id, size, GASPI_GROUP_ALL, GASPI_BLOCK, GASPI_MEM_UNINITIALIZED));
	void* ptr;
	GASPI_CHECK(gaspi_segment_ptr(id, &ptr));
	memset(ptr, c, size);
	GASPI_CHECK(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK));
}

void free_gaspi_memory(const gaspi_segment_id_t id) {
	GASPI_CHECK(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK));
	GASPI_CHECK(gaspi_segment_delete(id));
}

void allocate_memory(void** buffer, const size_t size) {
	int r;
	*buffer = malloc(size);
	size_t alignment = sysconf(_SC_PAGESIZE);
	r = posix_memalign(buffer, alignment, size);
	if (r < 0) {
		fprintf(stderr, "posix_memalign failed with %i", r);
		exit(EXIT_FAILURE);
	}
}

void free_memory(void* buffer) {
	free(buffer);
}
