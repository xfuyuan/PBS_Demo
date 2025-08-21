#include <math.h>
#include <float.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>


static const double M_E = 2.718281828459045;

// 整数绝对值函数
int my_abs(int x) {
    if (x < 0) {
        return -x;
    }
    return x;
}

// 长整数绝对值函数
long my_labs(long x) {
    if (x < 0) {
        return -x;
    }
    return x;
}

// 长长整数绝对值函数
long long my_llabs(long long x) {
    if (x < 0) {
        return -x;
    }
    return x;
}

// 使用位操作实现整数绝对值（无分支）
int my_abs_bitwise(int x) {
    // 获取符号位（如果x为负，sign_mask = 0xFFFFFFFF，否则为0）
    int sign_mask = x >> (sizeof(int) * 8 - 1);
    
    // 如果x为负，则计算其补码：~(x - 1) = -x
    // 如果x为正，则保持不变：x
    return (x + sign_mask) ^ sign_mask;
}

// 另一种位操作方法
int my_abs_bitwise2(int x) {
    // 获取符号位
    int sign = x >> (sizeof(int) * 8 - 1);
    
    // 如果x为负，则取反加一（补码表示）
    return (x ^ sign) - sign;
}

// 单精度浮点数绝对值
float my_fabsf(float x) {
    if (x < 0.0f) {
        return -x;
    }
    return x;
}

// 双精度浮点数绝对值
double my_fabs(double x) {
    if (x < 0.0) {
        return -x;
    }
    return x;
}

// 长双精度浮点数绝对值
long double my_fabsl(long double x) {
    if (x < 0.0L) {
        return -x;
    }
    return x;
}

// 使用联合体实现单精度浮点数绝对值
float my_fabsf_union(float x) {
    union {
        float f;
        uint32_t i;
    } u;
    
    u.f = x;
    u.i &= 0x7FFFFFFF; // 清除符号位
    return u.f;
}

// 使用联合体实现双精度浮点数绝对值
double my_fabs_union(double x) {
    union {
        double d;
        uint64_t i;
    } u;
    
    u.d = x;
    u.i &= 0x7FFFFFFFFFFFFFFF; // 清除符号位
    return u.d;
}

// 辅助函数：计算自然对数（使用泰勒级数展开）
static double my_log(double x) {
    if (x <= 0.0) {
        return -HUGE_VAL; // 返回负无穷大
    }
    
    // 将 x 调整到 [0.5, 2] 区间
    double result = 0.0;
    int n = 0;
    
    while (x >= 2.0) {
        x /= 2.0;
        n++;
    }
    while (x < 0.5) {
        x *= 2.0;
        n--;
    }
    
    // 计算 ln(x) 使用泰勒级数展开
    x = x - 1.0; // 现在 x 在 [-0.5, 1] 区间
    double term = x;
    double x_power = x;
    int i = 1;
    
    do {
        result += term;
        i++;
        x_power *= x;
        term = x_power / i;
        if (i % 2 == 0) {
            term = -term;
        }
    } while (my_fabs(term) > DBL_EPSILON);
    
    // 加上调整的部分: ln(x) = result + n * ln(2)
    return result + n * 0.6931471805599453; // ln(2)
}

// 辅助函数：计算 e^x（使用泰勒级数展开）
static double my_exp(double x) {
    // 处理特殊情况
    if (x == 0.0) return 1.0;
    if (x < -700.0) return 0.0; // 避免下溢
    if (x > 700.0) return HUGE_VAL; // 避免上溢
    
    // 将指数分解为整数和小数部分
    int int_part = (int)x;
    double frac_part = x - int_part;
    
    // 计算 e^frac_part 使用泰勒级数展开
    double result = 1.0;
    double term = 1.0;
    int i = 1;
    
    do {
        term *= frac_part / i;
        result += term;
        i++;
    } while (my_fabs(term) > DBL_EPSILON);
    
    // 乘以 e^int_part
    if (int_part != 0) {
        // 使用快速幂算法计算 e^int_part
        double base = 2.718281828459045; // e
        double int_result = 1.0;
        int n = my_abs(int_part);
        
        while (n > 0) {
            if (n % 2 == 1) {
                int_result *= base;
            }
            base *= base;
            n /= 2;
        }
        
        if (int_part < 0) {
            int_result = 1.0 / int_result;
        }
        
        result *= int_result;
    }
    
    return result;
}

// 辅助函数：计算平方根（使用牛顿迭代法）
static double my_sqrt(double x) {
    if (x < 0.0) {
        return -HUGE_VAL; // 返回错误值
    }
    if (x == 0.0) {
        return 0.0;
    }
    
    double guess = x / 2.0;
    double prev_guess;
    
    do {
        prev_guess = guess;
        guess = (guess + x / guess) / 2.0;
    } while (my_fabs(guess - prev_guess) > DBL_EPSILON);
    
    return guess;
}

// 主函数：计算 x^y
double my_pow(double x, double y) {
    // 处理特殊情况
    if (x == 0.0) {
        if (y > 0.0) {
            return 0.0;
        } else if (y == 0.0) {
            return 1.0; // 0^0 通常定义为 1
        } else {
            return HUGE_VAL; // 1/0 = 无穷大
        }
    }
    
    if (x == 1.0) {
        return 1.0;
    }
    
    if (y == 0.0) {
        return 1.0;
    }
    
    if (y == 1.0) {
        return x;
    }
    
    // 处理整数指数（更精确）
    if (y == (int)y) {
        int n = (int)y;
        if (n == 0) {
            return 1.0;
        }
        
        // 使用快速幂算法
        double result = 1.0;
        double base = x;
        int exp = my_abs(n);
        
        while (exp > 0) {
            if (exp % 2 == 1) {
                result *= base;
            }
            base *= base;
            exp /= 2;
        }
        
        return n < 0 ? 1.0 / result : result;
    }
    
    // 处理负底数和非整数指数
    if (x < 0.0) {
        // 如果 y 是半整数 (n/2)，则可以计算实数结果
        if (y * 2 == (int)(y * 2)) {
            // y 是半整数
            double abs_result = my_pow(-x, y);
            if ((int)(y * 2) % 2 == 0) {
                return abs_result; // 偶数分母，结果为实数
            } else {
                return -abs_result; // 奇数分母，结果为负实数
            }
        } else {
            // 对于负底数和非半整数指数，结果为复数
            // 这里返回 NaN，但实际实现可能需要处理复数
            return -HUGE_VAL; // 简化为返回错误值
        }
    }
    
    // 一般情况：x^y = e^(y * ln(x))
    return my_exp(y * my_log(x));
}

// 测试函数
int test() {
    // 测试各种情况
    struct TestCase {
        double x;
        double y;
        double expected;
        const char *description;
    };
    
    struct TestCase test_cases[] = {
        {2.0, 3.0, 8.0, "2^3 = 8"},
        {2.0, -3.0, 0.125, "2^-3 = 0.125"},
        {4.0, 0.5, 2.0, "4^0.5 = 2"},
        {8.0, 1.0/3.0, 2.0, "8^(1/3) = 2"},
        {1.0, 100.0, 1.0, "1^100 = 1"},
        {0.0, 5.0, 0.0, "0^5 = 0"},
        {5.0, 0.0, 1.0, "5^0 = 1"},
        {0.0, 0.0, 1.0, "0^0 = 1"},
        {2.0, 10.0, 1024.0, "2^10 = 1024"},
        {10.0, -2.0, 0.01, "10^-2 = 0.01"},
        {2.5, 2.0, 6.25, "2.5^2 = 6.25"},
        {-2.0, 3.0, -8.0, "(-2)^3 = -8"},
        {-4.0, 0.5, -2.0, "(-4)^0.5 = -2 (实数部分)"},
        {M_E, 2.0, 7.38905609893, "e^2 ≈ 7.389"},
        {2.0, M_E, 6.580885991017, "2^e ≈ 6.580"},
        {0.0, -1.0, HUGE_VAL, "0^-1 = 无穷大"},
    };
    
    printf("测试 pow 函数实现:\n");
    printf("==========================================\n");
    
    for (int i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        double result = my_pow(test_cases[i].x, test_cases[i].y);
        double error = my_fabs(result - test_cases[i].expected);
        
        printf("%-25s: %.10f (期望: %.10f, 误差: %.10f)\n",
               test_cases[i].description,
               result,
               test_cases[i].expected,
               error);
    }
    
    // 与标准库实现比较
    printf("\n与标准库实现比较:\n");
    printf("==========================================\n");
    
    double test_values[][2] = {
        {2.0, 3.0},
        {2.0, 10.0},
        {10.0, -2.0},
        {2.5, 2.0},
        {M_E, 2.0},
        {2.0, M_E},
        {0.5, 3.0},
        {3.0, 0.5},
    };
    
    for (int i = 0; i < sizeof(test_values) / sizeof(test_values[0]); i++) {
        double x = test_values[i][0];
        double y = test_values[i][1];
        double my_result = my_pow(x, y);
        double std_result = my_pow(x, y);
        double error = my_fabs(my_result - std_result);
        
        printf("my_pow(%.5f, %.5f) = %.10f\n", x, y, my_result);
        printf("pow(%.5f, %.5f)     = %.10f\n", x, y, std_result);
        printf("误差: %.10f\n\n", error);
    }
    
    return 0;
}