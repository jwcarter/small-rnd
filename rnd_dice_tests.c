#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h> // For timing loop

#include "rnd.h"

#define NBITS 64
#define RR64(x,r) ((x)>>(r) | (x)<<(64-(r)))

/*
 * Chi-Square [15 degrees of freedom] to p-value
 * 3.4830   1e-03    37.70  1-1e-03
 * 1.2160   1e-06    56.49  1-1e-06    
 * 0.4632   1e-09    73.63  1-1e-09    
 * 0.1814   1e-12    89.98  1-1e-12    
 *0.07175  1e-15   105.7   1-1e-15
 */

#define FAIR_SIDES 16

/*
 * Vertical Fair 16-sided die test
 * Takes four random numbers and creates a 4-bit number (0-15) for each bit
 * position to simulate a fair sixteen sided die. A count of how many times
 * each side is rolled is kept and a Chi-Square test is performed to determine
 * if the null hypothesis that all sides are equally probable is evaluated.
 */
static inline void vert_fair_d16_test_calc(uint64_t results[NBITS][FAIR_SIDES],
										   uint64_t x0, uint64_t x1, uint64_t x2,
										   uint64_t x3)
{
	uint64_t i,k;
	uint64_t bx0, bx1, bx2, bx3;
	for (i=0; i<NBITS; i++) {
		bx0 = 0x1 & x0;
		bx1 = 0x1 & x1;
		bx2 = 0x1 & x2;
		bx3 = 0x1 & x3;
		k = bx0 | (bx1 << 1) | (bx2 << 2) | (bx3 << 3);
		results[i][k]++;
		x0 = x0 >> 1;
		x1 = x1 >> 1;
		x2 = x2 >> 1;
		x3 = x3 >> 1;
	}
}

static void vert_fair_d16_test_results(uint64_t results[NBITS][FAIR_SIDES],
									   uint64_t num_rnds)
{
	unsigned i, j;
	double e = num_rnds/4.0/16.0;
	int fail = 0;
	const char* const name = "Vertical Fair die (16)";
	for (i=0; i<NBITS; i++) {
		double chi = 0.0;
		for (j=0; j<FAIR_SIDES; j++) {
			double d = (double)results[i][j] - e;
			chi += d*d/e;
		}
		if (chi >= 1.216 && chi <= 56.49) continue;

		if (chi < 0.07175) {
			printf("  %s: Bit %02d: > 1-1e-15\n", name, i);
			fail += 8;
		} else if (chi < 0.1814) {
			printf("  %s: Bit %02d: > 1-1e-12\n", name, i);
			fail += 4;
		} else if (chi < 0.4632) {
			printf("  %s: Bit %02d: > 1-1e-09\n", name, i);
			fail += 2;
		} else if (chi < 1.216) {
			printf("  %s: Bit %02d: > 1-1e-06\n", name, i);
			fail += 1;
		} else if (chi > 105.7) {
			printf("  %s: Bit %02d: < 1e-15\n", name, i);
			fail += 8;
		} else if (chi > 89.98) {
			printf("  %s: Bit %02d: < 1e-12\n", name, i);
			fail += 4;
		} else if (chi > 73.63) {
			printf("  %s: Bit %02d: < 1e-09\n", name, i);
			fail += 2;
		} else if (chi > 56.49) {
			printf("  %s: Bit %02d: < 1e-06\n", name, i);
			fail += 1;
		}
	}

	if (fail >= 50) {
		printf("%s: Aborting test due to the number of failures\n", name);
		exit(1);
	}
}

/*
 * Horizontal Fair 16-sided die test
 * Takes one random number and creates a 4-bit number (0-15) from every four
 * bits to simulate a fair sixteen sided die. A count of how many times each
 * side is rolled is kept and a Chi-Square test is performed to determine if
 * the null hypothesis that all sides are equally probable is evaluated.
 */
static inline void horz_fair_d16_test_calc(uint64_t results[NBITS][FAIR_SIDES],
										   uint64_t x)
{
	uint64_t i,k;
	for (i=0; i<NBITS; i+=4) {
		k = 0xf & x;
		results[i][k]++;
		x = x >> 4;
	}
}

static void horz_fair_d16_test_results(uint64_t results[NBITS][FAIR_SIDES],
									   uint64_t num_rnds)
{
	unsigned i, j;
	double e = num_rnds/16.0;
	int fail = 0;
	const char* const name = "Horizontal Fair die (16)";
	for (i=0; i<NBITS; i+=4) {
		double chi = 0.0;
		for (j=0; j<FAIR_SIDES; j++) {
			double d = (double)results[i][j] - e;
			chi += d*d/e;
		}
		if (chi >= 1.216 && chi <= 56.49) continue;

		if (chi < 0.07175) {
			printf("  %s: Bits %02d-%02d: > 1-1e-15\n", name, i, i+4);
			fail += 8;
		} else if (chi < 0.1814) {
			printf("  %s: Bits %02d-%02d: > 1-1e-12\n", name, i, i+4);
			fail += 4;
		} else if (chi < 0.4632) {
			printf("  %s: Bits %02d-%02d: > 1-1e-09\n", name, i, i+4);
			fail += 2;
		} else if (chi < 1.216) {
			printf("  %s: Bits %02d-%02d: > 1-1e-06\n", name, i, i+4);
			fail += 1;
		} else if (chi > 105.7) {
			printf("  %s: Bits %02d-%02d: < 1e-15\n", name, i, i+4);
			fail += 8;
		} else if (chi > 89.98) {
			printf("  %s: Bits %02d-%02d: < 1e-12\n", name, i, i+4);
			fail += 4;
		} else if (chi > 73.63) {
			printf("  %s: Bits %02d-%02d: < 1e-09\n", name, i, i+4);
			fail += 2;
		} else if (chi > 56.49) {
			printf("  %s: Bits %02d-%02d: < 1e-06\n", name, i, i+4);
			fail += 1;
		}
	}

	if (fail >= 50) {
		printf("%s: Aborting test due to the number of failures\n", name);
		exit(1);
	}
}

/*
 * MWC32 starts getting warnings at 2^27 trials for bits 52-64 on the
 *   the horizontal tests. Starts getting warnings at 2^30 trials for
 *   bits 54-63 on the vertical tests. Fails at 2^31 trials
 * LCG64 fails immediately at 2^14 trials for bits 0-9 and 13 on the
 *   vertical tests.
 */

#define POW_START 14
#define POW_MAX 40
void fair_dice_tests(rnd_t rnd)
{
	int64_t start = 0, stop = 0;
	unsigned pow = POW_START;
	unsigned pow_stop = POW_MAX;
	uint64_t start_rnds = 0;
	uint64_t num_rnds = 1ull << POW_START;
	uint64_t max_rnds = 1ull << POW_MAX;
	uint64_t i,j,k;
	uint64_t x0,x1,x2,x3;
	uint64_t vert_results[64][16];
	uint64_t horz_results[64][16];
	
	for (i=0; i<NBITS; i++) {
		for (j=0; j<FAIR_SIDES; j++) {
			vert_results[i][j] = 0;
			horz_results[i][j] = 0;
		}
	}

	printf("============================================================\n");
	printf("Fair Dice Tests\n");
	printf("  Start: %16llu (2^%u)\n",num_rnds, pow);
	printf("  End  : %16llu (2^%u)\n",max_rnds, pow_stop);
	printf("============================================================\n");
	start = clock();
	printf("%25s    %s\n","Random Numbers Used", "Elapsed Time");
	while (num_rnds <= max_rnds) {
		stop = clock();
		printf("%16llu (2^%u)  %10.3f sec\n", num_rnds, pow,
		       ((double)stop-(double)start)/((double)CLOCKS_PER_SEC));
		for (i=start_rnds; i<num_rnds; i=i+4) {
			x0 = rnd_u64(rnd);
			x1 = rnd_u64(rnd);
			x2 = rnd_u64(rnd);
			x3 = rnd_u64(rnd);
			vert_fair_d16_test_calc(vert_results, x0, x1, x2, x3);
			horz_fair_d16_test_calc(horz_results, x0);
			horz_fair_d16_test_calc(horz_results, x1);
			horz_fair_d16_test_calc(horz_results, x2);
			horz_fair_d16_test_calc(horz_results, x3);
		}

		vert_fair_d16_test_results(vert_results, num_rnds);
		horz_fair_d16_test_results(horz_results, num_rnds);

		start_rnds = num_rnds;
		pow++;
		num_rnds = 1ull << pow;
	}
	stop = clock();
	printf("%16llu Trials (2^%u)  %10.3f sec\n", num_rnds, pow,
	       ((double)stop-(double)start)/((double)CLOCKS_PER_SEC));
}

/* gcc -O1 -lm -o rnd_dice_test rnd.c rnd_xx.c rnd_dice_test.c */
int main (int argc, char *argv[])
{
	rnd_t rnd;
	unsigned long seed = 87654321UL;

	if (argc > 1) {
		seed += strtoul(argv[1], NULL, 10);
	}

	rnd = rnd_new();

	rnd_init(rnd, 87654321);

	fair_dice_tests(rnd);

	rnd_free(rnd);

	return 0;
}
