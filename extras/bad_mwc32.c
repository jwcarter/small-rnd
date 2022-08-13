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

/* bad_mwc32 - Pseudorandom number generator with period ~2^63
 *
 * Author: James Carter
 *
 * Full 64-bit output from a 32-bit Multiply-With-Carry (MWC)
 * generator is used.
 *
 * Criteria for MWCs:
 * For a MWC with base B and lag L, chose multiplier A such that
 * A*B^L-1 and A*B^L/2-1 are both prime for a max period of A*B^L/2-1
 * Using A=~2^32, B=2^32, L=1: Period = A1*2^32/2-1 = ~2^63
 *
 * Pick initial [c,s] such that 0<=c<=A and 0<=x<B and exclude [0,0] and
 * [A-1,B-1] (See "Random Number Generators" by George Marsaglia in the
 * Journal of Modern Applied Statistical Methods, May 2003.)
 *
 * test_birthday results:
 * Bits used: 16  t: 4
 *   Left 43          : p-value > 1-1e-15
 *   Left 57, 36, 33  : p-value > 1-1e-06
 *
 * rnd_bit_tests results:
 * 2^14 trials
 *   Bits 52-20, 53-21, 54-22, 55-23, 56-24, 57-25,
 *        58-26, 59-27, 60-28, 61-29, 62-30, 63-31   : p-value > 1-1e-15
 *   Bits 51-19                                      : p-value < 1e-15
 *   Bits 50-18                                      : p-value < 1e-12
 *   Test aborts due to the number of failures
 *
 * rnd_dice_tests results:
 * 2^27 trials
 *   Horizontal Bits 60-64                           : p-value < 1e-15
 *   Horizontal Bits 52-56, 56-60                    : p-value < 1e-12
 * 2^28 trials
 *   Horizontal Bits 52-56, 56-60                    : p-value < 1e-15
 * 2^30 trials
 *   Vertical Bits 54                                : p-value < 1e-12
 *   Vertical Bits 55-63                             : p-value < 1e-06
 * 2^31 trials
 *   Vertical Bits 54-55, 58-59, 61-63               : p-value < 1e-15
 *   Vertical Bits 56, 60                            : p-value < 1e-12
 *   Vertical Bits 52, 53, 57                        : p-value < 1e-09
 *   Test aborts due to the number of failures
 *
 * test_bit_prev results:
 *   47-15, 48-16, 49-17, 50-18, 51-19, 52-20,
 *   53-21, 54-22, 55-23, 56-24, 57-25, 58-26,
 *   59-27, 60-28, 61-29, 62-30, 63-31          : p-value < 1e-15
 */

struct rnd {
	uint64_t s1;
};

#define H32(x) ((x)>>32)
#define L32(x) ((x)&0xFFFFFFFFUL)
#define FLIP32(x) ((x)>>32 | (x)<<32)
#define RR64(x,r) ((x)>>(r) | (x)<<(64-(r)))
#define A  4294095429ULL /* MWC L1: 3 * 13 * 110105011 */
#define Z1 4078645709ULL /* LCG: 77999 * 52291 */
#define Z2 3580663381ULL /* LCG: 68111 * 52571 */
							   
static inline uint64_t next_rnd(struct rnd *rnd)
{
	uint64_t x1 = L32(rnd->s1)*A+H32(rnd->s1);
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
gcc -O1 -lm -I../ -o test_birthday bad_mwc32.c test_birthday.c
gcc -O1 -lm -I../ -o rnd_dice_test bad_mwc32.c ../rnd_dice_test.c
gcc -O1 -lm -I../ -o rnd_bit_tests bad_mwc32.c ../rnd_bit_tests.c
gcc -O1 -lm -I../ -o test_bit_prev bad_mwc32.c test_bit_prev.c
gcc -O1 -lm -I../ -o test_bit_dist bad_mwc32.c test_bit_dist.c
gcc -O1 -lm -I../ -o test_bit_cur bad_mwc32.c test_bit_cur.c
*/
