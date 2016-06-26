#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <float.h>

/* rnd127 - Pseudorandom number generator with period ~2^127
 *
 * Author: James Carter
 *
 * Full 64-bit output from a 32-bit Multiply-With-Carry (MWC)
 * generator is added with the ouput of a 64-bit Add-with-Carry (AWC)
 *
 * Tested with the TestU01 library from
 * http://www.iro.umontreal.ca/~simardr/testu01/tu01.html
 * Passes all of the tests of BigCrush.
 * Four 32-bit ranges tested: 64-33, 48-17, 32-1, and 16-49
 *
 * Criteria for MWCs:
 * For a MWC with base B and lag L, chose multiplier A such that
 * A*B^L-1 and A*B^L/2-1 are both prime for a max period of A*B^L/2-1
 * Using A=~2^32, B=2^32, L=1: Period = A1*2^32/2-1 = ~2^63
 *
 * The AWC generator only has a lag of 1. On its own it fail any
 * tests for randomness. The period is 2^64 since the constant being
 * added is relatively prime with 2^64.
 *
 * The two generators have periods that are relatively prime with each
 * other, so the overall period is ~2^63*2^64 = ~2^127
 */

struct rnd {
	uint64_t s1;
	uint64_t s2;
};

#define H32(x) ((x)>>32)
#define L32(x) ((x)&0xFFFFFFFFUL)
#define A 4294095429ULL /* K1: MWC L1: 3 * 13 * 110105011 */
#define C 9390737881634721349ULL /* K10: LCG: 13613 * 68683 * 77377 * 129803 */								   
static inline uint64_t next_rnd(struct rnd *rnd)
{
	uint64_t x1 = L32(rnd->s1)*A+H32(rnd->s1);
	uint64_t x2 = rnd->s2 + C;
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
	x = (x == 0) ? x : Z1;
	rnd->s1 = MASK(x);
	rnd->s2 = x*Z1 + x*Z2 + Z1*Z2 + Z1;
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

#define H52(x) ((x)>>12)
#define UMAX52 4503599627370495ULL
#define HIGH_OPEN (1.0 - DBL_EPSILON)
#define LOW_OPEN (DBL_EPSILON)
#define CLOSED(x) (((double)(x))/((double)(UMAX52)))
#define OPEN(x) (CLOSED(x)*(HIGH_OPEN-LOW_OPEN)+LOW_OPEN)

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
	return CLOSED(H52(next_rnd(rnd)));
}

double rnd_open(struct rnd *rnd)
{
	/* Return double (0,1) in continuous uniform distribution */
	return OPEN(H52(next_rnd(rnd)));
}
