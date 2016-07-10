#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

#include <time.h> // For timing loop

#define A1 4294095429ULL /* MWC L1: 3 * 13 * 110105011 */
#define A2 4293977883ULL /* MWC L1: 3 * 13 * 110101997 */
#define A3 4293477123ULL /* MWC L1: 3 * 13 * 110089157 */
#define A4 4268670393ULL /* MWC L1: 3 * 13 * 109453087 */
#define A5 4293538899ULL /* MWC L2: 3 * 13 * 110090741 */
#define A6 4293666429ULL /* MWC L3: 3 * 13 * 110094011 */
#define A7 4294748679ULL /* MWC L6: 3 * 13 * 110121761 */
#define A8 4294258449ULL /* MWC L7: 3 * 13 * 110109191 */

/* LCG 64: */
#define A10 0x39f657e6d07ff2b5ULL
#define C10 0x04b1b67a68081529ULL

/* For MWC60 */
#define AA1 0x3F665FD9UL /* Lower 30-bits */
#define AA2 0x3FFFFFFFUL /* Upper 30-bits */

/* Other Constants */
#define Z1 4078645709ULL /* LCG: 77999 * 52291 */
#define Z2 3580663381ULL /* LCG: 68111 * 52571 */
#define Z3 3571494541ULL /* LCG: 91019 * 39239 */
#define Z4 3753453877ULL /* LCG: 13999 * 268123 */
#define Z5 18277323205306182053ULL /* LCG: 26777 * 65777 * 78787 * 131711 */
#define Z6  9390737881634721349ULL /* LCG: 13613 * 68683 * 77377 * 129803 */

#define L60(x) ((x)&0xFFFFFFFFFFFFFFFULL)
#define L59(x) ((x)&0x7FFFFFFFFFFFFFFULL)
#define H32(x) ((x)>>32)
#define L32(x) ((x)&0xFFFFFFFFUL)
#define L31(x) ((x)&0x7FFFFFFFUL)
#define L30(x) ((x)&0x3FFFFFFFUL)
#define SR30(x) ((x)>>30)
#define SL30(x) ((x)<<30)
#define FLIP32(x) ((x)>>32 | (x)<<32)
#define RR64(x,r) ((x)>>(r) | (x)<<(64-(r)))
#define RL64(x,r) ((x)<<(r) | (x)>>(64-(r)))
#define MASK64(x) ((x)&0x3FFFFFFF7FFFFFFFULL)

/* MWC32 with rotation
 * Regularly, but not always, fails BigCrush #15 BirthdaySpacings, t = 4
 * for the 32-bit range 48-17
 * Passes all of the tests for BigCrush for the 32-bit ranges:
 * 64-33, 32-1, and 16-49
 */
struct state_mwc32_r {
	uint64_t s1;
};

static inline uint64_t next_mwc32_r(struct state_mwc32_r *state)
{
	uint64_t x = L32(state->s1)*A1+H32(state->s1);
	state->s1 = x;
	return x + RR64(x,25);
}

static void init_mwc32_r(struct state_mwc32_r *state, uint32_t seed)
{
	int i;
	uint64_t x = seed*Z1 + seed*Z2 + Z3 + Z1;
	x = (MASK64(x) != 0) ? x : (Z3 + Z2);
	state->s1 = MASK64(x);
	for (i=0; i<11; i++)
		next_mwc32_r(state);
}
				
/* MWC32 M2 - 32-bit state variables */
struct state_mwc32_m2_32 {
	uint32_t s1;
	uint32_t c;
};

static inline uint64_t next_mwc32_m2_32(struct state_mwc32_m2_32 *state)
{
	uint64_t x = state->s1*A1+state->c;
	state->s1 = x;
	state->c = H32(x);
	uint64_t z1 = Z1 * H32(x);
	uint64_t z2 = Z2 * L32(x);
	return z1 + FLIP32(z2);
}

static void init_mwc32_m2_32(struct state_mwc32_m2_32 *state,
				uint32_t seed)
{
	int i;
	uint64_t x = seed*Z1 + seed*Z2 + Z5 + Z1;
	x = (MASK64(x) != 0) ? x : (Z5 + Z2);
	state->s1 = L31(x);
	state->c  = L30(H32(x));
	for (i=0; i<11; i++)
		next_mwc32_m2_32(state);
}

/* MWC32 M2 - 64-bit state variables
 * Passes all of the tests of BigCrush.
 * Four 32-bit ranges tested: 64-33, 48-17, 32-1, and 16-49
 */
struct state_mwc32_m2_64 {
	uint64_t s1;
};

static inline uint64_t next_mwc32_m2_64(struct state_mwc32_m2_64 *state)
{
	uint64_t x = L32(state->s1)*A1+H32(state->s1);
	state->s1 = x;	
	uint64_t z1 = Z1 * H32(x);
	uint64_t z2 = Z2 * L32(x);
	return z1 + FLIP32(z2);
}

static void init_mwc32_m2_64(struct state_mwc32_m2_64 *state,
				uint32_t seed)
{
	int i;
	uint64_t x = seed*Z3 + seed*Z4 + Z5 + Z3;
	x = (MASK64(x) != 0) ? x : (Z5 + Z4);
	state->s1 = MASK64(x);
	for (i=0; i<11; i++)
		next_mwc32_m2_64(state);
}

/* LCG 64 */
struct state_lcg64 {
  uint64_t s;
};

static inline uint64_t next_lcg64(struct state_lcg64 *state)
{
  state->s = state->s*A10+C10;
  return state->s;
}

static void init_lcg64(struct state_lcg64 *state, uint32_t seed)
{
	int i;
	uint64_t x = seed*Z1 + seed*Z2 + Z5 + Z1;
	x = (x != 0) ? x : (Z5 + Z2);
	state->s = x;
	for (i=0; i<11; i++)
		next_lcg64(state);
}

/*
 * Based on splitmix64 by Sebastiano Vigna
 * See: http://xorshift.di.unimi.it/splitmix64.c
 */
struct state_smix64 {
	uint64_t s;
};

#define M1 0x9E3779B97F4A7C15ULL
#define M2 0xBF58476D1CE4E5B9ULL
#define M3 0x94D049BB133111EBULL
static inline uint64_t next_smix64(struct state_smix64 *state) {
	uint64_t x = state->s + M1;
	state->s = x;
	uint64_t y = (x ^ (x >> 30)) * M2;
	uint64_t z = (y ^ (y >> 27)) * M3;
	return z ^ (z >> 31);
}

static void init_smix64(struct state_smix64 *state, uint32_t seed)
{
	int i;
	uint64_t x = seed*Z1 + seed*Z2 + Z5 + Z1;
	x = (x != 0) ? x : (Z5 + Z2);
	state->s = x;
	for (i=0; i<11; i++)
		next_smix64(state);
}

/* XORShift64 */
struct state_xorshift64 {
	uint64_t s1;
};

static inline uint64_t next_xorshift64(struct state_xorshift64 *state)
{
	uint64_t x = state->s1;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 43;
	state->s1 = x;
	return x;
}

static void init_xorshift64(struct state_xorshift64 *state, uint32_t seed)
{
	int i;
	uint64_t x = seed*Z1 + seed*Z2 + Z5 + Z1;
	x = (x != 0) ? x : (Z5 + Z2);
	state->s1 = x;
	for (i=0; i<11; i++)
		next_xorshift64(state);
}

/* MWC32 L2 use 2 */
struct state_mwc32_l2_u2 {
	uint32_t s1;
	uint32_t s2;
	uint32_t c;
};

static inline uint64_t next_mwc32_l2_u2(struct state_mwc32_l2_u2 *state)
{
	uint64_t x1 = state->s1*A5+state->c;
	uint64_t x2 = state->s2*A5+H32(x1);
	state->s1 = x1;
	state->s2 = x2;
	state->c = H32(x2);
	uint64_t z1 = Z1 * L32(x1);
	uint64_t z2 = Z2 * L32(x2);
	return z1 + FLIP32(z2);
}

static void init_mwc32_l2_u2(struct state_mwc32_l2_u2 *state, uint32_t seed)
{
	int i;
	uint32_t x = seed*Z1 + seed*Z2 + Z5 + Z3;
	x = (L30(x) != 0) ? x : (Z5 + Z4);
	state->c = L30(x);
	x = x*Z2 + x*Z3 + Z4;
	x = (L31(x) != 0) ? x : (Z5 + Z1);
	state->s1 = L31(x);
	x = x*Z3 + x*Z4 + Z1;
	x = (L31(x) != 0) ? x : (Z5 + Z2);
	state->s2 = L31(x);
	for (i=0; i<11; i++)
		next_mwc32_l2_u2(state);
}

/* MWC32 L2 with multiplication */
struct state_mwc32_l2_m2 {
	uint32_t s1;
	uint32_t s2;
	uint32_t c;
};

static inline uint64_t next_mwc32_l2_m2(struct state_mwc32_l2_m2 *state)
{
	uint64_t x = state->s1*A5+state->c;
	state->s1 = state->s2;
	state->s2 = x;
	state->c = H32(x);
	uint64_t z1 = Z1 * H32(x);
	uint64_t z2 = Z2 * L32(x);
	return z1 + FLIP32(z2);
}

static void init_mwc32_l2_m2(struct state_mwc32_l2_m2 *state, uint32_t seed)
{
	int i;
	uint32_t x = seed*Z1 + seed*Z2 + Z5 + Z3;
	x = (L30(x) != 0) ? x : (Z5 + Z4);
	state->c = L30(x);
	x = x*Z2 + x*Z3 + Z4;
	x = (L31(x) != 0) ? x : (Z5 + Z1);
	state->s1 = L31(x);
	x = x*Z3 + x*Z4 + Z1;
	x = (L31(x) != 0) ? x : (Z5 + Z2);
	state->s2 = L31(x);
	for (i=0; i<11; i++)
		next_mwc32_l2_m2(state);
}

/* MWC32 x2 */
struct state_mwc32_x2 {
	uint64_t s1;
	uint64_t s2;
};

static inline uint64_t next_mwc32_x2(struct state_mwc32_x2 *state)
{
	uint64_t x1 = L32(state->s1)*A1+H32(state->s1);
	uint64_t x2 = L32(state->s2)*A2+H32(state->s2);
	state->s1 = x1;
	state->s2 = x2;
	return x1 + x2;
}

static void init_mwc32_x2(struct state_mwc32_x2 *state, uint32_t seed)
{
	int i;
	uint64_t x = seed*Z1 + seed*Z2 + Z5 + Z1;
	x = (MASK64(x) != 0) ? x : (Z5 + Z2);
	state->s1 = MASK64(x);
	x = x*Z3 + x*Z4 + Z2;
	x = (MASK64(x) != 0) ? x : (Z5 + Z1);
	state->s2 = MASK64(x);
	for (i=0; i<11; i++)
		next_mwc32_x2(state);
}

/* MWC32 X2 M2
 * Passes all of the tests of BigCrush.
 * Four 32-bit ranges tested: 64-33, 48-17, 32-1, and 16-49
 */
struct state_mwc32_x2_m2 {
	uint64_t s1;
	uint64_t s2;
};

static inline uint64_t next_mwc32_x2_m2(struct state_mwc32_x2_m2 *state)
{
	uint64_t x1 = L32(state->s1)*A1+H32(state->s1);
	uint64_t x2 = L32(state->s2)*A2+H32(state->s2);
	state->s1 = x1;
	state->s2 = x2;
	uint64_t y = x1 + x2;
	uint64_t z1 = Z1 * H32(y);
	uint64_t z2 = Z2 * L32(y);
	return z1 + FLIP32(z2);
}

static void init_mwc32_x2_m2(struct state_mwc32_x2_m2 *state, uint32_t seed)
{
	int i;
	uint64_t x = seed*Z1 + seed*Z2 + Z5 + Z1;
	x = (MASK64(x) != 0) ? x : (Z5 + Z2);
	state->s1 = MASK64(x);
	x = x*Z3 + x*Z4 + Z2;
	x = (MASK64(x) != 0) ? x : (Z5 + Z1);
	state->s2 = MASK64(x);
	for (i=0; i<11; i++)
		next_mwc32_x2_m2(state);
}

/* MWC32 AWC64
 * Passes all of the tests of BigCrush.
 * Four 32-bit ranges tested: 64-33, 48-17, 32-1, and 16-49
 */
struct state_mwc32_awc64 {
	uint64_t s1;
	uint64_t s2;
};

static inline uint64_t next_mwc32_awc64(struct state_mwc32_awc64 *state)
{
	uint64_t x1 = L32(state->s1)*A1+H32(state->s1);
	uint64_t x2 = state->s2 + Z6;
	state->s1 = x1;
	state->s2 = x2;
	return x1 + x2;
}

static void init_mwc32_awc64(struct state_mwc32_awc64 *state, uint32_t seed)
{
	int i;
	uint64_t x = seed*Z1 + seed*Z2 + Z5 + Z1;
	x = (MASK64(x) != 0) ? x : (Z5 + Z2);
	state->s1 = MASK64(x);
	state->s2 = x*Z1 + x*Z2 + Z2;
	for (i=0; i<11; i++)
		next_mwc32_awc64(state);
}

/* MWC32 L3 with rotation
 * Passes all of the tests of BigCrush.
 * Three 32-bit ranges tested: 64-33, 48-17, and 32-1 (16-49 not tested)
 */
struct state_mwc32_l3_r {
	uint32_t s1;
	uint32_t s2;
	uint32_t s3;
	uint32_t c;
};

static inline uint64_t next_mwc32_l3_r(struct state_mwc32_l3_r *state)
{
	uint64_t x = state->s1*A6+state->c;
	state->s1 = state->s2;
	state->s2 = state->s3;
	state->s3 = x;
	state->c = H32(x);
	return x + RR64(x,21);
}

static void init_mwc32_l3_r(struct state_mwc32_l3_r *state, uint32_t seed)
{
	int i;
	uint32_t x = seed*Z1 + seed*Z2 + Z5 + Z3;
	x = (L30(x) != 0) ? x : (Z5 + Z4);
	state->c = L30(x);
	x = x*Z2 + x*Z3 + Z4;
	x = (L31(x) != 0) ? x : (Z5 + Z1);
	state->s1 = L31(x);
	x = x*Z3 + x*Z4 + Z1;
	x = (L31(x) != 0) ? x : (Z5 + Z2);
	state->s2 = L31(x);
	x = x*Z4 + x*Z1 + Z2;
	x = (L31(x) != 0) ? x : (Z5 + Z3);
	state->s3 = L31(x);
	for (i=0; i<11; i++)
		next_mwc32_l3_r(state);
}

/* MWC32 L3 M2 */
struct state_mwc32_l3_m2 {
	uint32_t s1;
	uint32_t s2;
	uint32_t s3;
	uint32_t c;
};

static inline uint64_t next_mwc32_l3_m2(struct state_mwc32_l3_m2 *state)
{
	uint64_t x = state->s1*A6+state->c;
	state->s1 = state->s2;
	state->s2 = state->s3;
	state->s3 = x;
	state->c = H32(x);
	uint64_t z1 = Z1 * H32(x^x<<17);
	uint64_t z2 = Z2 * L32(x^x>>13);
	return z1 + FLIP32(z2);
}

static void init_mwc32_l3_m2(struct state_mwc32_l3_m2 *state, uint32_t seed)
{
	int i;
	uint32_t x = seed*Z1 + seed*Z2 + Z5 + Z3;
	x = (L30(x) != 0) ? x : (Z5 + Z4);
	state->c = L30(x);
	x = x*Z2 + x*Z3 + Z4;
	x = (L31(x) != 0) ? x : (Z5 + Z1);
	state->s1 = L31(x);
	x = x*Z3 + x*Z4 + Z1;
	x = (L31(x) != 0) ? x : (Z5 + Z2);
	state->s2 = L31(x);
	x = x*Z4 + x*Z1 + Z2;
	x = (L31(x) != 0) ? x : (Z5 + Z3);
	state->s3 = L31(x);
	for (i=0; i<11; i++)
		next_mwc32_l3_m2(state);
}

/* XORShift128+
 * Based on xorshift+ by Sebastiano Vigna
 * See: http://xorshift.di.unimi.it/#intro
 * Parameters from page 4 of "Further scramblings of Marsaglia's
 * xorshift generators" by Sebastiano Vigna
 * See: http://vigna.di.unimi.it/ftp/papers/xorshiftplus.pdf
 */
struct state_xorshift128p {
	uint64_t s1;
	uint64_t s2;
};

static inline uint64_t next_xorshift128p(struct state_xorshift128p *state)
{
	/* xorshift128+ (A, B, C) = (23, 17, 26) */
	uint64_t s1 = state->s1;
	uint64_t s2 = state->s2;
	s1 ^= s1 << 23;
	s1 ^= s1 >> 17;
	s1 ^= s2 ^ (s2 >> 26);
	state->s1 = s2;
	state->s2 = s1;
	return s1 + s2; 
}

static void init_xorshift128p(struct state_xorshift128p *state, uint32_t seed)
{
	int i;
	uint64_t x = seed*Z1 + seed*Z2 + Z5 + Z3;
	x = (x != 0) ? x : (Z5 + Z4);
	state->s1 = x;
	x = x*Z2 + x*Z3 + Z4;
	x = (x != 0) ? x : (Z5 + Z1);
	state->s2 = x;
	for (i=0; i<11; i++)
		next_xorshift128p(state);
}

/* MWC32 L6 M2 */
struct state_mwc32_l6_m2 {
	uint8_t n;
	uint32_t c;
	uint32_t s[6];
};

const uint8_t next6[6] = {1, 2, 3, 4, 5, 0};
static inline uint64_t next_mwc32_l6_m2(struct state_mwc32_l6_m2 *state)
{
	uint8_t n = state->n;
	uint64_t x = state->s[n]*A7+state->c;
	state->s[n] = x;
	state->c = H32(x);
	state->n = next6[n];
	uint64_t z1 = Z1 * H32(x);
	uint64_t z2 = Z2 * L32(x);
	return z1 + FLIP32(z2);
}

static void init_mwc32_l6_m2(struct state_mwc32_l6_m2 *state, uint32_t seed)
{
	int i;
	uint32_t x = seed*Z1 + seed*Z2 + Z3;
	state->n = x % 7;
	x = x*Z1 + x*Z2 + Z5;
	x = (L30(x) != 0) ? x : (Z5 + Z1);
	state->c = L30(x);
	for (i=0; i<6; i++) {
		x = x*Z4 + x*Z3 + Z2;
		x = (L31(x) != 0) ? x : (Z5 + Z1);
		state->s[i] = L31(x);
	}
	for (i=0; i<11; i++)
		next_mwc32_l6_m2(state);
}

/* MWC32 X4 */
struct state_mwc32_x4 {
	uint64_t s1;
	uint64_t s2;
	uint64_t s3;
	uint64_t s4;
};

static inline uint64_t next_mwc32_x4(struct state_mwc32_x4 *state)
{
	uint64_t x1 = L32(state->s1)*A1+H32(state->s1);
	uint64_t x2 = L32(state->s2)*A2+H32(state->s2);
	uint64_t x3 = L32(state->s3)*A3+H32(state->s3);
	uint64_t x4 = L32(state->s4)*A4+H32(state->s4);
	state->s1 = x1;
	state->s2 = x2;
	state->s3 = x3;
	state->s4 = x4;
	uint64_t a1 = Z1 * L32(x1);
	uint64_t a2 = Z2 * L32(x2);
	uint64_t a3 = Z3 * L32(x3);
	uint64_t a4 = Z4 * L32(x4);
	return a1 + FLIP32(a2) + a3 + FLIP32(a4);
}

static void init_mwc32_x4(struct state_mwc32_x4 *state, uint32_t seed)
{
	int i;
	uint64_t x = seed*Z1 + seed*Z2 + Z5 + Z3;
	x = (MASK64(x) != 0) ? x : (Z5 + Z4);
	state->s1 = MASK64(x);
	x = x*Z2 + x*Z3 + Z4;
	x = (MASK64(x) != 0) ? x : (Z5 + Z1);
	state->s2 = MASK64(x);
	x = x*Z3 + x*Z4 + Z1;
	x = (MASK64(x) != 0) ? x : (Z5 + Z2);
	state->s3 = MASK64(x);
	x = x*Z4 + x*Z1 + Z2;
	x = (MASK64(x) != 0) ? x : (Z5 + Z3);
	state->s4 = MASK64(x);
	for (i=0; i<11; i++)
		next_mwc32_x4(state);
}

/* MWC32 L7 M2
 * Passes all of the tests of BigCrush.
 * Four 32-bit ranges tested: 64-33, 48-17, 32-1, and 16-49
 */
struct state_mwc32_l7_m2 {
	uint8_t n;
	uint32_t c;
	uint32_t s[7];
};

const uint8_t next7[7] = {1, 2, 3, 4, 5, 6, 0};
static inline uint64_t next_mwc32_l7_m2(struct state_mwc32_l7_m2 *state)
{
	uint8_t n = state->n;
	uint64_t x = state->s[n]*A8+state->c;
	state->s[n] = x;
	state->c = H32(x);
	state->n = next7[n];
	uint64_t z1 = Z1 * H32(x);
	uint64_t z2 = Z2 * L32(x);
	return z1 + FLIP32(z2);
}

static void init_mwc32_l7_m2(struct state_mwc32_l7_m2 *state, uint32_t seed)
{
	int i;
	uint32_t x = seed*Z1 + seed*Z2 + Z3;
	state->n = x % 7;
	x = x*Z1 + x*Z2 + Z5;
	x = (L30(x) != 0) ? x : (Z5 + Z1);
	state->c = L30(x);
	for (i=0; i<7; i++) {
		x = x*Z4 + x*Z3 + Z2;
		x = (L31(x) != 0) ? x : (Z5 + Z1);
		state->s[i] = L31(x);
	}
	for (i=0; i<11; i++)
		next_mwc32_l7_m2(state);
}

/* MWC60 L4 M2 */
struct state_mwc60_l4_m2 {
	uint64_t s1;
	uint64_t s2;
	uint64_t s3;
	uint64_t s4;
	uint64_t c;
};

static inline uint64_t next_mwc60_l4_m2(struct state_mwc60_l4_m2 *state)
{
	uint64_t s = state->s1;
	uint64_t n1 = L30(s);
	uint64_t n2 = SR30(s);
	uint64_t x1 = n1*AA1 + state->c;
	uint64_t x2 = n1*AA2 + n2*AA1 + SR30(x1);
	uint64_t x3 = n2*AA2 + SR30(x2);
	uint64_t x4 = SL30(L30(x2)) | L30(x1);
	state->s1 = state->s2;
	state->s2 = state->s3;
	state->s3 = state->s4;
	state->s4 = x4;
	state->c = x3;
	uint64_t z1 = Z1 * state->s2;
	uint64_t z2 = Z2 * x4;
	return z1 + FLIP32(z2);
}

static void init_mwc60_l4_m2(struct state_mwc60_l4_m2 *state, uint32_t seed)
{
	int i;
	uint64_t x = seed*Z1 + seed*Z2 + Z5 + Z3;
	x = (L59(x) != 0) ? x : (Z5 + Z4);
	state->c = L59(x);
	x = x*Z2 + x*Z3 + Z4;
	x = (L60(x) != 0) ? x : (Z5 + Z1);
	state->s1 = L60(x);
	x = x*Z3 + x*Z4 + Z1;
	x = (L60(x) != 0) ? x : (Z5 + Z2);
	state->s2 = L60(x);
	x = x*Z4 + x*Z1 + Z2;
	x = (L60(x) != 0) ? x : (Z5 + Z3);
	state->s3 = L60(x);
	x = x*Z1 + x*Z2 + Z3;
	x = (L60(x) != 0) ? x : (Z5 + Z4);
	state->s4 = L60(x);
	for (i=0; i<11; i++)
		next_mwc60_l4_m2(state);
}


#define NUM_TRIALS 1000000000

#define TEST_GEN(type,name,period) (   \
    {                                  \
	struct state_##type type;          \
    init_##type(&type, 12345);         \
	total = 0;                         \
	start = clock();                   \
	for (i=0; i<NUM_TRIALS; i++) {     \
		total += next_##type(&type);   \
	}                                  \
	stop = clock();                    \
	grand_total += total;              \
	dt2 = stop-start;                  \
	printf("%25s  2^%-3d  %5.3f sec\n",name, period,\
	       (double)(dt2-dt1)/((double)CLOCKS_PER_SEC));\
    }                                  \
)

/* gcc -O1 -o examples_64bit -lm examples_64bit.c */
int main (void)
{
	int start, stop, dt1, dt2;
	unsigned i;
	uint64_t total = 0;
	uint64_t grand_total = 0;

	start = clock();
	for (i=0; i<NUM_TRIALS; i++) {
		total += i;
	}
	stop = clock();
	grand_total += total;
	dt1 = stop-start;
	printf("time to add index: %5.3f\n\n",(double)dt1/((double)CLOCKS_PER_SEC));

	TEST_GEN(mwc32_r, "MWC32 R", 63);
	TEST_GEN(mwc32_m2_32, "MWC32 *2 (32)", 63);
	TEST_GEN(mwc32_m2_64, "MWC32 *2 (64)", 63);
	TEST_GEN(lcg64, "LCG64", 64);
	TEST_GEN(smix64, "SMIX64", 64);
	TEST_GEN(xorshift64, "XORShift64", 64);
	TEST_GEN(mwc32_l2_u2, "MWC32 L2 U2", 94);
	TEST_GEN(mwc32_l2_m2, "MWC32 L2 *2", 95);
	TEST_GEN(mwc32_x2, "MWC32 X2", 126);
	TEST_GEN(mwc32_x2_m2, "MWC32 X2 *2", 126);
	TEST_GEN(mwc32_awc64, "MWC32 AWC64", 127);
	TEST_GEN(mwc32_l3_r, "MWC32 L3 R", 127);
	TEST_GEN(mwc32_l3_m2, "MWC32 L3 *2", 127);
	TEST_GEN(xorshift128p, "XORShift128+", 128);
	TEST_GEN(mwc32_l6_m2, "MWC32 L6 *2", 223);
	TEST_GEN(mwc32_x4, "MWC32 X4", 252);
	TEST_GEN(mwc32_l7_m2, "MWC32 L7 *2", 255);
	TEST_GEN(mwc60_l4_m2, "MWC60 L4 *2", 299);

	printf("\ntotal = %llu (So compiler won't optimize away the loops)\n",
	       grand_total);
	return 0;
}
