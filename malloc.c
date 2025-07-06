#include <assert.h>
#include <stdio.h>
#include <unistd.h>

void *global_base = NULL;

struct block_meta {
	size_t size;
	struct block_meta *next;
	int free;
};

#define META_SIZE sizeof(struct block_meta)

struct block_meta *allocate_block(struct block_meta *last, size_t size) {
	struct block_meta *block = sbrk(0);

	void *request = sbrk(META_SIZE + size);

	assert((void *)block == request);

	if (request == (void *)-1) {
		return NULL; // sbrk failed
	}

	if (last) {
		last->next = block;
	}

	block->size = size;
	block->free = 0;
	block->next = NULL;

	return block;
}

struct block_meta *find_free_block(struct block_meta **last, size_t size) {
	struct block_meta *current = global_base;

	while (current && !(current->free && current->size >= size)) {
		*last = current;
		current = current->next;
	}

	return current;
}

void *malloc(size_t size) {
	struct block_meta *block;

	if (size <= 0) {
		return NULL;
	}
	
	if (!global_base) { // only in the first malloc call.
		block = allocate_block(NULL, size);
		if (!block) {
			return NULL;
		}
		global_base = block;
	} else {
		struct block_meta *last = global_base;
		block = find_free_block(&last, size);
		if (!block) { // failed to find free block.
			block = allocate_block(last, size);
			if (!block) {
				return NULL;
			}
		} else {
			block->free = 0;
		}
	}
	
	return (block+1);
}

struct block_meta *get_block_ptr(void *ptr) {
	return ((struct block_meta *)ptr - 1);
}

void free(void *ptr) {
	if (!ptr) {
		return;
	}

	struct block_meta *block_ptr = get_block_ptr(ptr);
	assert(block_ptr->free == 0);
	block_ptr->free = 1;
}
