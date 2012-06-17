#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

/* 32 bit Multiply-With-Carry (MWC) with Lag 5
 *
 * 9.11 seconds for 1,000,000,000 calls
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
 * A = 1326557154 [2 * 3 * 13 * 23 * 229 * 3229]
 */

struct rnd {
	unsigned n;
	unsigned long c;
	unsigned long s[5];
};

/*
 * Internal - RNG Core
 */

#define L32(x) ((x)&0xFFFFFFFFUL)
#define H32(x) ((x)>>32)
#define A 1326557154ULL

static inline unsigned long next(struct rnd *rnd)
{
	unsigned n = rnd->n;
	unsigned long v;
	unsigned long long x = (unsigned long long)rnd->s[n]*A+rnd->c;
	v = L32(x);
	rnd->s[n] = v;
	rnd->c = H32(x);
	rnd->n = (n == 4) ? 0 : n+1;
	return v;
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
	unsigned long c = 96557UL;
	unsigned long s = seed;
	unsigned long long x;
	int i;

	for (i=0; i<5; i++) {
		x = (unsigned long long)s*A+c;
		c = H32(x);
		s = L32(x);
		rnd->s[i] = s;
	}
	rnd->c = c;
	rnd->n = 0;
	for (i=0; i<5; i++)
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
	return OPEN_DBL((double)next(rnd)+(double)next(rnd));
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
