/*
 * ISC License
 *
 * Copyright (c) 2016 James William Carter
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "rnd.h"

#define EXPECTED(x,p) ((double)(x)*(p))
#define SD(x,p) (sqrt((double)(x)*(p)*(1.0-(p))))

#define BITS_T 10 /* NUM_RND = 2^(BITS_T*2), etc */

void test_bit_cur_to_prev(rnd_t rnd, unsigned bits)
{
	uint64_t num_rnds = 0x1 << (BITS_T*2);
	double e = 0x1 << (BITS_T*2 - 1);
	double sd = 0x1 << (BITS_T - 1);

	unsigned i,j;
	uint64_t k;
	uint64_t x0,x,y0,y;
	unsigned bitx, bity;
	int d;
	uint64_t res_count[64][64];
	int found = 0;

	printf("\n");
	printf("Testing relation between current and previous bits\n");
	printf("num_rnds=%ld  e=%.0f  sd=%.0f\n",num_rnds, e, sd);

	for (i=0; i<bits; i++) {
		for (j=0; j<bits; j++) {
			res_count[i][j] = 0;
		}
	}

	x0 = rnd_u64(rnd);
	for (k=0; k<num_rnds; k++) {
		y0 = x0;
		x0 = rnd_u64(rnd);
		x = x0;
		for (i=0; i<bits; i++) {
			bitx = 0x1 & x;
			x = x >> 1;
			y = y0;
			for (j=0; j<bits; j++) {
				bity = 0x1 & y;
				y = y >> 1;
				res_count[i][j] += bitx == bity;
			}
		}
	}

	for (i=0; i<bits; i++) {
		for (j=0; j<bits; j++) {
			double sn = 2.0*((double)res_count[i][j] - e);
			if (sn < 0) sn = -sn;
			double pv = erfc(sn/sqrt((double)(2*num_rnds)));
			if (pv < 1.0e-99) pv = 0.0;
			double r = (double)(res_count[i][j] - e)/sd;
			if (r > 4.0 || r < -4.0) {
				if (!found) {
					printf(" Cur Prev  Dist    SD      p-val\n");
					found = 1;
				}
				d = i-j;
				if (d < 0) d = -d;
				if (d*2 > bits) d -= bits;
				if (d < 0) d = -d;
				printf("%4d %4d  (%2d) %6.1f %11.3e\n",i,j,d,r,pv);
			}
		}
	}
	if (!found) {
		printf("All within 4 SD\n");
	}
}

/* gcc -O1 -o test_bit_prev -lm bad_lcg64.c test_bit_prev.c */
int main (int argc, char **argv)
{
	rnd_t rnd;
	unsigned long seed = 2201;

	if (argc > 1) {
		seed += strtoul(argv[1],NULL,10);
	}

	rnd = rnd_new();
	rnd_init(rnd, seed);

	test_bit_cur_to_prev(rnd, 64);

	rnd_free(rnd);

	return 0;
}
