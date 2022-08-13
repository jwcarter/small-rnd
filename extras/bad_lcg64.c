/*
 * ISC License
 *
 * Copyright (c) 2022 James William Carter
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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

/* bad_lcg64 - LCG64 generator with period 2^64
 *
 * Author: James Carter
 *
 * Full 64-bit output from a 64-bit Linear Congruential Generator
 *
 * Criteria for LCGs:
 *  2^64 and C10 are relatively prime
 *  A10 - 1 is divisible by 2 (the prime factor of 2^64)
 *  A10 - 1 is divisible by 4 (since 2^64 is divisible by 4)
 *  A10 % 8 = 5 (A10 not divisible by 8)
 *
 * test_birthday results:
 * Bits used: 64:  t: 1
 *   Left 26, 17-9, 7-5  : p-value > 1-1e-15
 *   Left 4              : p-value > 1-1e-12
 *   Left 8              : p-value > 1-1e-09
 *   Left 24-18          : p-value < 1e-15
 * Bits used: 32  t: 2
 *   Left 63-0           : p-value > 1-1e-15
 * Bits used: 16  t: 4
 *   Left 63-0           : p-value > 1-1e-15
 *
 * rnd_bit_tests results:
 * 2^14 trials
 *   Bit-to-Prev-Bit 03-03                       : p-value > 1-1e-15
 *   Bit-to-Prev-Bit 00-00, 02-02, 04-04, 05-05  : p-value < 1e-15
 * 2^15 trials
 *   Bit-to-Prev-Bit 06-06                       : p-value < 1e-06
 * 2^16 trials
 *   Bit-to-Prev-Bit 06-06                       : p-value < 1e-15
 *   Bit-to-Prev-Bit 08-08                       : p-value < 1e-09
 * 2^17 trials
 *   Bit-to-Prev-Bit 08-08                       : p-value < 1e-15
 *   Bit-to-Prev-Bit 07-07                       : p-value < 1e-06
 *   Test aborts due to the number of failures
 *
 * rnd_dice_tests results:
 * 2^14 trials
 *   Vertical Bits 00-09  : p-value < 1e-15
 *   Vertical Bits 13     : p-value > 1-1e-09
 *   Test aborts due to the number of failures
 *
 * test_bit_prev results:
 *   0-0, 2-2, 3-3, 4-4, 5-5, 6-6, 7-7, 8-8, 9-9  : p-value < 1e-15
 *   10-10                                        : p-value < 1e-09
 *
 * test_bit_dist results:
 *   Bits 00-27 have bad distributions
 */

struct rnd {
	uint64_t s1;
};

#define H32(x) ((x)>>32)
#define L32(x) ((x)&0xFFFFFFFFUL)
#define FLIP32(x) ((x)>>32 | (x)<<32)
#define A10 13624260627007768477ULL /* LCG64: 2621890429 * 5196350113 */
#define C10  8407169572674124741ULL /* LCG64: 13913 * 68683 * 77773 * 113123 */
#define Z1 4078645709ULL /* LCG: 77999 * 52291 */
#define Z2 3580663381ULL /* LCG: 68111 * 52571 */
							   
static inline uint64_t next_rnd(struct rnd *rnd)
{
	uint64_t x1 = rnd->s1*A10+C10;
	rnd->s1 = x1;
	return x1;
}

#define K1 4110542893ULL /* 59443 * 69151 = 0xF501E82D */
#define K2 3667735229ULL /* 51407 * 71347 = 0xDA9D32BD */
#define K3 4068774709ULL /* 46399 * 87691 = 0xF2849335 */
#define K4 3322924477ULL /* 44939 * 73943 = 0xC60FCDBD */

#define Z3 3571494541ULL /* LCG: 91019 * 39239 */
#define Z4 3753453877ULL /* LCG: 13999 * 268123 */
#define Z5 18277323205306182053ULL /* LCG: 26777 * 65777 * 78787 * 131711 */
#define L31(x) ((x)&0x7FFFFFFFUL)
#define L30(x) ((x)&0x3FFFFFFFUL)

void rnd_init(struct rnd *rnd, unsigned long seed)
{
	int i = 0;
	uint32_t z[10];
	uint64_t x1 = ((seed + K1) * K2) + K3;
	while (i < 10) {
		x1 = ((seed + x1) * K3) + K4;
		if (L30(x1) != 0) {
			z[i] = L32(x1);
			i++;
		}
	}
	rnd->s1 = (L31(z[9]) << 32) | L30(z[7]);

	for (i=0; i<13; i++)
		next_rnd(rnd);
}

struct rnd *rnd_new()
{
	struct rnd *rnd = malloc(sizeof (struct rnd));
	if (!rnd) {
		fprintf(stderr,"rnd: Malloc failed!");
		exit(-1);
	}
	return rnd;
}

void rnd_free(struct rnd *rnd)
{
	free(rnd);
}

void rnd_get_state(struct rnd *rnd, uint32_t state[])
{
	state[0] = H32(rnd->s1);
	state[1] = L32(rnd->s1);
}

#define SL32(x) ((uint64_t)(x)<<32)

void rnd_set_state(struct rnd *rnd, uint32_t state[])
{
	rnd->s1 = SL32(state[0]) | state[1];
}

#define INV52M1 2.2204460492503136e-16 /* 1/(2^52-1) */
#define INV52P1 2.2204460492503126e-16 /* 1/(2^52+1) */
#define H52(x) ((x)>>12)
#define CLOSED(x) ((double)H52(x)*INV52M1)
#define OPEN(x) ((double)(H52(x)+1)*INV52P1)

uint32_t rnd_u32(struct rnd *rnd)
{
	return H32(next_rnd(rnd));
}

uint64_t rnd_u64(struct rnd *rnd)
{
	return next_rnd(rnd);
}

double rnd_closed(struct rnd *rnd)
{
	/* Return double [0,1] in continuous uniform distribution */
	return CLOSED(next_rnd(rnd));
}

double rnd_open(struct rnd *rnd)
{
	/* Return double (0,1) in continuous uniform distribution */
	return OPEN(next_rnd(rnd));
}

/*
gcc -O1 -lm -I../ -o test_birthday bad_lcg64.c test_birthday.c
gcc -O1 -lm -I../ -o rnd_dice_test bad_lcg64.c ../rnd_dice_test.c
gcc -O1 -lm -I../ -o rnd_bit_tests bad_lcg64.c ../rnd_bit_tests.c
gcc -O1 -lm -I../ -o test_bit_prev bad_lcg64.c test_bit_prev.c
gcc -O1 -lm -I../ -o test_bit_dist bad_lcg64.c test_bit_dist.c
gcc -O1 -lm -I../ -o test_bit_cur bad_lcg64.c test_bit_cur.c
*/
