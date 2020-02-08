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

/* rnd_190 - Pseudorandom number generator with period ~2^190
 *
 * Author: James Carter
 *
 * Full 64-bit outputs from two 32-bit Multiply-With-Carry (MWC) Lag 2
 * generators are mixed by rotating the output of the second generator
 * by 32-bits and adding it to the output of the first generator.
 *
 * Tested with the TestU01 library from
 * http://www.iro.umontreal.ca/~simardr/testu01/tu01.html
 * Passes all of the tests of BigCrush for the following 32-bit ranges:
 *   03-36, 07-40, 11-44, 15-48, 19-52, 23-56, 27-60, 31-00, 35-04, 39-08,
 *   43-12, 47-16, 51-20, 55-24, 59-28, 63-32
 * No ranges failed any of the BigCrush tests.
 *
 * Criteria for MWCs:
 * For a MWC with base B and lag L, chose multiplier A such that
 * A*B^L-1 and A*B^L/2-1 are both prime for a max period of A*B^L/2-1
 * Using A=~2^32, B=2^32, L=2
 * P1 = A1*(2^32)^2/2-1 = ~2^95
 * P2 = A2*(2^32)^2/2-1 = ~2^95
 *
 * The two generators have periods that are relatively prime with each
 * other, so the overall period is ~2^95*2^95 = ~2^190
 */

struct rnd {
	uint32_t s1a;
	uint32_t s1b;
	uint32_t c1;
	uint32_t s2a;
	uint32_t s2b;
	uint32_t c2;
};

#define H32(x) ((x)>>32)
#define L32(x) ((x)&0xFFFFFFFFUL)
#define FLIP32(x) ((x)>>32 | (x)<<32)
#define A1 4293538899ULL /* MWC L2: 3 * 13 * 110090741 */
#define A2 4291750983ULL /* MWC L2: 3 * 13 * 110044897 */
								   
static inline uint64_t next_rnd(struct rnd *rnd)
{
	uint64_t x1 = rnd->s1a*A1+rnd->c1;
	rnd->s1a = rnd->s1b;
	rnd->s1b = x1;
	rnd->c1 = H32(x1);
	uint64_t x2 = rnd->s2a*A2+rnd->c2;
	rnd->s2a = rnd->s2b;
	rnd->s2b = x2;
	rnd->c2 = H32(x2);
	return x1 + FLIP32(x2);
}

#define Z3 3571494541ULL /* LCG: 91019 * 39239 */
#define Z4 3753453877ULL /* LCG: 13999 * 268123 */
#define Z5 18277323205306182053ULL /* LCG: 26777 * 65777 * 78787 * 131711 */
#define L31(x) ((x)&0x7FFFFFFFUL)
#define L30(x) ((x)&0x3FFFFFFFUL)

void rnd_init(struct rnd *rnd, unsigned long seed)
{
	int i=0;
	uint64_t x = (Z3 + seed)*Z4 + Z5;
	uint32_t z[6];

	while (i < 6) {
		x = (x + seed)*Z3 + Z4;
		if (L30(x) != 0) {
			z[i] = x;
			i++;
		}
	}
	rnd->s1a = L31(z[5]);
	rnd->s1b = L31(z[4]);
	rnd->c1  = L30(z[3]);
	rnd->s2a = L31(z[2]);
	rnd->s2b = L31(z[1]);
	rnd->c2  = L30(z[0]);
	for (i=0; i<13; i++)
		next_rnd(rnd);
}

unsigned rnd_get_state_size_bytes()
{
	return (sizeof (struct rnd));
}

void rnd_get_state(struct rnd *rnd, uint32_t state[])
{
	state[0] = rnd->s1a;
	state[1] = rnd->s1b;
	state[2] = rnd->c1;
	state[3] = rnd->s2a;
	state[4] = rnd->s2b;
	state[5] = rnd->c2;
}

void rnd_set_state(struct rnd *rnd, uint32_t state[])
{
	rnd->s1a = state[0];
	rnd->s1b = state[1];
	rnd->c1  = state[2];
	rnd->s2a = state[3];
	rnd->s2b = state[4];
	rnd->c2  = state[5];
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

