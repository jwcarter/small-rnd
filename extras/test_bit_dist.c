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

#define BITS_T 7 /* NUM_RND = 2^(BITS_T*2), etc */

#define P1 0.682689492137086 /* expected fraction inside 1 SD */
#define P2 0.954499736103642 /* expected fraction inside 2 SD */
#define P3 0.997300203936740 /* expected fraction inside 3 SD */

void test_bit_distribution(rnd_t rnd, unsigned bits)
{
	uint64_t num_rnds = 0x1 << (BITS_T*2);
	uint64_t e = 0x1 << (BITS_T*2 - 1);
	uint64_t sd = 0x1 << (BITS_T - 1);

	uint64_t num_tests = 0x1 << (BITS_T*2 + 3);
	double e0 = EXPECTED(num_tests,0.5); /* Expected number of + SDs */
	double e1 = EXPECTED(num_tests,P1);  /* Expected <= 1 SD */
	double e2 = EXPECTED(num_tests,P2);  /* Expected <= 2 SD */
	double e3 = EXPECTED(num_tests,P3);  /* Expected <= 3 SD */
	double sd0 = SD(num_tests,0.5);
	double sd1 = SD(num_tests,P1);
	double sd2 = SD(num_tests,P2);
	double sd3 = SD(num_tests,P3);
	double e0t = EXPECTED(num_tests*bits,0.5); /* Expected number of + SDs */
	double e1t = EXPECTED(num_tests*bits,P1);  /* Expected <= 1 SD */
	double e2t = EXPECTED(num_tests*bits,P2);  /* Expected <= 2 SD */
	double e3t = EXPECTED(num_tests*bits,P3);  /* Expected <= 3 SD */
	double sd0t = SD(num_tests*bits,0.5);
	double sd1t = SD(num_tests*bits,P1);
	double sd2t = SD(num_tests*bits,P2);
	double sd3t = SD(num_tests*bits,P3);

	uint64_t i,j,k;
	uint64_t res_count[64];
	uint64_t res_sd[65][15];
	double sd_sd[65][15];
	int found = 0;

	printf("Testing distribution of bits\n");
	printf("num_tests=%ld num_rnds=%ld\n",num_tests, num_rnds);

	for (i=0; i<bits+1; i++) {
		for (j=0; j<15; j++) {
			res_sd[i][j] = 0;
			sd_sd[i][j] = 0.0;
		}
	}

	for (i=0; i<num_tests; i++) {
		for (j=0; j<bits; j++) {
			res_count[j] = 0;
		}

		for (k=0; k<num_rnds; k++) {
			uint64_t x = rnd_u64(rnd);
			for (j=0; j<bits; j++) {
				res_count[j] += 0x1 & x;
				x = x >> 1;
			}
		}
               
		for (j=0; j<bits; j++) {
			int c;
			int64_t d = res_count[j] - e;
			if (d == 0) {
				c = 7;
			} else {
				int s = 1;
				if (d < 0.0) {
					s = -1;
					d = -d;
				}
				if (d < sd)
					c = 7+s*1;
				else if (d == sd)
					c = 7+s*2;
				else if (d < 2*sd)
					c = 7+s*3;
				else if (d == 2*sd)
					c = 7+s*4;
				else if (d < 3*sd)
					c = 7+s*5;
				else if (d == 3*sd)
					c = 7+s*6;
				else
					c = 7+s*7;
                               
			}
			res_sd[j][c]++;
			res_sd[bits][c]++;
		}
	}

	for (i=0; i<bits; i++) {
		int m;
		double t = 0;
		double p = 0;

		p = res_sd[i][7]/2.0;
		for (m=8; m<15; m++)
			p += res_sd[i][m];
		sd_sd[i][0] = (p - e0)/sd0;

		t = res_sd[i][6] + res_sd[i][7] + res_sd[i][8];
		sd_sd[i][1] = (res_sd[i][5]/2.0 + t + res_sd[i][9]/2.0 - e1)/sd1;
		t += res_sd[i][4] + res_sd[i][5] + res_sd[i][9] + res_sd[i][10];
		sd_sd[i][2] = (res_sd[i][3]/2.0 + t + res_sd[i][11]/2.0 - e2)/sd2;
		t += res_sd[i][2] + res_sd[i][3] + res_sd[i][11] + res_sd[i][12];
		sd_sd[i][3] = (res_sd[i][1]/2.0 + t + res_sd[i][13]/2.0 - e3)/sd3;
	}
	{
		i = bits;
		int m;
		double t = 0;
		double p = 0;

		p = res_sd[i][7]/2.0;
		for (m=8; m<15; m++)
			p += res_sd[i][m];
		sd_sd[i][0] = (p - e0t)/sd0t;

		t = res_sd[i][6] + res_sd[i][7] + res_sd[i][8];
		sd_sd[i][1] = (res_sd[i][5]/2.0 + t + res_sd[i][9]/2.0 - e1t)/sd1t;
		t += res_sd[i][4] + res_sd[i][5] + res_sd[i][9] + res_sd[i][10];
		sd_sd[i][2] = (res_sd[i][3]/2.0 + t + res_sd[i][11]/2.0 - e2t)/sd2t;
		t += res_sd[i][2] + res_sd[i][3] + res_sd[i][11] + res_sd[i][12];
		sd_sd[i][3] = (res_sd[i][1]/2.0 + t + res_sd[i][13]/2.0 - e3t)/sd3t;
	}

	printf("%3s %5s %5s %5s %5s\n", "bit","+ ","<1 ","<2 ","<3");

	for (i=0; i<bits; i++) {
		printf("%2d: %5.1f %5.1f %5.1f %5.1f\n", i,
			   sd_sd[i][0], sd_sd[i][1], sd_sd[i][2], sd_sd[i][3]);
	}
	printf("%2s: %5.1f %5.1f %5.1f %5.1f\n", "T",
		   sd_sd[bits][0], sd_sd[bits][1], sd_sd[bits][2], sd_sd[bits][3]);

}

/* gcc -O1 -o test_bit_dist -lm bad_lcg64.c test_bit_dist.c */
int main (int argc, char **argv)
{
	rnd_t rnd;
	unsigned long seed = 2201;

	if (argc > 1) {
		seed += strtoul(argv[1],NULL,10);
	}

	rnd = rnd_new();
	rnd_init(rnd, seed);

	test_bit_distribution(rnd, 64);

	rnd_free(rnd);

	return 0;
}
