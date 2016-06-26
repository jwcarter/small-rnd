#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

/* rnd126 - Pseudorandom number generator with period ~2^126
 *
 * Author: James Carter
 *
 * Full 64-bit output from two 32-bit Multiply-With-Carry (MWC)
 * generators are added together
 *
 * Tested with the TestU01 library from
 * http://www.iro.umontreal.ca/~simardr/testu01/tu01.html
 * Passes all of the tests of BigCrush.
 * Four 32-bit ranges tested: 64-33, 48-17, 32-1, and 16-49
 *
 * Criteria for MWCs:
 * For a MWC with base B and lag L, chose multiplier A such that
 * A*B^L-1 and A*B^L/2-1 are both prime for a max period of A*B^L/2-1
 * Using A=~2^32, B=2^32, L=1
 * P1 = A1*2^32/2-1 = ~2^63
 * P2 = A2*2^32/2-1 = ~2^63
 *
 * The two generators have periods that are relatively prime with each
 * other, so the overall period is ~2^63*2^64 = ~2^126
 */

struct rnd {
	uint64_t s1;
	uint64_t s2;
};

#define H32(x) ((x)>>32)
#define L32(x) ((x)&0xFFFFFFFFUL)
#define A1 4294095429ULL /* K1: MWC L1: 3 * 13 * 110105011 */
#define A2 4293977883ULL /* K2: MWC L1: 3 * 13 * 110101997 */
								   
static inline uint64_t next_rnd(struct rnd *rnd)
{
	uint64_t x1 = L32(rnd->s1)*A1+H32(rnd->s1);
	uint64_t x2 = L32(rnd->s2)*A2+H32(rnd->s2);
	rnd->s1 = x1;
	rnd->s2 = x2;
	return x1 + x2;
}

#define Z1 4078645709ULL /* K6: LCG: 77999 * 52291 */
#define Z2 3580663381ULL /* K7: LCG: 68111 * 52571 */
#define MASK(x) ((x)&0x7FFFFFFF3FFFFFFFULL)

void rnd_init(struct rnd *rnd, unsigned long seed)
{
	int i;
	uint64_t x = seed*Z1 + seed*Z2 + Z1*Z2 + Z2;
	x = (MASK(x) != 0) ? x : Z1;
	rnd->s1 = MASK(x);
	x = x*Z1 + x*Z2 + Z1*Z2 + Z1;
	x = (MASK(x) != 0) ? x : Z2;
	rnd->s2 = MASK(x);
	for (i=0; i<11; i++)
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
	state[2] = H32(rnd->s2);
	state[3] = L32(rnd->s2);
}

#define SL32(x) ((uint64_t)(x)<<32)

void rnd_set_state(struct rnd *rnd, uint32_t state[])
{	
	rnd->s1 = SL32(state[0]) | state[1];
	rnd->s2 = SL32(state[2]) | state[3];
}

#define INV52M1 2.2204460492503135739e-16 /* 1/(2^52-1) */
#define INV52P1 2.2204460492503125878e-16 /* 1/(2^52+1) */
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

