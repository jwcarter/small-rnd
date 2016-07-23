#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "rnd.h"

#define EXPECTED(x,p) ((double)(x)*(p))
#define SD(x,p) (sqrt((double)(x)*(p)*(1.0-(p))))

#define BITS_T 11 /* NUM_RND = 2^(BITS_T*2), etc */

void test_bit_cur_to_cur(rnd_t rnd, unsigned bits)
{
	uint64_t num_rnds = 0x1 << (BITS_T*2);
	double e = 0x1 << (BITS_T*2 - 1);
	double sd = 0x1 << (BITS_T - 1);

	unsigned i,j;
	uint64_t k;
	int d;
	unsigned bit[64];
	uint64_t res_count[64][64];
	int found = 0;

	printf("\n");
	printf("Testing relation between bits\n");
	printf("num_rnds=%ld  e=%.0f  sd=%.0f\n",num_rnds, e, sd);

	for (i=0; i<bits; i++) {
		for (j=0; j<bits; j++) {
			res_count[i][j] = 0;
		}
	}

	for (k=0; k<num_rnds; k++) {
		uint64_t x = rnd_u64(rnd);
		for (i=0; i<bits; i++) {
			bit[i] = 0x1 & x;
			x = x >> 1;
		}
		for (i=0; i<bits-1; i++) {
			for (j=i+1; j<bits; j++) {
				res_count[i][j] += bit[i] == bit[j];
			}
		}
	}

	for (i=0; i<bits-1; i++) {
		for (j=i+1; j<bits; j++) {
			double r = (double)(res_count[i][j] - e)/sd;
			if (r > 4.0 || r < -4.0) {
				if (!found) {
					printf(" Bit  Bit  Dist   SD\n");
					found = 1;
				}
				d = i-j;
				if (d < 0) d = -d;
				if (d*2 > bits) d -= bits;
				if (d < 0) d = -d;
				printf("%4d %4d  (%2d) %5.1f\n",i+1,j+1,d,r);
			}
		}
	}
	if (!found) {
		printf("All within 4 SD\n");
	}
}

/* gcc -O1 -o test_bit_cur -lm rnd_127.c rnd.c test_bit_cur.c */
int main (int argc, char **argv)
{
	rnd_t rnd;
	unsigned long seed = 2201;

	if (argc > 1) {
		seed += strtoul(argv[1],NULL,10);
	}

	rnd = rnd_new();
	rnd_init(rnd, seed);

	test_bit_cur_to_cur(rnd, 64);

	rnd_free(rnd);

	return 0;
}
