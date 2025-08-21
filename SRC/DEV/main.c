#include "../LIB/utils.h"

extern void run_demo(void);
extern int my_printf(const char *format, ...);

int main() {
    my_printf("PBS项目演示程序启动\n");
    
    // 调用库函数
    print_welcome();
    
    // 调用驱动模块
    run_demo();
    
    my_printf("演示程序结束\n");
    return 0;
}