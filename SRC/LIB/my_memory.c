#include <stddef.h> // for size_t
#include <stdint.h>

void* my_memcpy_basic(void* dest, const void* src, size_t n) {
    // 将参数转换为字节指针
    char* d = (char*) dest;
    const char* s = (const char*) src;
    
    // 逐字节复制
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    
    return dest;
}

void* my_memcpy(void* dest, const void* src, size_t n) {
    // 将参数转换为字节指针
    char* d = (char*) dest;
    const char* s = (const char*) src;
    
    // 检查地址对齐情况
    size_t align = (sizeof(void*) - 1);
    size_t misalign_mask = (size_t)s | (size_t)d;
    
    // 如果源地址或目标地址没有对齐，或者复制的数据量很小，使用逐字节复制
    if ((misalign_mask & align) || n < sizeof(void*)) {
        for (size_t i = 0; i < n; i++) {
            d[i] = s[i];
        }
        return dest;
    }
    
    // 复制前面对齐部分（逐字节）
    size_t prefix = ((sizeof(void*) - ((size_t)d & align)) & align);
    for (size_t i = 0; i < prefix; i++) {
        d[i] = s[i];
    }
    
    // 使用字长复制主体部分
    size_t words = (n - prefix) / sizeof(void*);
    size_t* d_word = (size_t*)(d + prefix);
    const size_t* s_word = (const size_t*)(s + prefix);
    
    for (size_t i = 0; i < words; i++) {
        d_word[i] = s_word[i];
    }
    
    // 复制剩余尾部（逐字节）
    size_t suffix = (n - prefix) % sizeof(void*);
    size_t tail_start = prefix + words * sizeof(void*);
    for (size_t i = 0; i < suffix; i++) {
        d[tail_start + i] = s[tail_start + i];
    }
    
    return dest;
}

void* my_memcpy_fast(void* dest, const void* src, size_t n) {
    // 将参数转换为字节指针
    char* d = (char*) dest;
    const char* s = (const char*) src;
    
    // 检查重叠情况（memcpy不处理重叠，但这里作为安全措施）
    if (d > s && d < s + n) {
        // 如果目标地址在源地址范围内，从后向前复制
        for (size_t i = n; i > 0; i--) {
            d[i-1] = s[i-1];
        }
        return dest;
    }
    
    // 使用寄存器变量加速复制
    size_t i = 0;
    
    // 复制对齐部分
    while ((((size_t)(d + i) | (size_t)(s + i)) & (sizeof(size_t) - 1)) && i < n) {
        d[i] = s[i];
        i++;
    }
    
    // 使用字长复制
    size_t* d_word = (size_t*)(d + i);
    const size_t* s_word = (const size_t*)(s + i);
    size_t words = (n - i) / sizeof(size_t);
    
    for (size_t j = 0; j < words; j++) {
        d_word[j] = s_word[j];
    }
    
    // 复制剩余部分
    i += words * sizeof(size_t);
    for (; i < n; i++) {
        d[i] = s[i];
    }
    
    return dest;
}

#if defined(__aarch64__) && defined(__ARM_NEON)
#include <arm_neon.h>

void* my_memcpy_neon(void* dest, const void* src, size_t n) {
    if (n == 0) return dest;
    
    char* d = (char*)dest;
    const char* s = (const char*)src;
    
    // 处理前面对齐部分
    size_t align = (16 - ((uintptr_t)d % 16)) % 16;
    if (align > n) align = n;
    
    for (size_t i = 0; i < align; i++) {
        d[i] = s[i];
    }
    
    d += align;
    s += align;
    n -= align;
    
    // 使用 NEON 指令复制主体部分
    size_t chunks = n / 64;
    for (size_t i = 0; i < chunks; i++) {
        // 一次加载4个128位寄存器
        uint8x16x4_t data = vld4q_u8((const uint8_t*)s);
        // 一次存储4个128位寄存器
        vst4q_u8((uint8_t*)d, data);
        
        d += 64;
        s += 64;
    }
    
    // 处理剩余部分
    n %= 64;
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    
    return dest;
}
#endif

// 基本实现 - 逐字节填充
void* my_memset_basic(void* dest, int value, size_t count) {
    unsigned char* ptr = (unsigned char*)dest;
    unsigned char byte_value = (unsigned char)value;
    
    for (size_t i = 0; i < count; i++) {
        ptr[i] = byte_value;
    }
    
    return dest;
}

// 优化实现 - 使用字长填充和对齐处理
void* my_memset(void* dest, int value, size_t count) {
    if (count == 0) return dest;
    
    unsigned char* ptr = (unsigned char*)dest;
    unsigned char byte_value = (unsigned char)value;
    
    // 构建64位字（每个字节都是value）
    uint64_t word_value = 0;
    for (int i = 0; i < 8; i++) {
        word_value = (word_value << 8) | byte_value;
    }
    
    // 复制前面对齐部分（逐字节）
    size_t align = ((uintptr_t)ptr % sizeof(uint64_t));
    if (align > 0) {
        align = sizeof(uint64_t) - align;
        if (align > count) align = count;
        
        for (size_t i = 0; i < align; i++) {
            ptr[i] = byte_value;
        }
        
        ptr += align;
        count -= align;
    }
    
    // 使用64位字填充主体部分
    uint64_t* word_ptr = (uint64_t*)ptr;
    size_t words = count / sizeof(uint64_t);
    
    for (size_t i = 0; i < words; i++) {
        word_ptr[i] = word_value;
    }
    
    // 填充剩余尾部（逐字节）
    size_t tail = count % sizeof(uint64_t);
    size_t tail_start = words * sizeof(uint64_t);
    
    for (size_t i = 0; i < tail; i++) {
        ptr[tail_start + i] = byte_value;
    }
    
    return dest;
}

// 使用 NEON SIMD 指令的优化实现
#if defined(__aarch64__) && defined(__ARM_NEON)
#include <arm_neon.h>

void* my_memset_neon(void* dest, int value, size_t count) {
    if (count == 0) return dest;
    
    unsigned char* ptr = (unsigned char*)dest;
    unsigned char byte_value = (unsigned char)value;
    
    // 创建128位NEON寄存器，所有字节设置为value
    uint8x16_t vector = vdupq_n_u8(byte_value);
    
    // 处理前面对齐部分
    size_t align = (16 - ((uintptr_t)ptr % 16)) % 16;
    if (align > count) align = count;
    
    for (size_t i = 0; i < align; i++) {
        ptr[i] = byte_value;
    }
    
    ptr += align;
    count -= align;
    
    // 使用NEON指令进行向量化填充
    size_t vectors = count / 16;
    uint8x16_t* vector_ptr = (uint8x16_t*)ptr;
    
    for (size_t i = 0; i < vectors; i++) {
        vst1q_u8((uint8_t*)vector_ptr, vector);
        vector_ptr++;
    }
    
    // 处理剩余部分
    size_t remaining = count % 16;
    ptr += vectors * 16;
    
    for (size_t i = 0; i < remaining; i++) {
        ptr[i] = byte_value;
    }
    
    return dest;
}
#endif