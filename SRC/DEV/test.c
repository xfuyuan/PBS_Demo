#include <stdio.h>
#include "../LIB/utils.h"

extern int my_printf(const char *format, ...);
extern int my_scanf(const char *format, ...);
// 示例模块函数
void run_demo(void) {
    my_printf("my_printf test 驱动模块运行中...\n");
    my_printf("调用库函数: 5 + 3 = %d\n", add(5, 3));
    char str[256] = {0};
    my_scanf("%s", str);
    my_printf("intput string : %s\n", str);
}