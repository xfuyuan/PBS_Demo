# PBS项目主Makefile
PROJECT_NAME := PBS

# 重要目录定义
ROOT_DIR := .
SRC_DIR := $(ROOT_DIR)/SRC
DEV_DIR := $(SRC_DIR)/DEV
LIB_DIR := $(SRC_DIR)/LIB
INTEGRATION_DIR := $(ROOT_DIR)/Integration
COMMON_DIR := $(INTEGRATION_DIR)/Common
GENERATE_DIR := $(INTEGRATION_DIR)/Generate

# 包含通用编译规则
include $(COMMON_DIR)/Makefile.inc

# 默认目标
all: $(TARGET)

# 清理目标
clean:
	$(RM) -r $(GENERATE_DIR)
	@echo "清理完成!"

# 显示帮助信息
help:
	@echo "PBS项目构建系统"
	@echo "可用目标:"
	@echo "  make all     - 构建整个项目(默认)"
	@echo "  make clean   - 清理编译产物"
	@echo "  make help    - 显示此帮助信息"

.PHONY: all clean help