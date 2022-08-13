/*
 * ISC License
 *
 * Copyright (c) 2020 James William Carter
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
#include <time.h> // For timing loop
#include "rnd.h"

void quicksort(uint64_t data[], uint64_t x, uint64_t l, uint64_t r)
{
	uint64_t i = l;
	uint64_t j = r;
	while (1) {
		while (data[i] < x && i < r) i++;
		while (data[j] > x && j > l) j--;
		if (i >= j) break;
		uint64_t t = data[i];
		data[i] = data[j];
		data[j] = t;
		i++;
		j--;
	}
	if (l < j) quicksort(data, data[(l+j)/2], l, j);
	if (j+1 < r) quicksort(data, data[(j+1+r)/2], j+1, r);
}

#define RL64(x,r) ((x)<<(r) | (x)>>(64-(r)))
#define HI(x,b) ((x) >> (64-(b)))

uint64_t get_number(rnd_t rnd, unsigned bits, unsigned t,
					uint64_t d, unsigned r)
{
	unsigned i;
	uint64_t y = rnd_u64(rnd);
	if (r > 0) {
		y = RL64(y, r);
	}
	if (bits < 64) {
		y = y >> (64-bits);
	}
	uint64_t x = y;
	for (i = 0; i < t-1; i++) {
		// bits < 64 if t > 1
		y = rnd_u64(rnd);
		if (r > 0) {
			y = RL64(y, r);
		}
		y = y >> (64-bits);
		x = d*x + y;
	}
	return x;
}

void print_bad(unsigned bit, unsigned bits, unsigned t, const char *pval_str)
{
	printf("\n");
	printf("Left-most bit: %u  Bits used: %u  t: %u  p-value: %s\n",
		   63-bit, bits, t, pval_str);
}


#define NUM 80000000ULL // 80,000,000
#define NUM_TESTS 3
#define NUM_BITS 64

// Lambda based on n = 80,000,000 and k = 2^64
//        n^3/(4*k) = 6938.89
// The p-values are for Lambda = 6938.89

void test_birthdayspacings(rnd_t rnd)
{
   uint32_t i, b, j;
   uint64_t *dates, *diffs;
   unsigned test_d_bits[NUM_TESTS] = {64, 32, 16};
   unsigned test_t[NUM_TESTS] = {1, 2, 4};

   dates = calloc(NUM+1, sizeof(uint64_t));
   diffs = calloc(NUM, sizeof(uint64_t));

   for (i=0; i<NUM_TESTS; i++ ) {
	   unsigned t = test_t[i];
	   unsigned bits = test_d_bits[i];
	   uint64_t d = (bits < 64) ? 0x1ULL << bits : 0;
	   printf("Bits: %u  Dimensions: %u\n", bits, t);
	   for (b=0; b<NUM_BITS; b++) {
		   fprintf(stderr,".");
		   for (j = 0; j < NUM; j++) {
			   dates[j] = get_number(rnd, bits, t, d, b);
		   }

		   quicksort(dates, dates[0], 0, NUM-1);

		   for (j=1; j<NUM; j++) {
			   diffs[j] = dates[j] - dates[j-1];
		   }

		   quicksort(diffs, diffs[1], 1, NUM-1);

		   uint64_t sum = 0;
		   for (j=2; j<NUM; j++) {
			   if (diffs[j] == diffs[j-1]) {
				   sum++;
			   }
		   }

		   if (sum < 6547 || sum > 7337) {
			   if (sum < 6288) {
				   print_bad(b, bits, t, "< 1e-15");
			   } else if (sum < 6361) {
				   print_bad(b, bits, t, "< 1e-12");
			   } else if (sum < 6445) {
				   print_bad(b, bits, t, "< 1e-09");
			   } else if (sum < 6547) {
				   print_bad(b, bits, t, "< 1e-06");
			   } else if (sum > 7610) {
				   print_bad(b, bits, t, "> 1-1e-15");
			   } else if (sum > 7532) {
				   print_bad(b, bits, t, "> 1-1e-12");
			   } else if (sum > 7443) {
				   print_bad(b, bits, t, "> 1-1e-09");
			   } else if (sum > 7337) {
				   print_bad(b, bits, t, "> 1-1e-06");
			   }
		   }
	   }
	   fprintf(stderr,"\n");
   }

   free(dates);
   free(diffs);
}

/* gcc -O1 -lm -I../ -o test_birthday bad_xorshift64.c test_birthday.c */
/* gcc -O1 -lm -I../ -o test_birthday ../rnd.c ../rnd_63.c test_birthday.c */
int main (int argc, char *argv[])
{
	rnd_t rnd = rnd_new();

	rnd_init(rnd, 87654321);

	test_birthdayspacings(rnd);

	rnd_free(rnd);

	return 0;
}
