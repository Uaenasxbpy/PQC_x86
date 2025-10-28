CC=/usr/bin/gcc
CFLAGS += -O3 -march=native -fomit-frame-pointer
LDFLAGS=-lcrypto

SOURCES= cbd.c fips202.c indcpa.c kem.c ntt.c poly.c polyvec.c reduce.c rng.c verify.c symmetric-shake.c cpucycles.c
HEADERS= api.h cbd.h fips202.h indcpa.h ntt.h params.h poly.h polyvec.h reduce.h rng.h verify.h symmetric.h cpucycles.h

PQCgenKAT_kem: $(HEADERS) $(SOURCES) PQCgenKAT_kem.c
	$(CC) $(CFLAGS) -o $@ $(SOURCES) PQCgenKAT_kem.c $(LDFLAGS)

test_main: test_main.c $(HEADERS) $(SOURCES)
	$(CC) $(CFLAGS) -o $@ test_main.c $(SOURCES) $(LDFLAGS)

.PHONY: clean

clean:
	-rm -f PQCgenKAT_kem test_main