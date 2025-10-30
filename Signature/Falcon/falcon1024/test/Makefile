# ========== Falcon 测速 Makefile（修复 clock_gettime 问题） ==========

CC = gcc
CFLAGS = -W -Wall -O2 -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE
LD = gcc
LDFLAGS =
LIBS = -lrt

OBJ = build/codec.o build/common.o build/fft.o build/fpr.o build/keygen.o \
      build/nist.o build/rng.o build/shake.o build/sign.o build/vrfy.o \
      build/katrng.o

build:
	-mkdir -p build

build/codec.o: ../codec.c ../api.h ../fpr.h ../inner.h | build
	$(CC) $(CFLAGS) -c -o $@ $<

build/common.o: ../common.c ../api.h ../fpr.h ../inner.h | build
	$(CC) $(CFLAGS) -c -o $@ $<

build/fft.o: ../fft.c ../api.h ../fpr.h ../inner.h | build
	$(CC) $(CFLAGS) -c -o $@ $<

build/fpr.o: ../fpr.c ../api.h ../fpr.h ../inner.h | build
	$(CC) $(CFLAGS) -c -o $@ $<

build/keygen.o: ../keygen.c ../api.h ../fpr.h ../inner.h | build
	$(CC) $(CFLAGS) -c -o $@ $<

build/nist.o: ../nist.c ../api.h ../fpr.h ../inner.h | build
	$(CC) $(CFLAGS) -c -o $@ $<

build/rng.o: ../rng.c ../api.h ../fpr.h ../inner.h | build
	$(CC) $(CFLAGS) -c -o $@ $<

build/shake.o: ../shake.c ../api.h ../fpr.h ../inner.h | build
	$(CC) $(CFLAGS) -c -o $@ $<

build/sign.o: ../sign.c ../api.h ../fpr.h ../inner.h | build
	$(CC) $(CFLAGS) -c -o $@ $<

build/vrfy.o: ../vrfy.c ../api.h ../fpr.h ../inner.h | build
	$(CC) $(CFLAGS) -c -o $@ $<

build/katrng.o: ../../KAT/generator/katrng.c ../../KAT/generator/katrng.h | build
	$(CC) $(CFLAGS) -c -o $@ $<

build/test_speed.o: test_speed.c ../api.h | build
	$(CC) $(CFLAGS) -c -o $@ $<

test_speed: build/test_speed.o $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	-rm -rf build test_speed
