#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <float.h>

/* 60 bit Multiply-With-Carry (MWC) with lag 3
 *
 * Author: James Carter
 *
 * Tested using the TestU01 library by Pierre L'Ecuyer of the Universite de 
 * Montreal. [http://www.iro.umontreal.ca/~simardr/testu01/tu01.html]
 * Passes all of the tests of SmallCrush, FIPS-140-2, and pseudoDIEHARD
 * Passes all of the tests of Crush and BigCrush
 *
 * Criteria for MWCs:
 * For a given base B and lag L, chose multiplier A such that:
 * A*B^L-1 and A*B^L/2-1 are both prime.
 * This will give a period of A*B^L/2-1
 *
 * Using:
 * A = 1152921504589211409  [3*13*29562089861261831]  (0FFFFFFFFEF2E711)
 * B = 2^60
 * L = 3
 * Period: ~2^239
 */

#define A1 0x3EF2E711UL /* Lower 30-bits */
#define A2 0x3FFFFFFFUL /* Upper 30-bits */

struct rnd {
	uint64_t s1;
	uint64_t s2;
	uint64_t s3;
	uint64_t c;
};

/*
 * Internal - RNG Core
 */

#define L30(x) ((x)&0x3FFFFFFFULL)
#define SR30(x) ((x)>>30)
#define SL30(x) ((x)<<30)

static inline uint64_t next(struct rnd *state)
{
	uint64_t s = state->s1;
	uint64_t n1 = L30(s);
	uint64_t n2 = SR30(s);
	uint64_t x1 = n1*A1 + state->c;
	uint64_t x2 = n1*A2 + n2*A1 + SR30(x1);
	state->s1 = state->s2;
	state->s2 = state->s3;
	state->s3 = SL30(L30(x2)) | L30(x1);
	state->c = n2*A2 + SR30(x2);
	return state->s3;
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

#define L60(x) ((x)&0xFFFFFFFFFFFFFFFULL)

void rnd_init(struct rnd *state, unsigned long seed)
{
	state->s1 = L60(seed);
	state->s2 = 576460752303423433ULL;  /* 1st prime < 2^59 */
	state->s3 = 288230376151711717ULL;  /* 1st prime < 2^58 */
	state->c  = 144115188075855859ULL;  /* 1st prime < 2^57 */
	next(state);
	next(state);
	next(state);
	next(state);
}

void rnd_free(struct rnd *rnd)
{
	if (!rnd)
		return;
	free(rnd);
}


/*
 * Random Numbers
 */

#define UMAX 1125899906842623ULL /* 2^50-1 */
#define L50(x) ((x)&0x3FFFFFFFFFFFFULL)
#define HIGH_OPEN (1.0 - DBL_EPSILON)
#define LOW_OPEN (DBL_EPSILON)
#define CLOSED(x) (((double)(L50(x)))/((double)(UMAX)))
#define OPEN1(x) (CLOSED(x)*(HIGH_OPEN-LOW_OPEN)+LOW_OPEN)
#define OPEN2(x) (CLOSED(x)*(HIGH_OPEN-LOW_OPEN)+2.0*LOW_OPEN)
#define OPENn(x,n) (CLOSED(x)*(HIGH_OPEN-LOW_OPEN)+(n)*LOW_OPEN)

unsigned rnd_unsigned(struct rnd *rnd)
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
	return OPEN2((double)next(rnd) + (double)next(rnd));
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
