// #include <sys/stat.h>
// #include <sys/types.h>
// #include <errno.h>
// #include <unistd.h>
// #include "utils.h"

// // 实现 _write 系统调用（用于输出）
// int _write(int file, char *ptr, int len)
// {
//     int i;
    
//     if (file == STDOUT_FILENO || file == STDERR_FILENO) {
//         for (i = 0; i < len; i++) {
//             if (ptr[i] == '\n') {
//                 serial_putc('\r');
//             }
//             serial_putc(ptr[i]);
//         }
//         return i;
//     }
    
//     errno = EIO;
//     return -1;
// }

// // 实现 _read 系统调用（用于输入）
// int _read(int file, char *ptr, int len)
// {
//     int i;
    
//     if (file == STDIN_FILENO) {
//         for (i = 0; i < len; i++) {
//             ptr[i] = serial_getc();
//             if (ptr[i] == '\r') {
//                 ptr[i] = '\n'; // 将回车转换为换行
//                 serial_putc('\r');
//                 serial_putc('\n');
//                 i++;
//                 break;
//             }
//             serial_putc(ptr[i]); // 回显
//         }
//         return i;
//     }
    
//     errno = EIO;
//     return -1;
// }

// // 实现其他必要的系统调用（简化版本）
// int _close(int file) { return -1; }
// int _fstat(int file, struct stat *st) { st->st_mode = S_IFCHR; return 0; }
// int _isatty(int file) { return 1; }
// int _lseek(int file, int ptr, int dir) { return 0; }
// int _open(const char *name, int flags, int mode) { return -1; }