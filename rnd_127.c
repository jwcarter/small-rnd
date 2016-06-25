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

/*
 * Creation, Initialization, and Destruction
 */

struct rnd *rnd_new()
{
	struct rnd *rnd = malloc(sizeof(struct rnd));
	if (!rnd) {
		fprintf(stderr,"rnd: Malloc failed!");
		exit(-1);
	}
	return rnd;
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

#define RND_STATE_SIZE_BYTES (sizeof(struct rnd))

unsigned rnd_get_state_size_bytes()
{
	return RND_STATE_SIZE_BYTES;
}

#define RND_STATE_SIZE_U32 (RND_STATE_SIZE_BYTES/(sizeof (uint32_t)))

unsigned rnd_get_state_size_u32()
{
	return RND_STATE_SIZE_U32;
}

uint32_t *rnd_get_state(struct rnd *rnd)
{
	uint32_t *state = malloc(RND_STATE_SIZE_BYTES);
	if (!state) {
		fprintf(stderr,"rnd: Malloc failed!");
		exit(-1);
	}
	state[0] = H32(rnd->s1);
	state[1] = L32(rnd->s1);
	state[2] = H32(rnd->s2);
	state[3] = L32(rnd->s2);

	return state;
}

void rnd_free_state(uint32_t *state)
{
	free(state);
}

#define SL32(x) ((uint64_t)(x)<<32)

void rnd_set_state(struct rnd *rnd, uint32_t state[], unsigned size)
{
	unsigned s32,i,j;
	uint32_t a[RND_STATE_SIZE_U32];

	s32 = size/(sizeof(uint32_t));
	if (s32 > RND_STATE_SIZE_U32)
		s32 = RND_STATE_SIZE_U32;
	if (s32 == 0) {
		fprintf(stderr,"rnd: No state passed");
		exit(-1);
	}
	i=j=0;
	while (i < RND_STATE_SIZE_U32) {
		a[i] = state[j];
		i++;
		j++;
		if (j >= s32)
			j=0;
	}
	
	rnd->s1 = SL32(a[0]) | a[1];
	rnd->s2 = SL32(a[2]) | a[3];
}

void rnd_free(struct rnd *rnd)
{
	free(rnd);
}

/*
 * Random Numbers
 */

#define H52(x) ((x)>>12)
#define UMAX52 4503599627370495ULL
#define HIGH_OPEN (1.0 - DBL_EPSILON)
#define LOW_OPEN (DBL_EPSILON)
#define CLOSED(x) (((double)(x))/((double)(UMAX52)))
#define OPEN1(x) (CLOSED(x)*(HIGH_OPEN-LOW_OPEN)+LOW_OPEN)
#define OPEN2(x) (CLOSED(x)*(HIGH_OPEN-LOW_OPEN)+2.0*LOW_OPEN)
#define OPENn(x,n) (CLOSED(x)*(HIGH_OPEN-LOW_OPEN)+(n)*LOW_OPEN)

uint32_t rnd_unsigned32(struct rnd *rnd)
{
	return H32(next_rnd(rnd));
}

uint64_t rnd_unsigned64(struct rnd *rnd)
{
	return next_rnd(rnd);
}

double rnd_closed(struct rnd *rnd)
{
	/* Return double [0,1] in continuous uniform distribution */
	return CLOSED(H52(next_rnd(rnd)));
}

double rnd_double(struct rnd *rnd)
{
	/* Return double (0,1) in continuous uniform distribution */
	return OPEN1(H52(next_rnd(rnd)));
}

double rnd_double_2(struct rnd *rnd)
{
	/* Return double (0,2) in continuous triangular distribution */
	return OPEN2((double)H52(next_rnd(rnd))+(double)H52(next_rnd(rnd)));
}

double rnd_double_n(struct rnd *rnd, unsigned n)
{
	/* Return double (0,n) in continuous irwin hall distribution */
	unsigned i = n;
	double x = 0;
	for (i = 0; i<n; i++) {
		x += H52(next_rnd(rnd));
	}
	return OPENn(x,n);
}
