#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

/* 32 bit Multiply-With-Carry (MWC) with Lag 2
 *
 * 7.81 seconds for 1,000,000,000 calls
 *
 * Using the TestU01 library from
 * http://www.iro.umontreal.ca/~simardr/testu01/tu01.html
 * Passes all of the tests of SmallCrush, FIPS-140-2, and pseudoDIEHARD
 * Passes all of the tests of Crush and Bigcrush
 *
 * Criteria for MWCs:
 * For a given A with,
 * B = 2^32, and where L = Lag length,
 * A*B^L-1 and (A*B^L)/2-1 must be prime
 *
 * Using:
 * A = 1326028899 [3 * 13 * 101 * 227 * 1483]
 */

struct rnd {
	unsigned long s1;
	unsigned long s2;
	unsigned long c;
};

/*
 * Internal - RNG Core
 */

#define L32(x) ((x)&0xFFFFFFFFUL)
#define H32(x) ((x)>>32)
#define A 1326028899ULL

static inline unsigned long next(struct rnd *rnd)
{
	unsigned long long x = (unsigned long long)rnd->s1*A+rnd->c;
	rnd->s1 = rnd->s2;
	rnd->s2 = L32(x);
	rnd->c = H32(x);
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
	rnd->s2 = 1405695061UL; /* Prime */
	rnd->c = 96557UL;       /* Prime */
	next(rnd);
	next(rnd);
}

void rnd_free(struct rnd *rnd)
{
	if (rnd)
		free(rnd);
}

/*
 * Random Numbers
 */

#define RND_MAX 4294967295UL
#define HIGH_OPEN (1.0-DBL_EPSILON)
#define LOW_OPEN DBL_EPSILON

/*
 * Uniform Distribution
 */

double rnd_closed(struct rnd *rnd)
{
/* Return double [0,1] in continuous uniform distribution */
	return (double)next(rnd)/(double)RND_MAX;
}

double rnd_double(struct rnd *rnd)
{
/* Return double [0,1) in continuous uniform distribution */
	return (double)next(rnd)/(double)RND_MAX*HIGH_OPEN;
}

double rnd_open(struct rnd *rnd)
{
/* Return double (0,1) in continuous uniform distribution */
	return (double)next(rnd)/(double)RND_MAX*(HIGH_OPEN-LOW_OPEN)+LOW_OPEN;
}

/*
 * Triangular Distribution
 */

double rnd_closed_2(struct rnd *rnd)
{
/* Return double [0,2] in continuous triangular distribution */
	return ((double)next(rnd)+(double)next(rnd))/(double)RND_MAX;
}

double rnd_double_2(struct rnd *rnd)
{
/* Return double [0,2) in continuous triangular distribution */
	return ((double)next(rnd)+(double)next(rnd))/(double)RND_MAX*HIGH_OPEN;
}

double rnd_open_2(struct rnd *rnd)
{
/* Return double (0,2) in continuous triangular distribution */
	return ((double)next(rnd)+(double)next(rnd))/(double)RND_MAX*
			(HIGH_OPEN-LOW_OPEN)+LOW_OPEN;
}

/*
 * Irwin Hall Distribution
 */

double rnd_closed_n(struct rnd *rnd, unsigned n)
{
/* Return double [0,n] in continuous irwin hall distribution */
	double x = 0;
	while (n-- > 0)
		x += next(rnd);
	return (double)x/(double)RND_MAX;
}

double rnd_double_n(struct rnd *rnd, unsigned n)
{
/* Return double [0,n) in continuous irwin hall distribution */
	double x = 0;
	while (n-- > 0)
		x += next(rnd);
	return (double)x/(double)RND_MAX*HIGH_OPEN;
}

double rnd_open_n(struct rnd *rnd, unsigned n)
{
/* Return double (0,n) in continuous irwin hall distribution */
	double x = 0;
	while (n-- > 0)
		x += next(rnd);
	return (double)x/(double)RND_MAX*(HIGH_OPEN-LOW_OPEN)+LOW_OPEN;
}

/*
 * RNG State
 */

/* char *rnd_state_to_string(struct rnd *rnd) */
/* { */
/* } */

/* struct rnd *rnd_string_to_state(struct rnd *rnd, char *state) */
/* { */
/* } */

/* void rnd_free_state_string(char *state) */
/* { */
/* } */
