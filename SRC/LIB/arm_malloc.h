#ifndef ARM_MALLOC_H
#define ARM_MALLOC_H

#include <stddef.h>
#include <stdint.h>

// 内存块对齐要求（ARMv8 通常需要 16 字节对齐）
#define ALIGNMENT 16
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

// 内存块元数据结构
typedef struct block_meta {
    size_t size;            // 块大小（包括元数据）
    struct block_meta *next; // 下一个空闲块
    struct block_meta *prev; // 上一个空闲快
    int free;               // 空闲标志
    // 注意：在64位系统上，这个结构体大小为32字节（16字节对齐）
} block_meta;

#define META_SIZE ALIGN(sizeof(block_meta))

// 内存分配函数
void *arm_malloc(size_t size);
void arm_free(void *ptr);
void *arm_calloc(size_t num, size_t size);
void *arm_realloc(void *ptr, size_t size);

// 堆管理函数
void arm_malloc_init(void *heap_start, size_t heap_size);
void arm_malloc_stats();

#endif // ARM_MALLOC_H