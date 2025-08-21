# PBS项目通用编译规则

# 编译器设置
CC := gcc
CFLAGS := -Wall -Wextra -std=c99 -I$(LIB_DIR)
LDFLAGS := 

# 目标定义
TARGET := $(GENERATE_DIR)/bin/pbs_demo

# 源文件查找
DEV_SRCS := $(wildcard $(DEV_DIR)/*.c)
LIB_SRCS := $(wildcard $(LIB_DIR)/*.c)

# 对象文件生成路径
DEV_OBJS := $(patsubst $(DEV_DIR)/%.c, $(GENERATE_DIR)/obj/dev/%.o, $(DEV_SRCS))
LIB_OBJS := $(patsubst $(LIB_DIR)/%.c, $(GENERATE_DIR)/obj/lib/%.o, $(LIB_SRCS))
OBJS := $(DEV_OBJS) $(LIB_OBJS)

# 目录创建
$(shell mkdir -p $(GENERATE_DIR)/bin)
$(shell mkdir -p $(GENERATE_DIR)/obj/dev)
$(shell mkdir -p $(GENERATE_DIR)/obj/lib)

# 主构建规则
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "构建成功: $@"

# 编译DEV目录下的C文件
$(GENERATE_DIR)/obj/dev/%.o: $(DEV_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# 编译LIB目录下的C文件
$(GENERATE_DIR)/obj/lib/%.o: $(LIB_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# 头文件依赖处理
-include $(OBJS:.o=.d)

%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$