#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <float.h>

/* 32 bit Multiply-With-Carry (MWC) with Lag 2
 *
 * Author: James Carter
 *
 * Tested with the TestU01 library from
 * http://www.iro.umontreal.ca/~simardr/testu01/tu01.html
 * Passes all of the tests of SmallCrush, FIPS-140-2, and pseudoDIEHARD
 * Passes all of the tests of Crush and BigCrush
 *
 * Criteria for MWCs:
 * For a given base B and lag L, chose multiplier A such that:
 * A*B^L-1 and A*B^L/2-1 are both prime.
 * This will give a period of A*B^L/2-1
 *
 * Using:
 * A = 4294095429 [3*13*110105011]  (FFF2B245)
 * B = 2^32
 * L = 2
 *
 * Period: ~2^95
 */

struct rnd {
	uint32_t s1;
	uint32_t s2;
	uint32_t c;
};

#define A 4294095429ULL

#define L32(x) ((x)&0xFFFFFFFFUL)
#define SR32(x) ((x)>>32)

static inline uint32_t next(struct rnd *rnd)
{
	uint64_t x = rnd->s1*A+rnd->c;
	rnd->s1 = rnd->s2;
	rnd->s2 = L32(x);
	rnd->c = SR32(x);
	return rnd->s2;
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

void rnd_init(struct rnd *rnd, unsigned long seed)
{
	rnd->s1 = seed;
	rnd->s2 = 2147483647UL; /* 1st prime < 2^31 */
	rnd->c  = 1073741789UL; /* 1st prime < 2^30 */
	next(rnd);
	next(rnd);
	next(rnd);
}

unsigned rnd_get_state_size()
{
	return sizeof(struct rnd);
}

void rnd_get_state(struct rnd *rnd, unsigned long state[])
{
	state[0] = rnd->s1;
	state[1] = rnd->s2;
	state[2] = rnd->c;
}

void rnd_set_state(struct rnd *rnd, unsigned long state[])
{
	rnd->s1 = state[0];
	rnd->s2 = state[1];
	rnd->c  = state[2];
}

void rnd_free(struct rnd *rnd)
{
	free(rnd);
}

/*
 * Random Numbers
 */

#define UMAX 4294967295ULL
#define HIGH_OPEN (1.0 - DBL_EPSILON)
#define LOW_OPEN (DBL_EPSILON)
#define CLOSED(x) (((double)(x))/((double)(UMAX)))
#define OPEN1(x) (CLOSED(x)*(HIGH_OPEN-LOW_OPEN)+LOW_OPEN)
#define OPEN2(x) (CLOSED(x)*(HIGH_OPEN-LOW_OPEN)+2.0*LOW_OPEN)
#define OPENn(x,n) (CLOSED(x)*(HIGH_OPEN-LOW_OPEN)+(n)*LOW_OPEN)

unsigned long rnd_number_max()
{
	return UMAX;
}

unsigned long rnd_number(struct rnd *rnd)
{
	return next(rnd);
}

double rnd_closed(struct rnd *rnd)
{
	/* Return double [0,1] in continuous uniform distribution */
	return CLOSED(next(rnd));
}

double rnd_double(struct rnd *rnd)
{
	/* Return double (0,1) in continuous uniform distribution */
	return OPEN1(next(rnd));
}

double rnd_double_2(struct rnd *rnd)
{
	/* Return double (0,2) in continuous triangular distribution */
	return OPEN2((double)next(rnd)+(double)next(rnd));
}

double rnd_double_n(struct rnd *rnd, unsigned n)
{
	/* Return double (0,n) in continuous irwin hall distribution */
	unsigned i = n;
	double x = 0;
	for (i = 0; i<n; i++) {
		x += next(rnd);
	}
	return OPENn(x,n);
}
