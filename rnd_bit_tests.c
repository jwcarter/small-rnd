#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h> /* For timing loop */

#include "rnd.h"

#define NBITS 64
#define RR64(x,r) ((x)>>(r) | (x)<<(64-(r)))

/*
 * SD to p-value
 * Right Tail      Two tail
 * 3.090  1e-03    3.291  1e-03
 * 4.753  1e-06    4.892  1e-06
 * 5.998  1e-09    6.109  1e-09
 * 7.034  1e-12    7.131  1e-12
 * 7.941  1e-15    8.027  1e-15
 */

void print_bad(const char* const name, const char* const desc,
			   unsigned bit1, unsigned bit2, int dist, const char *pval_str)
{
	printf("  %s: %s %02d-%02d (%2d bits apart) p-value: %s\n",
				       name, desc, bit1, bit2, dist, pval_str);
}

int determine_bad(const char* const name, const char* const desc,
				  double r, unsigned bit1, unsigned bit2)
{
	int d = bit1 - bit2;
	int fail = 0;

	if (d < 0) d = -d;
	if (d*2 > NBITS) d -= NBITS;
	if (d < 0) d = -d;
	if (r < -7.941) {
		print_bad(name, desc, bit1, bit2, d, "< 1e-15");
		fail += 8;
	} else if (r < -7.034) {
		print_bad(name, desc, bit1, bit2, d, "< 1e-12");
		fail += 4;
	} else if (r < -5.998) {
		print_bad(name, desc, bit1, bit2, d, "< 1e-09");
		fail += 2;
	} else if (r < -4.753) {
		print_bad(name, desc, bit1, bit2, d, "< 1e-06");
		fail += 1;
	} else if (r > 7.941) {
		print_bad(name, desc, bit1, bit2, d, "> 1-1e-15");
		fail += 8;
	} else if (r > 7.034) {
		print_bad(name, desc, bit1, bit2, d, "> 1-1e-12");
		fail += 4;
	} else if (r > 5.998) {
		print_bad(name, desc, bit1, bit2, d, "> 1-1e-09");
		fail += 2;
	} else if (r > 4.753) {
		print_bad(name, desc, bit1, bit2, d, "> 1-1e-06");
		fail += 1;
	}

	return fail;
}

/*
 * Bit to bit correlation test
 * Takes a random number and counts how many times a bit in the random number
 * is the same value as another bit in the random number.
 * The null hypothesis is that the bits are independent.
 */
static inline void bit_to_bit_test_calc(uint64_t results[NBITS][NBITS], uint64_t x)
{
	unsigned i, j;
	uint64_t x0, x1, b;

	x0 = x;
	x1 = RR64(x,1);
	for (i=1; i<NBITS; i++) {
		b = (x0 ^ x1) ^ 0xFFFFFFFFFFFFFFFFULL;
		for (j=0; j<NBITS-i; j++) {
			results[j][j+i] += 0x1ULL & b;
			b = b >> 1;
		}
		x1 = RR64(x1,1);
	}
}

static void bit_to_bit_test_results(uint64_t results[NBITS][NBITS], uint64_t num_rnds)
{
	unsigned i,j;
	double e = num_rnds / 2.0;
	double sd = sqrt(num_rnds)/2.0;
	const char* const name = "Bit to Bit";
	const char* const desc = "Bit-Bit";
	int fail = 0;

	for (i=1; i<NBITS; i++) {
		for (j=0; j<NBITS-i; j++) {
			double r = ((double)results[j][j+i] - e)/sd;
			if (r < -4.753 || r > 4.753) {
				fail += determine_bad(name, desc, r, j, j+i);
			}
		}
	}
	if (fail >= 50) {
		printf("%s: Aborting test due to the number of failures\n", name);
		exit(-1);
	}
}

/* Bit-to-previous bit correlation test
 * Takes two random numbers and counts how many times a bit in the current
 * random number is the same value as a bit in the previous random number.
 * The null hypothesis is that the bits are independent.
 */
static inline void bit_to_prev_bit_test_calc(uint64_t results[NBITS][NBITS],
											 uint64_t x0, uint64_t x1)
{
	unsigned i, j, k;
	uint64_t b;
	for (i=0; i<NBITS; i++) {
		b = (x0 ^ x1) ^ 0xFFFFFFFFFFFFFFFFULL;
		for (j=0; j<NBITS; j++) {
			k = (i + j) % NBITS;
			results[j][k] += 0x1ULL & b;
			b = b >> 1;
		}
		x0 = RR64(x0,1);
	}
}

static void bit_to_prev_bit_test_results(uint64_t results[NBITS][NBITS],
										 uint64_t num_rnds)
{
	unsigned i,j;
	double e = num_rnds / 2.0;
	double sd = sqrt(num_rnds)/2.0;
	const char* const name = "Bit to Prev Bit";
	const char* const desc = "Bit-Prev";
	int fail = 0;

	for (i=0; i<NBITS; i++) {
		for (j=0; j<NBITS; j++) {
			double r = ((double)results[i][j] - e)/sd;
			if (r < -4.753 || r > 4.753) {
				fail += determine_bad(name, desc, r, i, j);
			}
		}
	}
	if (fail > 50) {
		printf("%s: Aborting test due to the number of failures\n", name);
		exit(-1);
	}
}


/*
 * MWC32 fails right away at 2^14 trials for high bits on bit-to-prev bit tests.
 * LCG64 starts getting warnings at 2^14 trials for low bits on the
 *   bit-to-prev bit tests. Fails at 2^17 trials.
 */

#define POW_START 14
#define POW_MAX 40
void bit_tests(rnd_t rnd)
{
	int64_t start = 0, stop = 0;
	unsigned pow = POW_START;
	unsigned pow_stop = POW_MAX;
	uint64_t start_rnds = 0;
	uint64_t num_rnds = 1ull << POW_START;
	uint64_t max_rnds = 1ull << POW_MAX;
	uint64_t i,j;
	uint64_t x0,x1;
	uint64_t prev_results[64][64];
	uint64_t cur_results[64][64];
	
	for (i=0; i<NBITS; i++) {
		for (j=0; j<NBITS; j++) {
			prev_results[i][j] = 0;
			cur_results[i][j] = 0;
		}
	}

	printf("============================================================\n");
	printf("Bit Tests\n");
	printf("  Start: %16llu (2^%u)\n",num_rnds, pow);
	printf("  End  : %16llu (2^%u)\n",max_rnds, pow_stop);
	printf("============================================================\n");
	start = clock();
	x1 = rnd_u64(rnd);
	printf("%25s    %s\n","Random Numbers Used", "Elapsed Time");
	while (num_rnds <= max_rnds) {
		stop = clock();
		printf("%16llu (2^%u)  %10.3f sec\n", num_rnds, pow,
		       ((double)stop-(double)start)/((double)CLOCKS_PER_SEC));
		for (i=start_rnds; i<num_rnds; i++) {
			x0 = x1;
			x1 = rnd_u64(rnd);
			bit_to_prev_bit_test_calc(prev_results, x0, x1);
			bit_to_bit_test_calc(cur_results, x0);
		}

		bit_to_prev_bit_test_results(prev_results, num_rnds);
		bit_to_bit_test_results(cur_results, num_rnds);

		start_rnds = num_rnds;
		pow++;
		num_rnds = 1ull << pow;
	}
	stop = clock();
	printf("%16llu Trials (2^%u)  %10.3f sec\n", num_rnds, pow,
	       ((double)stop-(double)start)/((double)CLOCKS_PER_SEC));
}

/* gcc -O1 -lm -o rnd_bit_tests rnd.c rnd_xx.c rnd_bit_tests.c */
int main (int argc, char *argv[])
{
	rnd_t rnd;
	unsigned long seed = 87654321UL;

	if (argc > 1) {
		seed += strtoul(argv[1], NULL, 10);
	}

	rnd = rnd_new();

	rnd_init(rnd, 87654321);

	bit_tests(rnd);

	rnd_free(rnd);

	return 0;
}
