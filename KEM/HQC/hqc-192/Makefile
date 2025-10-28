ifeq ($(ROOT),)
ROOT:=.
endif

CC:=gcc

# 编译选项：启用RDPMC并包含必要的头文件路径
CFLAGS:=-O3 -std=c99 -funroll-all-loops -flto -pedantic -Wall -Wextra

# 核心代码路径（原src目录）
SRC_DIR:=$(ROOT)/src
# 测试代码路径（src/test目录）
TEST_DIR:=$(SRC_DIR)/test
# SHA3库路径
SHA3_DIR:=$(ROOT)/lib/fips202

# 输出目录
BUILD_DIR:=$(ROOT)/bin/build
BIN_DIR:=$(ROOT)/bin

# 头文件搜索路径：同时包含核心代码和测试代码目录
INCLUDES:=-I$(SRC_DIR) -I$(TEST_DIR) -I$(SHA3_DIR)

# 源文件定义
# 核心算法源文件
SHA3_SRC:=$(SHA3_DIR)/fips202.c
# 主程序源文件（原hqc-128）
MAIN_HQC_SRC:=$(SRC_DIR)/main_hqc.c
MAIN_KAT_SRC:=$(SRC_DIR)/main_kat.c
# 性能测试源文件（你的新程序）
MAIN_SPEED_SRC:=$(TEST_DIR)/test_speed.c
# CPU周期计数源文件
CPUCYCLES_SRC:=$(TEST_DIR)/cpucycles.c

# 目标文件定义（全部放在build目录下）
# 核心算法目标文件
HQC_OBJS:=$(addprefix $(BUILD_DIR)/, vector.o reed_muller.o reed_solomon.o fft.o gf.o gf2x.o code.o parsing.o hqc.o kem.o shake_ds.o shake_prng.o)
# SHA3库目标文件
SHA3_OBJ:=$(BUILD_DIR)/fips202.o
# 性能测试专用目标文件
CPUCYCLES_OBJ:=$(BUILD_DIR)/cpucycles.o

# 创建输出目录
$(BUILD_DIR) $(BIN_DIR):
	@echo "### 创建输出目录"
	mkdir -p $@

# 编译SHA3库
$(SHA3_OBJ): $(SHA3_SRC) | $(BUILD_DIR)
	@echo "### 编译SHA3库: $@"
	$(CC) $(CFLAGS) -c $< $(INCLUDES) -o $@

# 编译核心算法目标文件
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "### 编译核心模块: $@"
	$(CC) $(CFLAGS) -c $< $(INCLUDES) -o $@

# 编译CPU周期计数模块（关键：明确指定test目录下的源文件）
$(CPUCYCLES_OBJ): $(CPUCYCLES_SRC) $(TEST_DIR)/cpucycles.h | $(BUILD_DIR)
	@echo "### 编译性能测试模块: $@"
	$(CC) $(CFLAGS) -c $(CPUCYCLES_SRC) $(INCLUDES) -o $@

# hqc-192目标
hqc-192: $(HQC_OBJS) $(SHA3_OBJ) | $(BIN_DIR)
	@echo "### 链接hqc-192"
	$(CC) $(CFLAGS) $(MAIN_HQC_SRC) $^ $(INCLUDES) -o $(BIN_DIR)/$@

# hqc-192-kat目标
hqc-192-kat: $(HQC_OBJS) $(SHA3_OBJ) | $(BIN_DIR)
	@echo "### 链接hqc-192-kat"
	$(CC) $(CFLAGS) $(MAIN_KAT_SRC) $^ $(INCLUDES) -o $(BIN_DIR)/$@

# hqc-192-speed目标
hqc-192-speed: $(HQC_OBJS) $(SHA3_OBJ) $(CPUCYCLES_OBJ) | $(BIN_DIR)
	@echo "### 链接性能测试程序hqc-192-speed"
	$(CC) $(CFLAGS) $(MAIN_SPEED_SRC) $^ $(INCLUDES) -o $(BIN_DIR)/$@

# 清理目标
clean:
	@echo "### 清理编译产物"
	rm -rf $(BUILD_DIR) $(BIN_DIR)
	rm -f PQCkemKAT_* vgcore.*

# 声明伪目标，避免与文件重名
.PHONY: clean hqc-192 hqc-192-kat hqc-192-speed