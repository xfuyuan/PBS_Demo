#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>

extern int serial_getc(void);
extern void serial_putc(char);
extern double my_pow(double, double);

// 辅助函数：跳过输入中的空白字符
static void skip_whitespace(const char **input)
{
    while (**input && isspace((unsigned char)**input)) {
        (*input)++;
    }
}

// 辅助函数：读取一个整数（支持不同进制）
static bool read_integer(const char **input, int base, long *value, int width)
{
    const char *start = *input;
    long result = 0;
    int sign = 1;
    int digits = 0;
    
    // 跳过前导空白
    skip_whitespace(input);
    
    // 检查符号
    if (**input == '-') {
        sign = -1;
        (*input)++;
    } else if (**input == '+') {
        (*input)++;
    }
    
    // 检查基数前缀
    if (base == 0) {
        if (**input == '0') {
            (*input)++;
            if (**input == 'x' || **input == 'X') {
                base = 16;
                (*input)++;
            } else {
                base = 8;
            }
        } else {
            base = 10;
        }
    }
    
    // 读取数字
    while (**input && (width < 0 || digits < width)) {
        int digit;
        
        if (isdigit((unsigned char)**input)) {
            digit = **input - '0';
        } else if (isxdigit((unsigned char)**input)) {
            digit = tolower((unsigned char)**input) - 'a' + 10;
        } else {
            break;
        }
        
        if (digit >= base) {
            break;
        }
        
        result = result * base + digit;
        (*input)++;
        digits++;
    }
    
    // 如果没有读取到任何数字，恢复输入指针
    if (digits == 0) {
        *input = start;
        return false;
    }
    
    *value = sign * result;
    return true;
}

// 辅助函数：读取一个浮点数
static bool read_float(const char **input, double *value, int width)
{
    const char *start = *input;
    double result = 0.0;
    int sign = 1;
    int digits = 0;
    bool has_digits = false;
    
    // 跳过前导空白
    skip_whitespace(input);
    
    // 检查符号
    if (**input == '-') {
        sign = -1;
        (*input)++;
    } else if (**input == '+') {
        (*input)++;
    }
    
    // 读取整数部分
    while (**input && isdigit((unsigned char)**input) && (width < 0 || digits < width)) {
        result = result * 10.0 + (**input - '0');
        (*input)++;
        digits++;
        has_digits = true;
    }
    
    // 读取小数部分
    if (**input == '.' && (width < 0 || digits < width)) {
        (*input)++;
        double fraction = 0.0;
        double divisor = 1.0;
        
        while (**input && isdigit((unsigned char)**input) && (width < 0 || digits < width)) {
            fraction = fraction * 10.0 + (**input - '0');
            divisor *= 10.0;
            (*input)++;
            digits++;
            has_digits = true;
        }
        
        result += fraction / divisor;
    }
    
    // 读取指数部分
    if ((**input == 'e' || **input == 'E') && (width < 0 || digits < width)) {
        (*input)++;
        int exp_sign = 1;
        int exponent = 0;
        
        if (**input == '-') {
            exp_sign = -1;
            (*input)++;
        } else if (**input == '+') {
            (*input)++;
        }
        
        while (**input && isdigit((unsigned char)**input) && (width < 0 || digits < width)) {
            exponent = exponent * 10 + (**input - '0');
            (*input)++;
            digits++;
        }
        
        result *= my_pow(10.0, exp_sign * exponent);
    }
    
    // 如果没有读取到任何数字，恢复输入指针
    if (!has_digits) {
        *input = start;
        return false;
    }
    
    *value = sign * result;
    return true;
}

// 辅助函数：读取一个字符串
static bool read_string(const char **input, char *str, int width, const char *delimiters)
{
    const char *start = *input;
    int count = 0;
    
    // 跳过前导空白
    skip_whitespace(input);
    
    // 读取字符直到遇到分隔符或达到宽度限制
    while (**input && !isspace((unsigned char)**input) && 
           (width < 0 || count < width)) {
        // 检查是否遇到分隔符
        if (delimiters) {
            const char *d = delimiters;
            while (*d) {
                if (**input == *d) {
                    goto done;
                }
                d++;
            }
        }
        
        *str++ = **input;
        (*input)++;
        count++;
    }
    
done:
    *str = '\0';
    
    // 如果没有读取到任何字符，恢复输入指针
    if (count == 0) {
        *input = start;
        return false;
    }
    
    return true;
}

// 辅助函数：读取一个字符
static bool read_char(const char **input, char *c, int width)
{
    const char *start = *input;
    int count = 0;
    
    // 读取指定数量的字符
    while (**input && (width < 0 || count < width)) {
        *c++ = **input;
        (*input)++;
        count++;
    }
    
    // 如果没有读取到任何字符，恢复输入指针
    if (count == 0) {
        *input = start;
        return false;
    }
    
    return true;
}

// 辅助函数：匹配一个字符集合
static bool read_scanset(const char **input, char *str, int width, const char *scanset, bool invert)
{
    const char *start = *input;
    int count = 0;
    
    // 跳过前导空白
    skip_whitespace(input);
    
    // 读取字符直到遇到不在集合中的字符或达到宽度限制
    while (**input && (width < 0 || count < width)) {
        bool in_set = false;
        const char *s = scanset;
        
        // 检查字符是否在集合中
        while (*s) {
            if (**input == *s) {
                in_set = true;
                break;
            }
            s++;
        }
        
        // 根据是否反转决定是否接受字符
        if ((in_set && !invert) || (!in_set && invert)) {
            *str++ = **input;
            (*input)++;
            count++;
        } else {
            break;
        }
    }
    
    *str = '\0';
    
    // 如果没有读取到任何字符，恢复输入指针
    if (count == 0) {
        *input = start;
        return false;
    }
    
    return true;
}

// 主函数：简化版 vsscanf 实现
int my_vsscanf(const char *input, const char *format, va_list args)
{
    int count = 0;
    const char *fmt = format;
    
    while (*fmt && *input) {
        if (*fmt != '%') {
            // 匹配普通字符
            if (*fmt == *input) {
                fmt++;
                input++;
            } else {
                // 不匹配，结束扫描
                break;
            }
            continue;
        }
        
        // 处理格式说明符
        fmt++; // 跳过 '%'
        
        // 解析赋值抑制符 '*'
        bool assign_suppressed = false;
        if (*fmt == '*') {
            assign_suppressed = true;
            fmt++;
        }
        
        // 解析宽度
        int width = -1;
        if (isdigit((unsigned char)*fmt)) {
            width = 0;
            while (isdigit((unsigned char)*fmt)) {
                width = width * 10 + (*fmt - '0');
                fmt++;
            }
        }
        
        // 解析长度修饰符（简化版，忽略）
        if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' || *fmt == 'j' || *fmt == 'z' || *fmt == 't') {
            fmt++;
            if ((*fmt == 'h' && fmt[-1] == 'h') || (*fmt == 'l' && fmt[-1] == 'l')) {
                fmt++;
            }
        }
        
        // 处理转换说明符
        switch (*fmt) {
            case 'd': {
                long value;
                if (read_integer(&input, 10, &value, width)) {
                    if (!assign_suppressed) {
                        int *ptr = va_arg(args, int*);
                        *ptr = (int)value;
                        count++;
                    }
                } else {
                    goto done;
                }
                fmt++;
                break;
            }
            
            case 'i': {
                long value;
                if (read_integer(&input, 0, &value, width)) {
                    if (!assign_suppressed) {
                        int *ptr = va_arg(args, int*);
                        *ptr = (int)value;
                        count++;
                    }
                } else {
                    goto done;
                }
                fmt++;
                break;
            }
            
            case 'o': {
                long value;
                if (read_integer(&input, 8, &value, width)) {
                    if (!assign_suppressed) {
                        unsigned int *ptr = va_arg(args, unsigned int*);
                        *ptr = (unsigned int)value;
                        count++;
                    }
                } else {
                    goto done;
                }
                fmt++;
                break;
            }
            
            case 'u': {
                long value;
                if (read_integer(&input, 10, &value, width)) {
                    if (!assign_suppressed) {
                        unsigned int *ptr = va_arg(args, unsigned int*);
                        *ptr = (unsigned int)value;
                        count++;
                    }
                } else {
                    goto done;
                }
                fmt++;
                break;
            }
            
            case 'x': case 'X': {
                long value;
                if (read_integer(&input, 16, &value, width)) {
                    if (!assign_suppressed) {
                        unsigned int *ptr = va_arg(args, unsigned int*);
                        *ptr = (unsigned int)value;
                        count++;
                    }
                } else {
                    goto done;
                }
                fmt++;
                break;
            }
            
            case 'f': case 'e': case 'E': case 'g': case 'G': {
                double value;
                if (read_float(&input, &value, width)) {
                    if (!assign_suppressed) {
                        float *ptr = va_arg(args, float*);
                        *ptr = (float)value;
                        count++;
                    }
                } else {
                    goto done;
                }
                fmt++;
                break;
            }
            
            case 's': {
                char *str = assign_suppressed ? NULL : va_arg(args, char*);
                if (read_string(&input, str, width, NULL)) {
                    if (!assign_suppressed) {
                        count++;
                    }
                } else {
                    goto done;
                }
                fmt++;
                break;
            }
            
            case 'c': {
                char *c = assign_suppressed ? NULL : va_arg(args, char*);
                if (read_char(&input, c, width > 0 ? width : 1)) {
                    if (!assign_suppressed) {
                        count++;
                    }
                } else {
                    goto done;
                }
                fmt++;
                break;
            }
            
            case '[': {
                fmt++; // 跳过 '['
                
                // 检查是否反转扫描集
                bool invert = false;
                if (*fmt == '^') {
                    invert = true;
                    fmt++;
                }
                
                // 收集扫描集字符
                char scanset[256] = {0};
                int i = 0;
                
                if (*fmt == ']') {
                    scanset[i++] = *fmt++;
                }
                
                while (*fmt && *fmt != ']' && i < 255) {
                    // 处理范围表示法，如 a-z
                    if (*fmt == '-' && i > 0 && fmt[1] && fmt[1] != ']') {
                        char start = scanset[i-1];
                        char end = fmt[1];
                        
                        if (start <= end) {
                            for (char ch = start + 1; ch <= end; ch++) {
                                if (i < 255) {
                                    scanset[i++] = ch;
                                }
                            }
                            fmt += 2;
                        } else {
                            scanset[i++] = *fmt++;
                        }
                    } else {
                        scanset[i++] = *fmt++;
                    }
                }
                
                if (*fmt == ']') {
                    fmt++;
                }
                
                char *str = assign_suppressed ? NULL : va_arg(args, char*);
                if (read_scanset(&input, str, width, scanset, invert)) {
                    if (!assign_suppressed) {
                        count++;
                    }
                } else {
                    goto done;
                }
                break;
            }
            
            case 'n': {
                if (!assign_suppressed) {
                    int *ptr = va_arg(args, int*);
                    *ptr = (int)(input - va_arg(args, const char*));
                }
                fmt++;
                break;
            }
            
            case '%': {
                if (*input == '%') {
                    input++;
                } else {
                    goto done;
                }
                fmt++;
                break;
            }
            
            default: {
                // 未知格式说明符，结束扫描
                goto done;
            }
        }
    }
    
done:
    return count;
}


// 从串口读取一行输入
static char *serial_gets(char *buf, int size)
{
    int count = 0;
    char c;
    
    if (size < 1) {
        return NULL;
    }
    
    while (count < size - 1) {
        c = serial_getc();
        
        // 处理退格键
        if (c == '\b' || c == 0x7F) { // 退格或DEL键
            if (count > 0) {
                count--;
                // 回显退格操作：退格-空格-退格
                serial_putc('\b');
                serial_putc(' ');
                serial_putc('\b');
            }
            continue;
        }
        
        // 回车键结束输入
        if (c == '\r') {
            serial_putc('\r');
            serial_putc('\n');
            break;
        }
        
        // 只接受可打印字符
        if (c >= ' ' && c <= '~') {
            buf[count++] = c;
            serial_putc(c); // 回显
        }
    }
    
    buf[count] = '\0';
    return buf;
}

// 实现 getchar 函数
int my_getchar(void)
{
    return (int)serial_getc();
}

// 实现 gets 函数（不推荐使用，但为了完整性）
char *my_gets(char *s)
{
    return serial_gets(s, 0x7FFFFFFF); // 使用一个很大的值，实际使用时应该避免
}


// 包装函数：sscanf
int my_sscanf(const char *input, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int result = my_vsscanf(input, format, args);
    va_end(args);
    return result;
}

// 包装函数：从串口读取并解析
int my_scanf(const char *format, ...)
{
    // 从串口读取一行输入
    char input_buffer[256];
    int index = 0;
    
    while (index < sizeof(input_buffer) - 1) {
        char c = serial_getc();
        
        // 处理退格键
        if (c == '\b' || c == 0x7F) {
            if (index > 0) {
                index--;
                // 回显退格操作
                serial_putc('\b');
                serial_putc(' ');
                serial_putc('\b');
            }
            continue;
        }
        
        // 回车键结束输入
        if (c == '\r' || c == '\n') {
            serial_putc('\r');
            serial_putc('\n');
            break;
        }
        
        // 只接受可打印字符
        if (c >= ' ' && c <= '~') {
            input_buffer[index++] = c;
            serial_putc(c); // 回显
        }
    }
    
    input_buffer[index] = '\0';
    
    // 解析输入
    va_list args;
    va_start(args, format);
    int result = my_vsscanf(input_buffer, format, args);
    va_end(args);
    
    return result;
}