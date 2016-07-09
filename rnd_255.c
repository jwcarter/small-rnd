#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

/* rnd255 - Pseudorandom number generator with period ~2^255
 *
 * Author: James Carter
 *
 * Full 64-bit output from a 32-bit Multiply-With-Carry (MWC) Lag 7
 * generator is mixed by multiplying the upper and lower 32-bits
 * with 32-bit numbers, rotating one of the products by 32-bits
 * and adding with the other product.
 *
 * Tested with the TestU01 library from
 * http://www.iro.umontreal.ca/~simardr/testu01/tu01.html
 * Passes all of the tests of BigCrush.
 * Four 32-bit ranges tested: 64-33, 48-17, 32-1, and 16-49
 *
 * Criteria for MWCs:
 * For a MWC with base B and lag L, chose multiplier A such that
 * A*B^L-1 and A*B^L/2-1 are both prime for a max period of A*B^L/2-1
 * Using A=~2^32, B=2^32, L=7: Period = A1*(2^32)^7/2-1 = ~2^255
 */

struct rnd {
	uint32_t n;
	uint32_t c;
	uint32_t s[7];
};

#define H32(x) ((x)>>32)
#define L32(x) ((x)&0xFFFFFFFFUL)
#define FLIP32(x) ((x)>>32 | (x)<<32)
#define A 4294258449ULL  /* MWC L7: 3 * 13 * 110109191 */
#define Z1 4078645709ULL /* LCG: 77999 * 52291 */
#define Z2 3580663381ULL /* LCG: 68111 * 52571 */
const uint8_t next7[7] = {1, 2, 3, 4, 5, 6, 0};
							   
static inline uint64_t next_rnd(struct rnd *rnd)
{
	uint32_t n = rnd->n;
	uint64_t x = rnd->s[n]*A+rnd->c;
	rnd->s[n] = x;
	rnd->c = H32(x);
	rnd->n = next7[n];
	uint64_t z1 = Z1 * H32(x);
	uint64_t z2 = Z2 * L32(x);
	return z1 + FLIP32(z2);
}

#define Z3 3571494541ULL /* LCG: 91019 * 39239 */
#define Z4 3753453877ULL /* LCG: 13999 * 268123 */
#define Z5 18277323205306182053ULL /* LCG: 26777 * 65777 * 78787 * 131711 */
#define L31(x) ((x)&0x7FFFFFFFUL)
#define L30(x) ((x)&0x3FFFFFFFUL)

void rnd_init(struct rnd *rnd, unsigned long seed)
{
	int i;
	uint32_t x = seed*Z1 + seed*Z2 + Z3;
	rnd->n = x % 7;
	x = x*Z1 + x*Z2 + Z5;
	x = (L30(x) != 0) ? x : (Z5 + Z1);
	rnd->c = L30(x);
	for (i=0; i<7; i++) {
		x = x*Z4 + x*Z3 + Z2;
		x = (L31(x) != 0) ? x : (Z5 + Z1);
		rnd->s[i] = L31(x);
	}
	for (i=0; i<11; i++)
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
	for (i=0; i<7; i++)
		state[2+i] = rnd->s[i];
}

#define SL32(x) ((uint64_t)(x)<<32)

void rnd_set_state(struct rnd *rnd, uint32_t state[])
{
	int i;
	rnd->n = state[0] % 7;
	rnd->c = state[1];
	for (i=0; i<7; i++)
		rnd->s[i] = state[2+i];
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
