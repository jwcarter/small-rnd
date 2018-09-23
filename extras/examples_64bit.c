/*
 * ISC License
 *
 * Copyright (c) 2016 James William Carter
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <time.h> // For timing loop

 // time_test() in different file to minimize optimization
#include "timing_test.h"


#define A1A 4294095429ULL /* MWC L1: 3 * 13 * 110105011 */
#define A1B 4293977883ULL /* MWC L1: 3 * 13 * 110101997 */
#define A1C 4293477123ULL /* MWC L1: 3 * 13 * 110089157 */
#define A1D 4268670393ULL /* MWC L1: 3 * 13 * 109453087 */
#define A2A 4293538899ULL /* MWC L2: 3 * 13 * 110090741 */
#define A2B 4291750983ULL /* MWC L2: 3 * 13 * 110044897 */
#define A3A 4293666429ULL /* MWC L3: 3 * 13 * 110094011 */
#define A3B 4293542019ULL /* MWC L3: 3 * 13 * 110090821 */
#define A6A 4294748679ULL /* MWC L6: 3 * 13 * 110121761 */
#define A7A 4294258449ULL /* MWC L7: 3 * 13 * 110109191 */
#define A8A 4293082443ULL /* MWC L8: 3 * 13 * 110079037 */

/* LCG 64: */
#define A10 13624260627007768477ULL /* LCG64: 2621890429 * 5196350113 */
#define C10  8407169572674124741ULL /* LCG64: 13913 * 68683 * 77773 * 113123 */

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

/* Unused Constants */
#define U01 17533487109243764213ULL /* LCG64: 1316877131 * 13314444223 */
#define U02 13174768461294744229ULL /* LCG64: 1317768131 * 9997789559 */
#define U03 13688810297206677709ULL /* LCG64: 2349389479 * 5826539371 */
#define U04  8154731968748247349ULL /* LCG64: 368773 * 22113148112113 */
#define U05  4631772754238921141ULL /* LCG64: 13913 * 62323 * 68683 * 77773 */
#define U06  8265071960137744637ULL /* LCG64: 13913 * 68683 * 77773 * 111211 */
#define U07           2611384021ULL /* LCG32: 33577 * 77773 */
#define U08           2322805621ULL /* LCG32: 33773 * 68777 */
#define U09           4294963023ULL /* MWC L1: 3 * 13 * 43 * 2561099 */
#define U10           4294937829ULL /* MWC L4: 3 * 7 * 13 * 15732373 */
#define U11           4294578093ULL /* MWC L4: 3 * 13 * 110117387 */
#define U12           4294921683ULL /* MWC L7: 3 * 13 * 89 * 1237373 */
#define U13           4294930299ULL /* MWC L8: 3 * 11 * 130149403 */

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

static inline uint64_t next_mwc32_r(void *s)
{
	struct state_mwc32_r *state = s;
	uint64_t x = L32(state->s1)*A1A+H32(state->s1);
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

static inline uint64_t next_mwc32_m2_32(void *s)
{
	struct state_mwc32_m2_32 *state = s;
	uint64_t x = state->s1*A1A+state->c;
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

static inline uint64_t next_mwc32_m2_64(void *s)
{
	struct state_mwc32_m2_64 *state = s;
	uint64_t x = L32(state->s1)*A1A+H32(state->s1);
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

static inline uint64_t next_lcg64(void *s)
{
	struct state_lcg64 *state = s;
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
static inline uint64_t next_smix64(void *s) {
	struct state_smix64 *state = s;
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

static inline uint64_t next_xorshift64(void *s)
{
	struct state_xorshift64 *state = s;
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

static inline uint64_t next_mwc32_l2_u2(void *s)
{
	struct state_mwc32_l2_u2 *state = s;
	uint64_t x1 = state->s1*A2A+state->c;
	uint64_t x2 = state->s2*A2A+H32(x1);
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

static inline uint64_t next_mwc32_l2_m2(void *s)
{
	struct state_mwc32_l2_m2 *state = s;
	uint64_t x = state->s1*A2A+state->c;
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

static inline uint64_t next_mwc32_x2(void *s)
{
	struct state_mwc32_x2 *state = s;
	uint64_t x1 = L32(state->s1)*A1A+H32(state->s1);
	uint64_t x2 = L32(state->s2)*A1B+H32(state->s2);
	state->s1 = x1;
	state->s2 = x2;
	return x1 + FLIP32(x2);
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

static inline uint64_t next_mwc32_x2_m2(void *s)
{
	struct state_mwc32_x2_m2 *state = s;
	uint64_t x1 = L32(state->s1)*A1A+H32(state->s1);
	uint64_t x2 = L32(state->s2)*A1B+H32(state->s2);
	state->s1 = x1;
	state->s2 = x2;
	uint64_t y = x1 + FLIP32(x2);
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

static inline uint64_t next_mwc32_awc64(void *s)
{
	struct state_mwc32_awc64 *state = s;
	uint64_t x1 = L32(state->s1)*A1A+H32(state->s1);
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

static inline uint64_t next_mwc32_l3_r(void *s)
{
	struct state_mwc32_l3_r *state = s;
	uint64_t x = state->s1*A3A+state->c;
	state->s1 = state->s2;
	state->s2 = state->s3;
	state->s3 = x;
	state->c = H32(x);
	return x + RR64(x,25);
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

static inline uint64_t next_mwc32_l3_m2(void *s)
{
	struct state_mwc32_l3_m2 *state = s;
	uint64_t x = state->s1*A3A+state->c;
	state->s1 = state->s2;
	state->s2 = state->s3;
	state->s3 = x;
	state->c = H32(x);
	uint64_t z1 = Z1 * H32(x);
	uint64_t z2 = Z2 * L32(x);
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

static inline uint64_t next_xorshift128p(void *s)
{
	struct state_xorshift128p *state = s;
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

/* xoroshiro128plus
 * Based on xoroshiro128plus by David Blackman and Sebastiano Vigna
 * See: http://xoroshiro.di.unimi.it/xoroshiro128plus.c
 */
struct state_xoroshiro {
        uint64_t s1;
        uint64_t s2;
};

static inline uint64_t next_xoroshiro(void *s)
{
	struct state_xoroshiro *state = s;
	const uint64_t s1 = state->s1;
	uint64_t s2 = state->s2;
	const uint64_t x = s1 + s2;
	s2 ^= s1;
	state->s1 = RL64(s1, 55) ^ s2 ^ (s2 << 14);
	state->s2 = RL64(s2, 36);
	return x;
}

static void init_xoroshiro(struct state_xoroshiro *state, uint32_t seed)
{
	int i;
	uint64_t x = seed*Z1 + seed*Z2 + Z5 + Z3;
	x = (x != 0) ? x : (Z5 + Z4);
	state->s1 = x;
	x = x*Z2 + x*Z3 + Z4;
	x = (x != 0) ? x : (Z5 + Z1);
	state->s2 = x;
	for (i=0; i<11; i++)
		next_xoroshiro(state);
}

/* MWC32 L2 X2 with multiplication */
struct state_mwc32_l2_x2_m2 {
	uint32_t s1a;
	uint32_t s1b;
	uint32_t c1;
	uint32_t s2a;
	uint32_t s2b;
	uint32_t c2;
};

static inline uint64_t next_mwc32_l2_x2_m2(void *s)
{
	struct state_mwc32_l2_x2_m2 *state = s;
	uint64_t x1 = state->s1a*A2A+state->c1;
	state->s1a = state->s1b;
	state->s1b = x1;
	state->c1 = H32(x1);
	uint64_t x2 = state->s2a*A2B+state->c2;
	state->s2a = state->s2b;
	state->s2b = x2;
	state->c2 = H32(x2);
	uint64_t z1 = Z1 * H32(x1);
	uint64_t z2 = Z2 * L32(x2);
	return z1 + FLIP32(z2);
}

static void init_mwc32_l2_x2_m2(struct state_mwc32_l2_x2_m2 *state, uint32_t seed)
{
	int i;
	uint32_t x = seed*Z1 + seed*Z2 + Z5 + Z3;
	x = (L30(x) != 0) ? x : (Z5 + Z4);
	state->c1 = L30(x);
	x = x*Z2 + x*Z3 + Z1;
	x = (L30(x) != 0) ? x : (Z5 + Z4);
	state->c2 = L30(x);
	x = x*Z3 + x*Z4 + Z2;
	x = (L31(x) != 0) ? x : (Z5 + Z1);
	state->s1a = L31(x);
	x = x*Z4 + x*Z1 + Z3;
	x = (L31(x) != 0) ? x : (Z5 + Z2);
	state->s1b = L31(x);
	x = x*Z1 + x*Z2 + Z4;
	x = (L31(x) != 0) ? x : (Z5 + Z3);
	state->s2a = L31(x);
	x = x*Z2 + x*Z3 + Z1;
	x = (L31(x) != 0) ? x : (Z5 + Z4);
	state->s2b = L31(x);
	for (i=0; i<11; i++)
		next_mwc32_l2_x2_m2(state);
}

/* MWC32 L6 M2 */
struct state_mwc32_l6_m2 {
	uint8_t n;
	uint32_t c;
	uint32_t s[6];
};

const uint8_t next6[6] = {1, 2, 3, 4, 5, 0};
static inline uint64_t next_mwc32_l6_m2(void *s)
{
	struct state_mwc32_l6_m2 *state = s;
	uint8_t n = state->n;
	uint64_t x = state->s[n]*A6A+state->c;
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

static inline uint64_t next_mwc32_x4(void *s)
{
	struct state_mwc32_x4 *state = s;
	uint64_t x1 = L32(state->s1)*A1A+H32(state->s1);
	uint64_t x2 = L32(state->s2)*A1B+H32(state->s2);
	uint64_t x3 = L32(state->s3)*A1C+H32(state->s3);
	uint64_t x4 = L32(state->s4)*A1D+H32(state->s4);
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

/* MWC32 L3 X2 M2 */
struct state_mwc32_l3_x2_m2 {
	uint32_t s1a;
	uint32_t s1b;
	uint32_t s1c;
	uint32_t c1;
	uint32_t s2a;
	uint32_t s2b;
	uint32_t s2c;
	uint32_t c2;
};

static inline uint64_t next_mwc32_l3_x2_m2(void *s)
{
	struct state_mwc32_l3_x2_m2 *state = s;
	uint64_t x1 = state->s1a*A3A+state->c1;
	state->s1a = state->s1b;
	state->s1b = state->s1c;
	state->s1c = x1;
	state->c1 = H32(x1);
	uint64_t x2 = state->s2a*A3B+state->c2;
	state->s2a = state->s2b;
	state->s2b = state->s2c;
	state->s2c = x2;
	state->c2 = H32(x2);
	uint64_t z1 = Z1 * H32(x1);
	uint64_t z2 = Z2 * L32(x2);
	return z1 + FLIP32(z2);
}

static void init_mwc32_l3_x2_m2(struct state_mwc32_l3_x2_m2 *state, uint32_t seed)
{
	int i;
	uint32_t x = seed*Z1 + seed*Z2 + Z5 + Z3;
	x = (L30(x) != 0) ? x : (Z5 + Z4);
	state->c1 = L30(x);
	x = x*Z1 + x*Z2 + Z3;
	x = (L30(x) != 0) ? x : (Z5 + Z4);
	state->c2 = L30(x);
	x = x*Z2 + x*Z3 + Z4;
	x = (L31(x) != 0) ? x : (Z5 + Z1);
	state->s1a = L31(x);
	x = x*Z3 + x*Z4 + Z1;
	x = (L31(x) != 0) ? x : (Z5 + Z2);
	state->s1b = L31(x);
	x = x*Z4 + x*Z1 + Z2;
	x = (L31(x) != 0) ? x : (Z5 + Z3);
	state->s1c = L31(x);
	x = x*Z2 + x*Z3 + Z4;
	x = (L31(x) != 0) ? x : (Z5 + Z1);
	state->s2a = L31(x);
	x = x*Z3 + x*Z4 + Z1;
	x = (L31(x) != 0) ? x : (Z5 + Z2);
	state->s2b = L31(x);
	x = x*Z4 + x*Z1 + Z2;
	x = (L31(x) != 0) ? x : (Z5 + Z3);
	state->s2c = L31(x);
	for (i=0; i<11; i++)
		next_mwc32_l3_x2_m2(state);
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
static inline uint64_t next_mwc32_l7_m2(void *s)
{
	struct state_mwc32_l7_m2 *state = s;
	uint8_t n = state->n;
	uint64_t x = state->s[n]*A7A+state->c;
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

/* MWC32 L8 M2
 */
struct state_mwc32_l8_m2 {
	uint8_t n;
	uint32_t c;
	uint32_t s[8];
};

static inline uint64_t next_mwc32_l8_m2(void *s)
{
	struct state_mwc32_l8_m2 *state = s;
	uint8_t n = 0x7&state->n;
	uint64_t x = state->s[n]*A8A+state->c;
	state->s[n] = x;
	state->c = H32(x);
	state->n++;
	uint64_t z1 = Z1 * H32(x);
	uint64_t z2 = Z2 * L32(x);
	return z1 + FLIP32(z2);
}

static void init_mwc32_l8_m2(struct state_mwc32_l8_m2 *state, uint32_t seed)
{
	int i;
	uint32_t x = seed*Z1 + seed*Z2 + Z3;
	state->n = 0x7&x;
	x = x*Z1 + x*Z2 + Z5;
	x = (L30(x) != 0) ? x : (Z5 + Z1);
	state->c = L30(x);
	for (i=0; i<8; i++) {
		x = x*Z4 + x*Z3 + Z2;
		x = (L31(x) != 0) ? x : (Z5 + Z1);
		state->s[i] = L31(x);
	}
	for (i=0; i<11; i++)
		next_mwc32_l8_m2(state);
}

/* MWC60 L4 M2 */
struct state_mwc60_l4_m2 {
	uint64_t s1;
	uint64_t s2;
	uint64_t s3;
	uint64_t s4;
	uint64_t c;
};

static inline uint64_t next_mwc60_l4_m2(void *s)
{
	struct state_mwc60_l4_m2 *state = s;
	uint64_t s1 = state->s1;
	uint64_t n1 = L30(s1);
	uint64_t n2 = SR30(s1);
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

/* DUMMY */
struct state_dummy {
	uint64_t s;
};

static inline uint64_t next_dummy(void *s)
{
	struct state_dummy *state = s;
	return state->s;
}

static void init_dummy(struct state_dummy *state, uint32_t seed)
{
	state->s = seed;
}


#define NUM_TRIALS 1000000000

#define TEST_GEN(type,name,period) (   \
{                                      \
	struct state_##type type;          \
    init_##type(&type, 12345);         \
	dt = time_test(&type, NUM_TRIALS, &total, &next_##type);  \
	grand_total += total;              \
    if (basetime == 0) {               \
		printf("%25s  2^%-3d  %5.3f sec\n",name, period,      \
	       (double)dt/((double)CLOCKS_PER_SEC));              \
	} else {                           \
		printf("%25s  2^%-3d  %5.3f sec %5.3f\n",name, period, \
	       (double)(dt-basetime)/((double)CLOCKS_PER_SEC),    \
           (double)dt/(double)basetime);					  \
    }                                  \
}                                      \
)

/* gcc -O1 -fno-move-loop-invariants -fno-unroll-loops -o examples_64bit -lm timing_test.c examples_64bit.c */

/* gcc -O1 -o examples_64bit -lm timing_test.c examples_64bit.c */
int main (void)
{
	int dt, basetime;
	uint64_t total = 0;
	uint64_t grand_total = 0;

	printf("\n");

	basetime = 0;
	TEST_GEN(dummy, "DUMMY", 0);
	basetime = dt;

	printf("\n");
	printf("d = Time for  DUMMY  to generate %llu numbers\n",NUM_TRIALS);
	printf("t = Time for rnd_gen to generate %llu numbers\n",NUM_TRIALS);
	printf("\n");
	printf("%25s %6s %5s %9s\n", "Test", "Period", "t-d", "t/d");


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
	TEST_GEN(xoroshiro, "xoroshiro128plus", 128);
	TEST_GEN(mwc32_l2_x2_m2, "MWC32 L2 X2 *2", 190);
	TEST_GEN(mwc32_l6_m2, "MWC32 L6 *2", 223);
	TEST_GEN(mwc32_x4, "MWC32 X4", 252);
	TEST_GEN(mwc32_l3_x2_m2, "MWC32 L3 X2 *2", 254);
	TEST_GEN(mwc32_l7_m2, "MWC32 L7 *2", 255);
	TEST_GEN(mwc32_l8_m2, "MWC32 L8 *2", 287);
	TEST_GEN(mwc60_l4_m2, "MWC60 L4 *2", 299);

	printf("\ntotal = %llu (So compiler won't optimize away the loops)\n",
	       grand_total);
	return 0;
}
