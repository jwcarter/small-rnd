#include <stdlib.h>
#include <stdio.h>

/* 32 bit Multiply-With-Carry (MWC) with Lag 2
 *
 * 7.81 seconds for 1,000,000,000 calls
 *
 * Using the TestU01 library from
 * http://www.iro.umontreal.ca/~simardr/testu01/tu01.html
 * Passes all of the tests of SmallCrush, FIPS-140-2, and pseudoDIEHARD
 * Passes all of the tests of Crush and BigCrush
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
	rnd->s2 = 1405695061UL; /* Prime - not that it matters */
	rnd->c = 96557UL;       /* Prime - not that it matters */
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

/*
 * All returned doubles are on an open interval.
 * For interval (0,1):
 * Min: 0.00000000000000232831
 * Max: 0.99999999999999766853
 * There is no round off when generating unsigned ints from 0 - U_MAX
 * like so: (unsigned)floor(OPEN_DBL(next(rnd))*(U_MAX+1))
 * Min: 0
 * Max: U_MAX
 * Because there is no round off, don't need to generate closed intervals
 */

#define U_MAX 4294967295UL
#define D 0.00001
#define D2 (2*(D))
#define OPEN_DBL(x) ((((double)x)+D)/((double)U_MAX+D2))

unsigned rnd_unsigned(struct rnd *rnd)
{
	return next(rnd);
}

double rnd_double(struct rnd *rnd)
{
/* Return double (0,1) in continuous uniform distribution */
	return OPEN_DBL(next(rnd));
}

double rnd_double_2(struct rnd *rnd)
{
/* Return double (0,2) in continuous triangular distribution */
	return OPEN_DBL(next(rnd)+next(rnd));
}

double rnd_double_n(struct rnd *rnd, unsigned n)
{
/* Return double (0,n) in continuous irwin hall distribution */
	double x = 0;
	while (n-- > 0)
		x += next(rnd);
	return OPEN_DBL(x);
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
