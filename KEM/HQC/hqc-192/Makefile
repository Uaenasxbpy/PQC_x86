# ========== HQC 192 项目 Makefile (支持 Ubuntu 的 clock_gettime) ==========

ifeq ($(ROOT),)
ROOT:=.
endif

CC:=gcc

# 编译选项：启用 POSIX 功能、GNU 扩展和高优化
CFLAGS:=-O3 -std=c99 -funroll-all-loops -flto -pedantic -Wall -Wextra \
        -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE

# 链接库（clock_gettime 需要 librt）
LDLIBS:=-lrt

# 核心代码路径（原 src 目录）
SRC_DIR:=$(ROOT)/src
# 测试代码路径（src/test 目录）
TEST_DIR:=$(SRC_DIR)/test
# SHA3 库路径
SHA3_DIR:=$(ROOT)/lib/fips202

# 输出目录
BUILD_DIR:=$(ROOT)/bin/build
BIN_DIR:=$(ROOT)/bin

# 头文件搜索路径
INCLUDES:=-I$(SRC_DIR) -I$(TEST_DIR) -I$(SHA3_DIR)

# 源文件定义
# SHA3 库
SHA3_SRC:=$(SHA3_DIR)/fips202.c
# 主程序源文件
MAIN_HQC_SRC:=$(SRC_DIR)/main_hqc.c
MAIN_KAT_SRC:=$(SRC_DIR)/main_kat.c
# 性能测试源文件
MAIN_SPEED_SRC:=$(TEST_DIR)/test_speed.c
# CPU 周期计数模块
CPUCYCLES_SRC:=$(TEST_DIR)/cpucycles.c

# 目标文件定义（放入 build 目录）
HQC_OBJS:=$(addprefix $(BUILD_DIR)/, vector.o reed_muller.o reed_solomon.o fft.o gf.o gf2x.o code.o parsing.o hqc.o kem.o shake_ds.o shake_prng.o)
SHA3_OBJ:=$(BUILD_DIR)/fips202.o
CPUCYCLES_OBJ:=$(BUILD_DIR)/cpucycles.o

# ========== 构建规则 ==========

# 创建输出目录
$(BUILD_DIR) $(BIN_DIR):
	@echo "### 创建输出目录"
	mkdir -p $@

# 编译 SHA3 库
$(SHA3_OBJ): $(SHA3_SRC) | $(BUILD_DIR)
	@echo "### 编译 SHA3 库: $@"
	$(CC) $(CFLAGS) -c $< $(INCLUDES) -o $@

# 编译核心算法模块
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "### 编译核心模块: $@"
	$(CC) $(CFLAGS) -c $< $(INCLUDES) -o $@

# 编译 CPU 周期计数模块（关键部分）
$(CPUCYCLES_OBJ): $(CPUCYCLES_SRC) $(TEST_DIR)/cpucycles.h | $(BUILD_DIR)
	@echo "### 编译性能测试模块: $@"
	$(CC) $(CFLAGS) -c $(CPUCYCLES_SRC) $(INCLUDES) -o $@

# 编译 HQC 192 主程序
hqc-192: $(HQC_OBJS) $(SHA3_OBJ) | $(BIN_DIR)
	@echo "### 链接 hqc-192"
	$(CC) $(CFLAGS) $(MAIN_HQC_SRC) $^ $(INCLUDES) -o $(BIN_DIR)/$@ $(LDLIBS)

# 编译 HQC 192 KAT 测试程序
hqc-192-kat: $(HQC_OBJS) $(SHA3_OBJ) | $(BIN_DIR)
	@echo "### 链接 hqc-192-kat"
	$(CC) $(CFLAGS) $(MAIN_KAT_SRC) $^ $(INCLUDES) -o $(BIN_DIR)/$@ $(LDLIBS)

# 编译 HQC 192 性能测试程序
hqc-192-speed: $(HQC_OBJS) $(SHA3_OBJ) $(CPUCYCLES_OBJ) | $(BIN_DIR)
	@echo "### 链接性能测试程序 hqc-192-speed"
	$(CC) $(CFLAGS) $(MAIN_SPEED_SRC) $^ $(INCLUDES) -o $(BIN_DIR)/$@ $(LDLIBS)

# 清理目标
clean:
	@echo "### 清理编译产物"
	rm -rf $(BUILD_DIR) $(BIN_DIR)
	rm -f PQCkemKAT_* vgcore.*

# 声明伪目标
.PHONY: clean hqc-192 hqc-192-kat hqc-192-speed