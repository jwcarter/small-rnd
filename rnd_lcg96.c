#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

/* 96 bit LCG  -- Values selected by James Carter
 *
 * 13.76 seconds for 1,000,000,000 calls
 * M = 2^96
 * A = 38684117612769018905840676133  (19577194573 * 1405695061 * 1405695061)
 * C = 904120679537356008193          (94418953 * 3276509 * 2922509)
 *
 * Passes all of the tests of SmallCrush, FIPS-140-2, and pseudoDIEHARD
 * Passes all of the tests of Crush and BigCrush
 *
 * Doing 96 bit multiplication (mod 2^96) using 64 bit variables.
 * Ignoring endian concerns.
 *
 * Starting Bit:      96   64   32    0
 * ------------------------------------
 *                         n1   n2   n3
 *                     x   a1   a2   a3
 *                     ----------------
 *                       n1a3 n2a3 n3a3
 *                  n1a2 n2a2 n3a2
 *             n1a1 n2a1 n3a1
 *
 * All the values starting at 96 bits or more are ignored since this is done
 * mode 2^96.
 * So for each starting bit postion:
 *  0 Bit: n3a3
 * 32 Bit: n2a3+n3a2
 * 64 Bit: n1a2+n2a2+n3a1
 *
 * 1) Add the lower 16 bits of the data starting at bit 0 with the lowest
 *    16 bits of C.
 * 2) The lower 16 bits of state is the sum of the lower 16 bits from step 1.
 * 3) The upper 32 bits of state is the sum of the data starting at 16 bits,
 *    the data starting at 32 bits shifted up 16, and the upper 32 bits of C.
 *    No worries about overflow here.
 *
 * The highest 32 bits are used for the random number.
 */

struct rnd {
	unsigned long s1;
	unsigned long s2;
	unsigned long s3;
};

/*
 * Internal - RNG Core
 */

#define L32(x) ((x)&0xFFFFFFFFUL)
#define H32(x) ((x)>>32)
#define A1 0x7CFEC089UL
#define A2 0xD28BDF7EUL
#define A3 0x5A616D25UL
#define C1 0x00000031UL
#define C2 0x0331E7CDUL
#define C3 0x74E36B01UL

static inline unsigned long next(struct rnd *rnd)
{
	unsigned long long n1 = rnd->s1;
	unsigned long long n2 = rnd->s2;
	unsigned long long n3 = rnd->s3;
	unsigned long long x1, x2, x3, x4;
	x1 = n3*A3 + C3;
	x2 = n2*A3;
	x3 = n3*A2 + C2;
	x4 = L32(x2) + L32(x3) + H32(x1);
	rnd->s3 = L32(x1);
	rnd->s2 = L32(x4);
	rnd->s1 = L32(n1*A3 + n2*A2 + n3*A1 + C1 + H32(x2) + H32(x3) + H32(x4));
	return rnd->s1;
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
	rnd->s2 = 1405695061UL;
	rnd->s3 = 96557UL;
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
