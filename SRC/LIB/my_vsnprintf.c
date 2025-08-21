// -------------------------------------------my_vsnprintf------------------------------
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

extern void serial_putc(char);

// 辅助函数：将数字转换为指定进制的字符串
static char *number_to_string(char *buf, unsigned long num, int base, bool is_signed, bool uppercase)
{
    char *ptr = buf;
    char *low = buf;
    const char *digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
    
    // 处理有符号数的负数情况
    if (is_signed && (long)num < 0) {
        *ptr++ = '-';
        low++;
        num = -(long)num;
    }
    
    // 生成数字字符串（反向）
    do {
        *ptr++ = digits[num % base];
        num /= base;
    } while (num > 0);
    
    *ptr = '\0';
    
    // 反转字符串
    char *high = ptr - 1;
    while (low < high) {
        char tmp = *low;
        *low++ = *high;
        *high-- = tmp;
    }
    
    return ptr;
}

// 辅助函数：计算字符串长度
static size_t string_length(const char *str)
{
    size_t len = 0;
    while (*str++) len++;
    return len;
}

// 辅助函数：复制字符串到缓冲区
static char *string_copy(char *dest, const char *src, size_t max_len)
{
    char *ptr = dest;
    while (*src && max_len-- > 1) {
        *ptr++ = *src++;
    }
    *ptr = '\0';
    return ptr;
}

// 主函数：简化版 vsnprintf 实现
int my_vsnprintf(char *buffer, size_t size, const char *format, va_list args)
{
    char *ptr = buffer;
    const char *fmt = format;
    char num_buf[32]; // 用于数字转换的临时缓冲区
    int written = 0;
    
    if (size == 0) {
        return 0;
    }
    
    // 确保缓冲区以空字符结尾
    buffer[0] = '\0';
    
    while (*fmt && written < (int)size - 1) {
        if (*fmt != '%') {
            // 普通字符，直接复制
            *ptr++ = *fmt++;
            written++;
            continue;
        }
        
        // 处理格式说明符
        fmt++; // 跳过 '%'
        
        // 解析标志字符（简化版，只支持部分）
        int width = 0;
        int precision = -1;
        bool left_align = false;
        bool zero_pad = false;
        bool always_sign = false;
        bool space_sign = false;
        bool alternate_form = false;
        
        // 解析标志
        while (true) {
            switch (*fmt) {
                case '-': left_align = true; fmt++; break;
                case '+': always_sign = true; fmt++; break;
                case ' ': space_sign = true; fmt++; break;
                case '#': alternate_form = true; fmt++; break;
                case '0': zero_pad = true; fmt++; break;
                default: goto parse_width;
            }
        }
        
    parse_width:
        // 解析宽度
        if (*fmt >= '0' && *fmt <= '9') {
            width = 0;
            while (*fmt >= '0' && *fmt <= '9') {
                width = width * 10 + (*fmt++ - '0');
            }
        } else if (*fmt == '*') {
            width = va_arg(args, int);
            fmt++;
        }
        
        // 解析精度
        if (*fmt == '.') {
            fmt++;
            precision = 0;
            if (*fmt >= '0' && *fmt <= '9') {
                precision = 0;
                while (*fmt >= '0' && *fmt <= '9') {
                    precision = precision * 10 + (*fmt++ - '0');
                }
            } else if (*fmt == '*') {
                precision = va_arg(args, int);
                fmt++;
            }
        }
        
        // 处理长度修饰符（简化版，忽略）
        switch (*fmt) {
            case 'h': case 'l': case 'L': case 'z': case 't': case 'j':
                fmt++;
                if ((*fmt == 'l' && fmt[-1] == 'l') || (*fmt == 'h' && fmt[-1] == 'h')) {
                    fmt++;
                }
                break;
        }
        
        // 处理转换说明符
        switch (*fmt) {
            case 'c': {
                // 字符
                char c = (char)va_arg(args, int);
                if (written < (int)size - 1) {
                    *ptr++ = c;
                    written++;
                }
                fmt++;
                break;
            }
            
            case 's': {
                // 字符串
                const char *s = va_arg(args, const char*);
                if (!s) s = "(null)";
                
                size_t len = string_length(s);
                if (precision >= 0 && (size_t)precision < len) {
                    len = precision;
                }
                
                // 处理宽度和对齐
                if (!left_align && width > (int)len) {
                    for (int i = 0; i < width - (int)len && written < (int)size - 1; i++) {
                        *ptr++ = ' ';
                        written++;
                    }
                }
                
                // 复制字符串
                for (size_t i = 0; i < len && written < (int)size - 1; i++) {
                    *ptr++ = *s++;
                    written++;
                }
                
                // 右对齐填充
                if (left_align && width > (int)len) {
                    for (int i = 0; i < width - (int)len && written < (int)size - 1; i++) {
                        *ptr++ = ' ';
                        written++;
                    }
                }
                fmt++;
                break;
            }
            
            case 'd': case 'i': {
                // 有符号十进制整数
                int num = va_arg(args, int);
                char *num_str = number_to_string(num_buf, (unsigned long)(num < 0 ? -num : num), 10, true, false);
                
                // 处理符号
                bool negative = num < 0;
                bool has_sign = negative || always_sign || space_sign;
                char sign_char = negative ? '-' : (always_sign ? '+' : (space_sign ? ' ' : '\0'));
                
                size_t len = string_length(num_buf);
                
                // 处理精度
                if (precision > (int)len) {
                    len = precision;
                }
                if (has_sign) {
                    len++; // 符号占一个字符
                }
                
                // 处理宽度和对齐
                if (!left_align && width > (int)len) {
                    char pad_char = zero_pad && precision < 0 ? '0' : ' ';
                    for (int i = 0; i < width - (int)len && written < (int)size - 1; i++) {
                        *ptr++ = pad_char;
                        written++;
                    }
                }
                
                // 输出符号
                if (has_sign && written < (int)size - 1) {
                    *ptr++ = sign_char;
                    written++;
                }
                
                // 输出前导零（精度指定的）
                if (precision > (int)string_length(num_buf)) {
                    for (int i = 0; i < precision - (int)string_length(num_buf) && written < (int)size - 1; i++) {
                        *ptr++ = '0';
                        written++;
                    }
                }
                
                // 输出数字
                for (const char *p = num_buf; *p && written < (int)size - 1; p++) {
                    *ptr++ = *p;
                    written++;
                }
                
                // 右对齐填充
                if (left_align && width > (int)len) {
                    for (int i = 0; i < width - (int)len && written < (int)size - 1; i++) {
                        *ptr++ = ' ';
                        written++;
                    }
                }
                fmt++;
                break;
            }
            
            case 'u': {
                // 无符号十进制整数
                unsigned int num = va_arg(args, unsigned int);
                char *num_str = number_to_string(num_buf, num, 10, false, false);
                size_t len = string_length(num_buf);
                
                // 处理精度
                if (precision > (int)len) {
                    len = precision;
                }
                
                // 处理宽度和对齐
                if (!left_align && width > (int)len) {
                    char pad_char = zero_pad && precision < 0 ? '0' : ' ';
                    for (int i = 0; i < width - (int)len && written < (int)size - 1; i++) {
                        *ptr++ = pad_char;
                        written++;
                    }
                }
                
                // 输出前导零（精度指定的）
                if (precision > (int)string_length(num_buf)) {
                    for (int i = 0; i < precision - (int)string_length(num_buf) && written < (int)size - 1; i++) {
                        *ptr++ = '0';
                        written++;
                    }
                }
                
                // 输出数字
                for (const char *p = num_buf; *p && written < (int)size - 1; p++) {
                    *ptr++ = *p;
                    written++;
                }
                
                // 右对齐填充
                if (left_align && width > (int)len) {
                    for (int i = 0; i < width - (int)len && written < (int)size - 1; i++) {
                        *ptr++ = ' ';
                        written++;
                    }
                }
                fmt++;
                break;
            }
            
            case 'x': case 'X': {
                // 十六进制整数
                unsigned int num = va_arg(args, unsigned int);
                bool upper = (*fmt == 'X');
                char *num_str = number_to_string(num_buf, num, 16, false, upper);
                size_t len = string_length(num_buf);
                
                // 处理替代形式
                if (alternate_form && num != 0) {
                    len += 2; // "0x" 或 "0X" 前缀
                }
                
                // 处理精度
                if (precision > (int)string_length(num_buf)) {
                    len += precision - string_length(num_buf);
                }
                
                // 处理宽度和对齐
                if (!left_align && width > (int)len) {
                    char pad_char = zero_pad && precision < 0 ? '0' : ' ';
                    for (int i = 0; i < width - (int)len && written < (int)size - 1; i++) {
                        *ptr++ = pad_char;
                        written++;
                    }
                }
                
                // 输出前缀
                if (alternate_form && num != 0 && written < (int)size - 1) {
                    *ptr++ = '0';
                    written++;
                    if (written < (int)size - 1) {
                        *ptr++ = upper ? 'X' : 'x';
                        written++;
                    }
                }
                
                // 输出前导零（精度指定的）
                if (precision > (int)string_length(num_buf)) {
                    for (int i = 0; i < precision - (int)string_length(num_buf) && written < (int)size - 1; i++) {
                        *ptr++ = '0';
                        written++;
                    }
                }
                
                // 输出数字
                for (const char *p = num_buf; *p && written < (int)size - 1; p++) {
                    *ptr++ = *p;
                    written++;
                }
                
                // 右对齐填充
                if (left_align && width > (int)len) {
                    for (int i = 0; i < width - (int)len && written < (int)size - 1; i++) {
                        *ptr++ = ' ';
                        written++;
                    }
                }
                fmt++;
                break;
            }
            
            case 'p': {
                // 指针
                void *p = va_arg(args, void*);
                char *num_str = number_to_string(num_buf, (unsigned long)p, 16, false, false);
                size_t len = string_length(num_buf) + 2; // "0x" 前缀
                
                // 处理宽度和对齐
                if (!left_align && width > (int)len) {
                    for (int i = 0; i < width - (int)len && written < (int)size - 1; i++) {
                        *ptr++ = ' ';
                        written++;
                    }
                }
                
                // 输出前缀
                if (written < (int)size - 1) {
                    *ptr++ = '0';
                    written++;
                }
                if (written < (int)size - 1) {
                    *ptr++ = 'x';
                    written++;
                }
                
                // 输出前导零
                if (string_length(num_buf) < sizeof(void*) * 2) {
                    for (size_t i = 0; i < sizeof(void*) * 2 - string_length(num_buf) && written < (int)size - 1; i++) {
                        *ptr++ = '0';
                        written++;
                    }
                }
                
                // 输出数字
                for (const char *p = num_buf; *p && written < (int)size - 1; p++) {
                    *ptr++ = *p;
                    written++;
                }
                
                // 右对齐填充
                if (left_align && width > (int)len) {
                    for (int i = 0; i < width - (int)len && written < (int)size - 1; i++) {
                        *ptr++ = ' ';
                        written++;
                    }
                }
                fmt++;
                break;
            }
            
            case '%': {
                // 百分号
                if (written < (int)size - 1) {
                    *ptr++ = '%';
                    written++;
                }
                fmt++;
                break;
            }
            
            default: {
                // 未知格式说明符，直接输出
                if (written < (int)size - 1) {
                    *ptr++ = '%';
                    written++;
                }
                if (written < (int)size - 1) {
                    *ptr++ = *fmt;
                    written++;
                }
                fmt++;
                break;
            }
        }
    }
    
    // 确保字符串以空字符结尾
    if (written < (int)size) {
        *ptr = '\0';
    } else if (size > 0) {
        buffer[size - 1] = '\0';
    }
    
    return written;
}

// 包装函数：snprintf
int my_snprintf(char *buffer, size_t size, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int result = my_vsnprintf(buffer, size, format, args);
    va_end(args);
    return result;
}

int my_printf(const char *format, ...)
{
    va_list args;
    int length;
    char buffer[256]; // 可根据需要调整大小
    
    va_start(args, format);
    // 使用 vsnprintf 避免缓冲区溢出
    length = my_vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // 如果格式化后的字符串超过缓冲区大小，则截断
    if (length >= (int)sizeof(buffer)) {
        length = sizeof(buffer) - 1;
        buffer[length] = '\0';
    }
    
    // 输出字符串，处理换行符转换
    for (int i = 0; i < length; i++) {
        if (buffer[i] == '\n') {
            serial_putc('\r');
        }
        serial_putc(buffer[i]);
    }
    
    return length;
}

// 实现 putchar 以支持更广泛的标准库函数
int my_putchar(int c)
{
    if (c == '\n') {
        serial_putc('\r');
    }
    serial_putc((char)c);
    return c;
}

// 实现 puts 函数
int my_puts(const char *str)
{
    int count = 0;
    while (*str) {
        my_putchar(*str++);
        count++;
    }
    my_putchar('\n'); // 自动添加换行符
    return count + 1; // 包括换行符
}
