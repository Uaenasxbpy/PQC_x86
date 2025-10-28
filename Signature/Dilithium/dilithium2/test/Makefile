# test/Makefile

BUILD_DIR := build
SRC := test_main.c cpucycles.c
OBJ := $(addprefix $(BUILD_DIR)/, $(SRC:.c=.o))
TARGET := $(BUILD_DIR)/test_main

CC ?= cc
CFLAGS += -I.. -I../.. -Wall -O2
LDFLAGS :=

# 依赖 Dilithium2 源码目录
DILITHIUM_SRC := ../sign.c ../packing.c ../polyvec.c ../poly.c ../ntt.c ../reduce.c ../rounding.c ../fips202.c ../symmetric-shake.c ../randombytes.c
DILITHIUM_OBJ := $(addprefix $(BUILD_DIR)/, $(notdir $(DILITHIUM_SRC:.c=.o)))

all: $(TARGET)

$(BUILD_DIR):
	@mkdir -p $@

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: ../%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: ../../%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ) $(DILITHIUM_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
