#include <am.h>
#include <klib.h>
#include <klib-macros.h>

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
    struct malloc_block_head *pre = NULL, *next = NULL;
    size_t size = 0;
} malloc_block_head;
static malloc_block_head* first_block = NULL;
static char malloc_buffer[64 * 1024 * 1024];

static malloc_block_head* alloc_block(size_t size) {
    size_t relSize = size + sizeof(malloc_block_head);
    if (first_block == NULL) {
        if (relSize > sizeof(malloc_buffer)) return NULL;
        first_block = (malloc_block_head*)malloc_buffer;
        first_block->size = size;
        return first_block;
    }
    if ((char*)first_block - malloc_buffer >= relSize) {
        malloc_block_head* new_block = malloc_buffer;
        new_block->next = first_block;
        new_block->size = size;
        first_block->pre = new_block;
        first_block = new_block;
        return first_block;
    }
    malloc_block_head* block = first_block;
    while (block != NULL) {
        char* block_end = (char*)block + sizeof(malloc_block_head) + block->size;
        char* region_end = block->next == NULL ? malloc_buffer + sizeof(malloc_buffer) : block->next;
        size_t empty = region_end - block_end;
        if (empty >= relSize) {
            malloc_block_head* new_block = (malloc_block_head*) block_end;
            new_block->pre = block;
            new_block->next = block->next;
            block->next = new_block;
            if (block->next != NULL) block->next->pre = new_block;
            new_block->size = size;
            return new_block;
        }
        block = block->next;
    }
    return NULL;
}

void free_block(malloc_block_head* block) {
    malloc_block_head* block_prev = block->pre;
    malloc_block_head* block_next = block->next;
    if (block_next != NULL) block_next->pre = block_prev;
    if (block_prev != NULL) block_prev->next = block_next;
    if (block == first_block) first_block = block_next;
}

void *malloc(size_t size) {
    malloc_block_head* tmp = alloc_block(size);
    if (tmp == NULL) return NULL;
    return tmp + sizeof(malloc_block_head);
}

void free(void *ptr) {
    free_block(ptr - sizeof(malloc_block_head));
}

#endif
