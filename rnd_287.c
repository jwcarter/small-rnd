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

/* rnd_287 - Pseudorandom number generator with period ~2^95
 *
 * Author: James Carter
 *
 * Full 64-bit output from a 32-bit Multiply-With-Carry (MWC) Lag 8
 * generator is mixed by multiplying the upper and lower 32-bits
 * with 32-bit numbers, rotating the second product by 32-bits and
 * adding it to the first product.
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
 * Using A=~2^32, B=2^32, L=3: Period = A*(2^32)^8/2-1 = ~2^287
 */

struct rnd {
	uint32_t n;
	uint32_t c;
	uint32_t s[8];
};

#define H32(x) ((x)>>32)
#define L32(x) ((x)&0xFFFFFFFFUL)
#define FLIP32(x) ((x)>>32 | (x)<<32)
#define A  4293082443ULL /* MWC L8: 3 * 13 * 110079037 */
#define Z1 4078645709ULL /* LCG: 77999 * 52291 */
#define Z2 3580663381ULL /* LCG: 68111 * 52571 */
   
static inline uint64_t next_rnd(struct rnd *rnd)
{
	uint32_t n = rnd->n & 0x7;
	uint64_t x1 = rnd->s[n]*A+rnd->c;
	rnd->s[n] = x1;
	rnd->c = H32(x1);
	rnd->n++;
	uint64_t z1 = Z1 * H32(x1);
	uint64_t z2 = Z2 * L32(x1);
	return z1 + FLIP32(z2);
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
	uint32_t z[10];

	while (i < 10) {
		x = (x + seed)*Z3 + Z4;
		if (L30(x) != 0) {
			z[i] = x;
			i++;
		}
	}
	rnd->n = z[9];
	rnd->c = L30(z[8]);
	for (i=0; i<8; i++) {
		rnd->s[i] = L31(z[7-i]);
	}
	for (i=0; i<37; i++)
		next_rnd(rnd);
}

unsigned rnd_get_state_size_bytes()
{
	return (sizeof (struct rnd));
}

void rnd_get_state(struct rnd *rnd, uint32_t state[])
{
	int i;
	state[0] = rnd->n;
	state[1] = rnd->c;
	for (i=0; i<8; i++) {
		state[2+i] = rnd->s[i];
	}
}

void rnd_set_state(struct rnd *rnd, uint32_t state[])
{
	int i;
	rnd->n = state[0];
	rnd->c = state[1];
	for (i=0; i<8; i++) {
		rnd->s[i] = state[2+i];
	}
}

#define INV52M1 2.2204460492503136e-16 /* 1/(2^52-1) */
#define INV52P1 2.2204460492503126e-16 /* 1/(2^52+1) */
#define L52(x) ((x)&0xFFFFFFFFFFFFFULL)
#define CLOSED(x) ((double)L52(x)*INV52M1)
#define OPEN(x) ((double)(L52(x)+1)*INV52P1)

uint32_t rnd_u32(struct rnd *rnd)
{
	return L32(next_rnd(rnd));
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
