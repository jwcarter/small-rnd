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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

/* rnd_63 - Pseudorandom number generator with period ~2^63
 *
 * Author: James Carter
 *
 * Full 64-bit output from a 32-bit Multiply-With-Carry (MWC)
 * generator is mixed by multiplying the upper and lower 32-bits
 * with 32-bit numbers, rotating one of the products by 32-bits
 * and adding with the other product.
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
 * Tested with the TestU01 library from
 * http://www.iro.umontreal.ca/~simardr/testu01/tu01.html
 * Passes all of the tests of BigCrush for the following 32-bit ranges:
 *   03-36, 07-40, 11-44, 15-48, 19-52, 23-56, 31-00, 35-04, 39-08, 47-16,
 *   51-20, 59-28, 63-32
 * The following ranges fail the listed tests:
 *   27-60: 14  BirthdaySpacings, t = 3
 *   43-12: 13  BirthdaySpacings, t = 2
 *          15  BirthdaySpacings, t = 4
 *   55-24: 13  BirthdaySpacings, t = 2
 *
 * test_birthday results:
 * Bits used: 32  t: 2
 *   Left 57-55, 52, 48, 44-43, 33-32, 22, 17  : p-value > 1-1e-15
 *   Left 63-62, 42                            : p-value > 1-1e-09
 *   Left 54                                   : p-value > 1-1e-06
 * Bits used: 16  t: 4
 *   Left 38, 29, 19, 18                       : p-value > 1-1e-15
 *   Left 39                                   : p-value > 1-1e-12
 */

struct rnd {
	uint64_t s1;
};

#define H32(x) ((x)>>32)
#define L32(x) ((x)&0xFFFFFFFFUL)
#define FLIP32(x) ((x)>>32 | (x)<<32)
#define A  4294095429ULL /* MWC L1: 3 * 13 * 110105011 */
#define Z1 4078645709ULL /* LCG: 77999 * 52291 */
#define Z2 3580663381ULL /* LCG: 68111 * 52571 */
							   
static inline uint64_t next_rnd(struct rnd *rnd)
{
	uint64_t x1 = L32(rnd->s1)*A+H32(rnd->s1);
	rnd->s1 = x1;
	uint64_t z1 = Z1 * H32(x1);
	uint64_t z2 = Z2 * L32(x1);
	return z1 + FLIP32(z2);
}

#define Z3 3571494541ULL /* LCG: 91019 * 39239 */
#define Z4 3753453877ULL /* LCG: 13999 * 268123 */
#define L31(x) ((x)&0x7FFFFFFFUL)

void rnd_init(struct rnd *rnd, unsigned long seed)
{
	int i = 0;
	uint32_t z[3];
	uint32_t x = Z4;
	while (i < 3) {
		x = ((x + seed) * Z3) + Z4;
		if (L31(x) != 0) {
			z[i] = L31(x);
			i++;
		}
	}
	rnd->s1 = (uint64_t)z[2] << 32 | (uint64_t)z[1];
	for (i=0; i<13; i++)
		next_rnd(rnd);
}

unsigned rnd_get_state_size_bytes()
{
	return (sizeof (struct rnd));
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

gcc -O1 -lm -o rnd_bit_tests rnd.c rnd_63.c rnd_bit_tests.c
gcc -O1 -lm -o rnd_dice_test rnd.c rnd_63.c rnd_dice_test.c
gcc -O1 -lm -I/home/jim/projects/rnd/ -o test_birthday rnd.c rnd_63.c extras/test_birthday.c

*/
