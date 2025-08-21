#include <stdio.h>
#include "../LIB/utils.h"

// 示例模块函数
void run_demo(void) {
    printf("驱动模块运行中...\n");
    printf("调用库函数: 5 + 3 = %d\n", add(5, 3));
}