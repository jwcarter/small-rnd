#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <float.h>

/* 32 bit Multiply-With-Carry (MWC) with Lag 3 with rotation
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
 * A = 4293666429  [3*13*110094011]  (FFEC267D)
 * B = 2^32
 * L = 3
 *
 * Period: ~2^127
 */

#define A 4293666429ULL

#define K1 4110542893ULL /* 59443 * 69151 = 0xF501E82D */
#define K2 3667735229ULL /* 51407 * 71347 = 0xDA9D32BD */
#define K3 4068774709ULL /* 46399 * 87691 = 0xF2849335 */
#define K4 3322924477ULL /* 44939 * 73943 = 0xC60FCDBD */

#define L52(x) ((x)&0xFFFFFFFFFFFFFUL)
#define H32(x) ((x)>>32)
#define L32(x) ((x)&0xFFFFFFFFUL)
#define L31(x) ((x)&0x7FFFFFFFUL)
#define L30(x) ((x)&0x3FFFFFFFUL)
#define RL64(x,r) ((x)<<(r) | (x)>>(64-(r)))

struct rnd {
	uint32_t s1;
	uint32_t s2;
	uint32_t s3;
	uint32_t c;
};

static inline uint64_t next(struct rnd *rnd)
{
	uint64_t x = rnd->s1*A+rnd->c;
	rnd->s1 = rnd->s2;
	rnd->s2 = rnd->s3;
	rnd->s3 = x;
	rnd->c = H32(x);
	return x + RL64(x,13);
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
	int i;
	uint32_t x1 = ((seed + K1) * K2) + K3;
	uint32_t x2 = ((seed + x1) * K3) + K4;
	uint32_t x3 = ((seed + x2) * K4) + K1;
	uint32_t x4 = ((seed + x3) * K1) + K2;
	rnd->s1 = L31(x1);
	rnd->s2 = L31(x2);
	rnd->s3 = L31(x3);
	rnd->c  = L30(x4);
	for (i=0; i<4; i++)
		next(rnd);
}

unsigned rnd_get_state_size()
{
	return sizeof(struct rnd);
}

void rnd_get_state(struct rnd *rnd, uint32_t state[])
{
	state[0] = rnd->s1;
	state[1] = rnd->s2;
	state[2] = rnd->s3;
	state[3] = rnd->c;
}

void rnd_set_state(struct rnd *rnd, uint32_t state[])
{
	rnd->s1 = state[0];
	rnd->s2 = state[1];
	rnd->s3 = state[2];
	rnd->c  = state[3];
}

void rnd_free(struct rnd *rnd)
{
	free(rnd);
}

/*
 * Random Numbers
 */

#define UMAX32 4294967295ULL       /* 2^32-1 */
#define UMAX52 4503599627370495ULL /* 2^52-1 */
#define HIGH_OPEN (1.0 - DBL_EPSILON)
#define LOW_OPEN (DBL_EPSILON)
#define CLOSED(x) (((double)(x))/((double)(UMAX52)))
#define OPEN1(x) (CLOSED(x)*(HIGH_OPEN-LOW_OPEN)+LOW_OPEN)
#define OPEN2(x) (CLOSED(x)*(HIGH_OPEN-LOW_OPEN)+2.0*LOW_OPEN)
#define OPENn(x,n) (CLOSED(x)*(HIGH_OPEN-LOW_OPEN)+(n)*LOW_OPEN)

uint32_t rnd_unsigned32(struct rnd *rnd)
{
	return L32(next(rnd));
}

uint64_t rnd_unsigned64(struct rnd *rnd)
{
	return next(rnd);
}

double rnd_closed(struct rnd *rnd)
{
	/* Return double [0,1] in continuous uniform distribution */
	return CLOSED(L52(next(rnd)));
}

double rnd_double(struct rnd *rnd)
{
	/* Return double (0,1) in continuous uniform distribution */
	return OPEN1(L52(next(rnd)));
}

double rnd_double_2(struct rnd *rnd)
{
	/* Return double (0,2) in continuous triangular distribution */
	return OPEN2((double)L52(next(rnd))+(double)L52(next(rnd)));
}

double rnd_double_n(struct rnd *rnd, unsigned n)
{
	/* Return double (0,n) in continuous irwin hall distribution */
	unsigned i = n;
	double x = 0;
	for (i = 0; i<n; i++) {
		x += L52(next(rnd));
	}
	return OPENn(x,n);
}
