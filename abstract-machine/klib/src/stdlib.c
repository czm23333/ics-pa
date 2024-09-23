#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdalign.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static unsigned long int next = 1;

int rand(void) {
    // RAND_MAX assumed to be 32767
    next = next * 1103515245 + 12345;
    return (unsigned int) (next / 65536) % 32768;
}

void srand(unsigned int seed) {
    next = seed;
}

int abs(int x) {
    return (x < 0 ? -x : x);
}

int atoi(const char *nptr) {
    int x = 0;
    while (*nptr == ' ') { nptr++; }
    while (*nptr >= '0' && *nptr <= '9') {
        x = x * 10 + *nptr - '0';
        nptr++;
    }
    return x;
}

typedef struct malloc_block_head {
    struct malloc_block_head *pre, *next;
    size_t size;
} malloc_block_head;

static malloc_block_head *first_block = NULL;
static char malloc_buffer[64 * 1024 * 1024];

static void *aligned_address(void *addr, size_t alignment) {
    return (void *) (((uintptr_t) addr + alignment - 1) & ~(alignment - 1));
}

static void *aligned_header_address(void *addr, size_t alignment) {
    return aligned_address(addr + sizeof(malloc_block_head), alignment) - sizeof(malloc_block_head);
}

static malloc_block_head *alloc_block(size_t size, size_t alignment) {
    if (alignment < alignof(malloc_block_head)) alignment = alignof(malloc_block_head);

    size_t relSize = size + sizeof(malloc_block_head);
    if (first_block == NULL) {
        char *aligned = aligned_header_address(malloc_buffer, alignment);
        if (relSize > sizeof(malloc_buffer) - (aligned - malloc_buffer)) return NULL;
        first_block = (malloc_block_head *) aligned;
        first_block->pre = NULL;
        first_block->next = NULL;
        first_block->size = size;
        return first_block;
    }

    size_t validSize = sizeof(malloc_buffer) + 1;
    malloc_block_head *pre_block = NULL, *cur_block = NULL, *next_block = NULL;

    do {
        char *aligned = aligned_header_address(malloc_buffer, alignment);
        if (aligned > (char *) first_block) break;
        size_t curSize = (char *) first_block - aligned;
        if (curSize >= relSize && curSize < validSize) {
            validSize = curSize;
            pre_block = NULL;
            cur_block = (malloc_block_head *) aligned;
            next_block = first_block;
        }
    } while (false);

    for (malloc_block_head *block = first_block; block != NULL; block = block->next) {
        char *block_end = aligned_header_address((char *) block + sizeof(malloc_block_head) + block->size, alignment);
        char *region_end = block->next == NULL ? malloc_buffer + sizeof(malloc_buffer) : (char *) block->next;
        if (block_end > region_end) continue;
        size_t empty = region_end - block_end;
        if (empty >= relSize && empty < validSize) {
            validSize = empty;
            pre_block = block;
            cur_block = (malloc_block_head *) block_end;
            next_block = block->next;
        }
    }

    if (cur_block == NULL) return NULL;
    cur_block->pre = pre_block;
    cur_block->next = next_block;
    if (pre_block != NULL) pre_block->next = cur_block;
    if (next_block != NULL) next_block->pre = cur_block;
    cur_block->size = size;
    if (next_block == first_block) first_block = cur_block;
    return cur_block;
}

void free_block(malloc_block_head *block) {
    malloc_block_head *block_prev = block->pre;
    malloc_block_head *block_next = block->next;
    if (block_next != NULL) block_next->pre = block_prev;
    if (block_prev != NULL) block_prev->next = block_next;
    if (block == first_block) first_block = block_next;
}

void *malloc(size_t size) {
    malloc_block_head *tmp = alloc_block(size, alignof(max_align_t));
    if (tmp == NULL) return NULL;
    return (char *) tmp + sizeof(malloc_block_head);
}

static bool is_power_of_two(size_t x) {
    return x && (x & (x - 1)) == 0;
}

void *aligned_alloc(size_t alignment, size_t size) {
    if (!is_power_of_two(alignment)) return NULL;
    malloc_block_head *tmp = alloc_block(size, alignment);
    if (tmp == NULL) return NULL;
    return (char *) tmp + sizeof(malloc_block_head);
}

void free(void *ptr) {
    free_block(ptr - sizeof(malloc_block_head));
}

#endif
