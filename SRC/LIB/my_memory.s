// my_memcpy_asm.S
// ARMv8 汇编实现的 memcpy

.text
.align 2
.global my_memcpy_asm
.type my_memcpy_asm, %function

.global my_memset_asm
.type my_memset_asm, %function

my_memcpy_asm:
    // 参数:
    // x0 - 目标地址
    // x1 - 源地址
    // x2 - 复制长度
    
    // 如果长度为0，直接返回
    cbz x2, .Lexit
    
    // 保存寄存器
    stp x3, x4, [sp, -16]!
    stp x5, x6, [sp, -16]!
    stp x7, x8, [sp, -16]!
    stp x9, x10, [sp, -16]!
    
    mov x3, x0  // 保存目标地址
    
    // 检查重叠
    cmp x0, x1
    b.hi .Lcheck_overlap
    b .Lcopy_forward
    
.Lcheck_overlap:
    add x4, x1, x2
    cmp x0, x4
    b.lo .Lcopy_backward  // 如果目标在源范围内，向后复制
    
.Lcopy_forward:
    // 复制前面对齐部分
    tst x0, #7
    b.eq .Laligned_dest
    
.Lalign_dest:
    tst x0, #7
    b.eq .Laligned_dest
    ldrb w4, [x1], #1
    strb w4, [x0], #1
    subs x2, x2, #1
    b.eq .Lexit_restore
    b .Lalign_dest
    
.Laligned_dest:
    // 检查源地址是否对齐
    tst x1, #7
    b.eq .Laligned_both
    
    // 源未对齐，使用字节复制
    cmp x2, #64
    b.lo .Lbyte_copy
    
.Lunaligned_src:
    // 使用非对齐加载和对齐存储
    ldp x4, x5, [x1], #16
    ldp x6, x7, [x1], #16
    stp x4, x5, [x0], #16
    stp x6, x7, [x0], #16
    subs x2, x2, #32
    b.hi .Lunaligned_src
    b .Lbyte_copy
    
.Laligned_both:
    // 两者都已对齐，使用64位复制
    cmp x2, #64
    b.lo .Lword_copy
    
.Lvector_copy:
    // 使用多寄存器加载/存储
    ldp x4, x5, [x1], #16
    ldp x6, x7, [x1], #16
    ldp x8, x9, [x1], #16
    ldp x10, x11, [x1], #16
    stp x4, x5, [x0], #16
    stp x6, x7, [x0], #16
    stp x8, x9, [x0], #16
    stp x10, x11, [x0], #16
    subs x2, x2, #64
    b.hi .Lvector_copy
    
.Lword_copy:
    // 复制剩余的字
    cmp x2, #8
    b.lo .Lbyte_copy
    ldr x4, [x1], #8
    str x4, [x0], #8
    subs x2, x2, #8
    b.hi .Lword_copy
    
.Lbyte_copy:
    // 复制剩余的字节
    cbz x2, .Lexit_restore
    ldrb w4, [x1], #1
    strb w4, [x0], #1
    subs x2, x2, #1
    b.hi .Lbyte_copy
    b .Lexit_restore
    
.Lcopy_backward:
    // 从后向前复制
    add x0, x0, x2
    add x1, x1, x2
    
.Lbyte_copy_backward:
    ldrb w4, [x1, #-1]!
    strb w4, [x0, #-1]!
    subs x2, x2, #1
    b.hi .Lbyte_copy_backward
    
.Lexit_restore:
    // 恢复寄存器
    ldp x9, x10, [sp], 16
    ldp x7, x8, [sp], 16
    ldp x5, x6, [sp], 16
    ldp x3, x4, [sp], 16
    
.Lexit:
    ret

.size my_memcpy_asm, .-my_memcpy_asm

// my_memset_asm.S
// ARMv8 汇编实现的 memset

my_memset_asm:
    // 参数:
    // x0 - 目标地址
    // w1 - 填充值（低8位有效）
    // x2 - 填充长度
    
    // 如果长度为0，直接返回
    cbz x2, .Lexit
    
    // 保存寄存器
    stp x3, x4, [sp, -16]!
    stp x5, x6, [sp, -16]!
    stp x7, x8, [sp, -16]!
    
    // 将填充值扩展到64位（所有字节相同）
    and w1, w1, 0xFF
    mov x3, x1
    orr x3, x3, x3, lsl 8
    orr x3, x3, x3, lsl 16
    orr x3, x3, x3, lsl 32
    
    mov x4, x0  // 保存目标地址
    
    // 复制前面对齐部分
    tst x0, #7
    b.eq .Laligned_dest
    
.Lalign_dest:
    tst x0, #7
    b.eq .Laligned_dest
    strb w1, [x0], #1
    subs x2, x2, #1
    b.eq .Lexit_restore
    b .Lalign_dest
    
.Laligned_dest:
    // 使用64位字填充
    cmp x2, #64
    b.lo .Lword_copy
    
.Lvector_copy:
    // 使用多寄存器存储
    mov x5, x3
    mov x6, x3
    mov x7, x3
    mov x8, x3
    
    stp x5, x6, [x0], #16
    stp x7, x8, [x0], #16
    stp x5, x6, [x0], #16
    stp x7, x8, [x0], #16
    subs x2, x2, #64
    b.hi .Lvector_copy
    
.Lword_copy:
    // 复制剩余的字
    cmp x2, #8
    b.lo .Lbyte_copy
    str x3, [x0], #8
    subs x2, x2, #8
    b.hi .Lword_copy
    
.Lbyte_copy:
    // 复制剩余的字节
    cbz x2, .Lexit_restore
    strb w1, [x0], #1
    subs x2, x2, #1
    b.hi .Lbyte_copy
    
.Lexit_restore:
    // 恢复寄存器
    ldp x7, x8, [sp], 16
    ldp x5, x6, [sp], 16
    ldp x3, x4, [sp], 16
    
.Lexit:
    ret

.size my_memset_asm, .-my_memset_asm