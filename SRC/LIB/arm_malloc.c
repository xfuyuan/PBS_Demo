#include "arm_malloc.h"
#include <string.h>

// 使用静态变量管理堆
static void *heap_start = NULL;
static size_t heap_size = 0;
static block_meta *free_list = NULL;

// 初始化堆
void arm_malloc_init(void *start, size_t size) {
    heap_start = start;
    heap_size = size;
    
    // 初始化整个堆为一个大的空闲块
    block_meta *first_block = (block_meta *)heap_start;
    first_block->size = heap_size;
    first_block->next = NULL;
    first_block->free = 1;
    
    free_list = first_block;
}

// 分割内存块
static void split_block(block_meta *block, size_t size) {
    if (block->size > size + META_SIZE + ALIGNMENT) {
        // 计算新块的位置
        block_meta *new_block = (block_meta *)((char *)block + META_SIZE + size);
        
        // 设置新块的元数据
        new_block->size = block->size - size - META_SIZE;
        new_block->free = 1;
        new_block->next = block->next;
        
        // 更新原块的大小和指针
        block->size = size;
        block->next = new_block;
    }
}

// 合并相邻的空闲块
static void coalesce_blocks() {
    block_meta *current = free_list;
    while (current != NULL && current->next != NULL) {
        // 检查当前块和下一个块是否相邻且都是空闲的
        if ((char *)current + META_SIZE + current->size == (char *)current->next) {
            // 合并块
            current->size += META_SIZE + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

// 寻找合适的空闲块（首次适应算法）
static block_meta *find_free_block(size_t size) {
    block_meta *current = free_list;
    block_meta *best = NULL;
    
    // 使用首次适应算法
    while (current != NULL) {
        if (current->free && current->size >= size) {
            // 如果找到合适的块，分割它
            split_block(current, size);
            current->free = 0;
            return current;
        }
        current = current->next;
    }
    
    return NULL; // 没有找到合适的块
}

// 定义不同大小的内存桶
#define NUM_BUCKETS 10
static block_meta *free_buckets[NUM_BUCKETS] = {NULL};

// 根据大小确定桶索引
static int get_bucket_index(size_t size) {
    int index = 0;
    size_t min_size = 16; // 最小块大小
    
    while (min_size < size && index < NUM_BUCKETS - 1) {
        min_size *= 2;
        index++;
    }
    
    return index;
}

// 优化后的查找函数
static block_meta *find_free_block_optimized(size_t size) {
    int bucket_index = get_bucket_index(size);
    
    // 在当前及更大的桶中查找
    for (int i = bucket_index; i < NUM_BUCKETS; i++) {
        block_meta *current = free_buckets[i];
        while (current != NULL) {
            if (current->free && current->size >= size) {
                // 从原桶中移除
                if (current->prev != NULL) {
                    current->prev->next = current->next;
                } else {
                    free_buckets[i] = current->next;
                }
                if (current->next != NULL) {
                    current->next->prev = current->prev;
                }
                
                // 分割块
                split_block(current, size);
                current->free = 0;
                
                // 将剩余部分放回合适的桶
                if (current->next != NULL && current->next->free) {
                    block_meta *remaining = current->next;
                    int rem_bucket = get_bucket_index(remaining->size);
                    remaining->next = free_buckets[rem_bucket];
                    if (free_buckets[rem_bucket] != NULL) {
                        free_buckets[rem_bucket]->prev = remaining;
                    }
                    remaining->prev = NULL;
                    free_buckets[rem_bucket] = remaining;
                }
                
                return current;
            }
            current = current->next;
        }
    }
    
    return NULL; // 没有找到合适的块
}

// malloc 实现
void *arm_malloc(size_t size) {
    if (size == 0 || heap_start == NULL) {
        return NULL;
    }
    
    // 对齐请求的大小
    size_t aligned_size = ALIGN(size);
    
    // 寻找合适的空闲块
    block_meta *block = find_free_block(aligned_size);
    if (block != NULL) {
        // 返回块的数据部分（跳过元数据）
        return (void *)((char *)block + META_SIZE);
    }
    
    // 没有可用的空闲块
    return NULL;
}

// free 实现
void arm_free(void *ptr) {
    if (ptr == NULL || heap_start == NULL) {
        return;
    }
    
    // 获取块的元数据
    block_meta *block = (block_meta *)((char *)ptr - META_SIZE);
    
    // 标记块为空闲
    block->free = 1;
    
    // 将块添加到空闲链表
    block->next = free_list;
    free_list = block;
    
    // 尝试合并相邻的空闲块
    coalesce_blocks();
}

// calloc 实现
void *arm_calloc(size_t num, size_t size) {
    size_t total_size = num * size;
    void *ptr = arm_malloc(total_size);
    if (ptr != NULL) {
        // 将分配的内存清零
        memset(ptr, 0, total_size);
    }
    return ptr;
}

// realloc 实现
void *arm_realloc(void *ptr, size_t size) {
    if (ptr == NULL) {
        return arm_malloc(size);
    }
    
    if (size == 0) {
        arm_free(ptr);
        return NULL;
    }
    
    // 获取原有块的元数据
    block_meta *block = (block_meta *)((char *)ptr - META_SIZE);
    
    // 如果原有块足够大，直接返回
    if (block->size >= size) {
        return ptr;
    }
    
    // 分配新内存
    void *new_ptr = arm_malloc(size);
    if (new_ptr != NULL) {
        // 复制数据
        memcpy(new_ptr, ptr, block->size);
        // 释放旧内存
        arm_free(ptr);
    }
    
    return new_ptr;
}

// // 打印内存统计信息
// void arm_malloc_stats() {
//     size_t total_free = 0;
//     int free_blocks = 0;
//     block_meta *current = free_list;
    
//     while (current != NULL) {
//         if (current->free) {
//             total_free += current->size;
//             free_blocks++;
//         }
//         current = current->next;
//     }
    
//     // 打印统计信息
//     printf("Heap stats:\n");
//     printf("  Total heap size: %zu bytes\n", heap_size);
//     printf("  Free memory: %zu bytes in %d blocks\n", total_free, free_blocks);
//     printf("  Used memory: %zu bytes\n", heap_size - total_free);
// }