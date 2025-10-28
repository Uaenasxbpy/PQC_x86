CC = c99
CFLAGS = -W -Wall -O2
LD = c99
LDFLAGS =
LIBS =

OBJ = build/codec.o build/common.o build/fft.o build/fpr.o build/keygen.o build/nist.o build/rng.o build/shake.o build/sign.o build/vrfy.o build/katrng.o

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
Arm/Signature/Falcon/KAT/generator/katrng.c
Arm/Signature/Falcon/falcon512/test/Makefile

build/test_speed.o: test_speed.c ../api.h | build
	$(CC) $(CFLAGS) -c -o $@ $<

test_speed: build/test_speed.o $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	-rm -rf build test_speed