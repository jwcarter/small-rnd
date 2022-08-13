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
#define Z1 4078645709ULL /* LCG32: 77999 * 52291 */
#define Z2 3580663381ULL /* LCG32: 68111 * 52571 */
#define Z3 3571494541ULL /* LCG32: 91019 * 39239 */
#define Z4 3753453877ULL /* LCG32: 13999 * 268123 */
#define Z5 2611384021ULL /* LCG32: 33577 * 77773 */
#define Z6 2322805621ULL /* LCG32: 33773 * 68777 */
#define Z7 18277323205306182053ULL /* LCG64: 26777 * 65777 * 78787 * 131711 */
#define Z8  9390737881634721349ULL /* LCG64: 13613 * 68683 * 77377 * 129803 */
#define Z9 17533487109243764213ULL /* LCG64: 1316877131 * 13314444223 */

/* Unused Constants */
#define U01 13174768461294744229ULL /* LCG64: 1317768131 * 9997789559 */
#define U02 13688810297206677709ULL /* LCG64: 2349389479 * 5826539371 */
#define U03  8154731968748247349ULL /* LCG64: 368773 * 22113148112113 */
#define U04  4631772754238921141ULL /* LCG64: 13913 * 62323 * 68683 * 77773 */
#define U05  8265071960137744637ULL /* LCG64: 13913 * 68683 * 77773 * 111211 */
#define U06           4294963023ULL /* MWC L1: 3 * 13 * 43 * 2561099 */
#define U07           4294937829ULL /* MWC L4: 3 * 7 * 13 * 15732373 */
#define U08           4294578093ULL /* MWC L4: 3 * 13 * 110117387 */
#define U09           4294921683ULL /* MWC L7: 3 * 13 * 89 * 1237373 */
#define U10           4294930299ULL /* MWC L8: 3 * 11 * 130149403 */

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


static void gen_init_state(uint32_t s[], unsigned num, uint32_t seed)
{
	int i = 0;
	uint32_t x = Z4;
	while (i < num) {
		x = ((x + seed) * Z3) + Z4;
		if (L31(x) != 0) {
			s[i] = x;
			i++;
		}
	}
}


/* MWC32
 * bad_mwc32
 *
 * test_birthday results:
 * Bits used: 16  t: 4
 *   Left 43          : p-value > 1-1e-15
 *   Left 57, 36, 33  : p-value > 1-1e-06
 *
 * rnd_bit_tests results:
 * 2^14 trials
 *   Bits 52-20, 53-21, 54-22, 55-23, 56-24, 57-25,
 *        58-26, 59-27, 60-28, 61-29, 62-30, 63-31   : p-value > 1-1e-15
 *   Bits 51-19                                      : p-value < 1e-15
 *   Bits 50-18                                      : p-value < 1e-12
 *   Test aborts due to the number of failures
 *
 * rnd_dice_tests results:
 * 2^27 trials
 *   Horizontal Bits 60-64                           : p-value < 1e-15
 *   Horizontal Bits 52-56, 56-60                    : p-value < 1e-12
 * 2^28 trials
 *   Horizontal Bits 52-56, 56-60                    : p-value < 1e-15
 * 2^30 trials
 *   Vertical Bits 54                                : p-value < 1e-12
 *   Vertical BIts 55-63                             : p-value < 1e-06
 * 2^31 trials
 *   Vertical Bits 54-55, 58-59, 61-63               : p-value < 1e-15
 *   Vertical Bits 56, 60                            : p-value < 1e-12
 *   Vertical Bits 52, 53, 57                        : p-value < 1e-09
 *   Test aborts due to the number of failures
 *
 * test_bit_prev results:
 *   47-15, 48-16, 49-17, 50-18, 51-19, 52-20,
 *   53-21, 54-22, 55-23, 56-24, 57-25, 58-26,
 *   59-27, 60-28, 61-29, 62-30, 63-31          : p-value < 1e-15
 */
struct state_mwc32 {
	uint64_t s1;
};

static inline uint64_t next_mwc32(void *s)
{
	struct state_mwc32 *state = s;
	uint64_t x = L32(state->s1)*A1A+H32(state->s1);
	state->s1 = x;
	return x + RR64(x,25);
}

static void init_mwc32(struct state_mwc32 *state, uint32_t seed)
{
	int i;
	uint32_t z[3];
	gen_init_state(z, 3, seed);
	state->s1 = (uint64_t)L31(z[2]) << 32 | (uint64_t)L31(z[1]);
	for (i=0; i<13; i++)
		next_mwc32(state);
}
		
/* MWC32 with rotation
 * Regularly, but not always, fails BigCrush #15 BirthdaySpacings, t = 4
 * for the 32-bit range 48-17
 * Passes all of the tests for BigCrush for the 32-bit ranges:
 * 64-33, 32-1, and 16-49
 *
 * test_birthday results:
 * Bits used: 64  t: 1
 *   Left 63-0                      : p-value > 1-1e-15
 * Bits used: 32  t: 2
 *   Left 51, 45, 39-38, 31, 11, 8  : p-value > 1-1e-15
 *   Left 23, 7, 1                  : p-value > 1-1e-12
 *   Left 9                         : p-value > 1-1e-06
 * Bits used: 16  t: 4
 *   Left 46                        : p-value > 1-1e-15
 *   Left 63                        : p-value > 1-1e-12
 *   Left 19, 0                     : p-value > 1-1e-09
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
	uint32_t z[3];
	gen_init_state(z, 3, seed);
	state->s1 = (uint64_t)L31(z[2]) << 32 | (uint64_t)L31(z[1]);
	for (i=0; i<13; i++)
		next_mwc32_r(state);
}
				
/* MWC32 M2 - 32-bit state variables
 * Functionally the same as the 64-bit state variable below
 * and has all the same problems.
*/
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
	uint32_t z[3];
	gen_init_state(z, 3, seed);
	state->s1 = L31(z[2]);
	state->c  = L31(z[1]);
	for (i=0; i<13; i++)
		next_mwc32_m2_32(state);
}

/* MWC32 M2 - 64-bit state variables
 * rnd_63
 * Passes all of the tests of BigCrush for the following 32-bit ranges:
 *   03-36, 07-40, 11-44, 15-48, 19-52, 23-56, 31-00, 35-04, 39-08, 47-16,
 *   51-20, 59-28, 63-32
 * The following ranges fail the listed tests:
 *   27-60: 14  BirthdaySpacings, t = 3
 *   43-12: 13  BirthdaySpacings, t = 2
 *          15  BirthdaySpacings, t = 4
 *   55-24: 13  BirthdaySpacings, t = 2
 *
 * test_birthday results:
 * Bits used: 32  t: 2
 *   Left 57-55, 52, 48, 44-43, 33-32, 22, 17  : p-value > 1-1e-15
 *   Left 63-62, 42                            : p-value > 1-1e-09
 *   Left 54                                   : p-value > 1-1e-06
 * Bits used: 16  t: 4
 *   Left 38, 29, 19, 18                       : p-value > 1-1e-15
 *   Left 39                                   : p-value > 1-1e-12
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
	uint32_t z[3];
	gen_init_state(z, 3, seed);
	state->s1 = (uint64_t)L31(z[2]) << 32 | (uint64_t)L31(z[1]);
	for (i=0; i<13; i++)
		next_mwc32_m2_64(state);
}

/* LCG 64
 * Criteria for LCGs:
 *  2^64 and C10 are relatively prime
 *  A10 - 1 is divisible by 2 (the prime factor of 2^64)
 *  A10 - 1 is divisible by 4 (since 2^64 is divisible by 4)
 *  A10 % 8 = 5 (A10 not divisible by 8)
 *
 * test_birthday results:
 * Bits used: 64:  t: 1
 *   Left 26, 17-9, 7-5  : p-value > 1-1e-15
 *   Left 4              : p-value > 1-1e-12
 *   Left 8              : p-value > 1-1e-09
 *   Left 24-18          : p-value < 1e-15
 * Bits used: 32  t: 2
 *   Left 63-0           : p-value > 1-1e-15
 * Bits used: 16  t: 4
 *   Left 63-0           : p-value > 1-1e-15
 *
 * rnd_bit_tests results:
 * 2^14 trials
 *   Bit-to-Prev-Bit 03-03                       : p-value > 1-1e-15
 *   Bit-to-Prev-Bit 00-00, 02-02, 04-04, 05-05  : p-value < 1e-15
 * 2^15 trials
 *   Bit-to-Prev-Bit 06-06                       : p-value < 1e-06
 * 2^16 trials
 *   Bit-to-Prev-Bit 06-06                       : p-value < 1e-15
 *   Bit-to-Prev-Bit 08-08                       : p-value < 1e-09
 * 2^17 trials
 *   Bit-to-Prev-Bit 08-08                       : p-value < 1e-15
 *   Bit-to-Prev-Bit 07-07                       : p-value < 1e-06
 *   Test aborts due to the number of failures
 *
 * rnd_dice_tests results:
 * 2^14 trials
 *   Vertical Bits 00-09  : p-value < 1e-15
 *   Vertical Bits 13     : p-value > 1-1e-09
 *   Test aborts due to the number of failures
 *
 * test_bit_prev results:
 *   0-0, 2-2, 3-3, 4-4, 5-5, 6-6, 7-7, 8-8, 9-9  : p-value < 1e-15
 *   10-10                                        : p-value < 1e-09
 *
 * test_bit_dist results:
 *   Bits 00-27 have bad distributions
 */
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
	uint32_t z[3];
	gen_init_state(z, 3, seed);
	state->s = (uint64_t)z[2] << 32 | (uint64_t)z[1];
	for (i=0; i<13; i++)
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
	uint32_t z[3];
	gen_init_state(z, 3, seed);
	state->s = (uint64_t)z[2] << 32 | (uint64_t)z[1];
	for (i=0; i<13; i++)
		next_smix64(state);
}

/* XORShift64
 * Values come from "Xorshift RNGs" by George Marsaglia in the Journal
 * of Statistical Software, 2003.
 *
 * test_birthday results:
 * Bits used: 32  t: 2
 *   Left 59-9, 3-1          : p-value > 1-1e-15
 *   Left 60                 : p-value > 1-1e-09
 * Bits used: 16  t: 4
 *   Left 61-55, 47-18, 13-1 : p-value > 1-1e-15
 *   Left 14                 : p-value > 1-1e-06
 *
 * rnd_bit_tests results:
 * 2^30 trials: no problems
 *
 * rnd_dice_tests results:
 * 2^33 trials: no problems
 *
 * test_bit_dist results: no problems
 */
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
	uint32_t z[3];
	gen_init_state(z, 3, seed);
	state->s1 = (uint64_t)z[2] << 32 | (uint64_t)z[1];
	for (i=0; i<13; i++)
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
	uint32_t z[4];
	gen_init_state(z, 4, seed);
	state->s1 = L31(z[3]);
	state->s2 = L31(z[2]);
	state->c  = L31(z[1]);
	for (i=0; i<13; i++)
		next_mwc32_l2_u2(state);
}

/* MWC32 L2
 *
 * Passes all of the tests of BigCrush for the following 32-bit ranges:
 *   31-00, 35-04, 39-08, 43-12, 47-16, 48-17, 49-18, 50-19
 * The following ranges fail the listed tests:
 *   51-20: 51  SampleProd, t = 16
 *          55  SampleCorr, k = 1
 *          56  SampleCorr, k = 2
 *          65  SumCollector
 *   52-21: 50  SampleProd, t = 8
 *          51  SampleProd, t = 16
 *          52  SampleProd, t = 24
 *          53  SampleMean, r = 0
 *          55  SampleCorr, k = 1
 *          56  SampleCorr, k = 2
 *          59  WeightDistrib, r = 0
 *          65  SumCollector
 *         101  Run of bits, r = 0
 *
 * Bits 63-51 are biased.
*/
struct state_mwc32_l2 {
	uint32_t s1;
	uint32_t s2;
	uint32_t c;
};

static inline uint64_t next_mwc32_l2(void *s)
{
	struct state_mwc32_l2 *state = s;
	uint64_t x = state->s1*A2A+state->c;
	state->s1 = state->s2;
	state->s2 = x;
	state->c = H32(x);
	return x;
}

static void init_mwc32_l2(struct state_mwc32_l2 *state, uint32_t seed)
{
	int i;
	uint32_t z[4];
	gen_init_state(z, 4, seed);
	state->s1 = L31(z[3]);
	state->s2 = L31(z[2]);
	state->c  = L31(z[1]);
	for (i=0; i<11; i++)
		next_mwc32_l2(state);
}

/* MWC32 L2 with rotation */
struct state_mwc32_l2_r {
	uint32_t s1;
	uint32_t s2;
	uint32_t c;
};

static inline uint64_t next_mwc32_l2_r(void *s)
{
	struct state_mwc32_l2_r *state = s;
	uint64_t x = state->s1*A2A+state->c;
	state->s1 = state->s2;
	state->s2 = x;
	state->c = H32(x);
	return x + RR64(x,25);
}

static void init_mwc32_l2_r(struct state_mwc32_l2_r *state, uint32_t seed)
{
	int i;
	uint32_t z[4];
	gen_init_state(z, 4, seed);
	state->s1 = L31(z[3]);
	state->s2 = L31(z[2]);
	state->c  = L31(z[1]);
	for (i=0; i<11; i++)
		next_mwc32_l2_r(state);
}

/* MWC32 L2 with multiplication
 * rnd_95
 * Passes all of the tests of BigCrush for the following 32-bit ranges:
 *   03-36, 07-40, 11-44, 15-48, 19-52, 23-56, 27-60, 31-00, 35-04, 39-08,
 *   43-12, 47-16, 51-20, 55-24, 59-28, 63-32
 * No ranges failed any of the BigCrush tests.
 */
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
	uint32_t z[4];
	gen_init_state(z, 4, seed);
	state->s1 = L31(z[3]);
	state->s2 = L31(z[2]);
	state->c  = L31(z[1]);
	for (i=0; i<13; i++)
		next_mwc32_l2_m2(state);
}

/* MWC32 x2
 * rnd_126
 * Passes all of the tests of BigCrush for the following 32-bit ranges:
 *   03-36, 07-40, 11-44, 15-48, 19-52, 23-56, 27-60, 31-00, 35-04, 39-08,
 *   43-12, 47-16, 51-20, 55-24, 59-28, 63-32
 * No ranges failed any of the BigCrush tests.
*/
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
	uint32_t z[5];
	gen_init_state(z, 5, seed);
	state->s1 = (uint64_t)L31(z[3]) << 32 | (uint64_t)L31(z[1]);
	state->s2 = (uint64_t)L31(z[4]) << 32 | (uint64_t)L31(z[2]);
	for (i=0; i<13; i++)
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
	uint32_t z[5];
	gen_init_state(z, 5, seed);
	state->s1 = (uint64_t)L31(z[3]) << 32 | (uint64_t)L31(z[1]);
	state->s2 = (uint64_t)L31(z[4]) << 32 | (uint64_t)L31(z[2]);
	for (i=0; i<13; i++)
		next_mwc32_x2_m2(state);
}

/* MWC32 MAS64
 * MAS - Modular Addtion Sequence
 * Passes all of the tests of BigCrush.
 * Four 32-bit ranges tested: 64-33, 48-17, 32-1, and 16-49
 */
struct state_mwc32_mas64 {
	uint64_t s1;
	uint64_t s2;
};

static inline uint64_t next_mwc32_mas64(void *s)
{
	struct state_mwc32_mas64 *state = s;
	uint64_t x1 = L32(state->s1)*A1A+H32(state->s1);
	uint64_t x2 = state->s2 + Z8;
	state->s1 = x1;
	state->s2 = x2;
	return x1 + x2;
}

static void init_mwc32_mas64(struct state_mwc32_mas64 *state, uint32_t seed)
{
	int i;
	uint32_t z[5];
	gen_init_state(z, 5, seed);
	state->s1 = (uint64_t)L31(z[3]) << 32 | (uint64_t)L31(z[1]);
	state->s2 = (uint64_t)z[4] << 32 | (uint64_t)z[2];
	for (i=0; i<13; i++)
		next_mwc32_mas64(state);
}

/* MWC32 L3
 *
 * Passes all of the tests of BigCrush for the following 32-bit ranges:
 *   31-00, 35-04, 39-08, 43-12, 47-16, 48-17, 49-18, 50-19
 * The following ranges fail the listed tests:
 *   51-20: 55  SampleCorr, k = 1
 *          65  SumCollector
 *   52-21: 50  SampleProd, t = 8
 *          51  SampleProd, t = 16
 *          52  SampleProd, t = 24
 *          53  SampleMean, r = 0
 *          55  SampleCorr, k = 1
 *          56  SampleCorr, k = 2
 *          59  WeightDistrib, r = 0
 *          62  WeightDistrib, r = 0
 *          65  SumCollector
 *         101  Run of bits, r = 0
 *
 * Bits 63-51 are biased.
 */
struct state_mwc32_l3 {
	uint32_t s1;
	uint32_t s2;
	uint32_t s3;
	uint32_t c;
};

static inline uint64_t next_mwc32_l3(void *s)
{
	struct state_mwc32_l3 *state = s;
	uint64_t x = state->s1*A3A+state->c;
	state->s1 = state->s2;
	state->s2 = state->s3;
	state->s3 = x;
	state->c = H32(x);
	return x;
}

static void init_mwc32_l3(struct state_mwc32_l3 *state, uint32_t seed)
{
	int i;
	uint32_t z[5];
	gen_init_state(z, 5, seed);
	state->s1 = L31(z[4]);
	state->s2 = L31(z[3]);
	state->s3 = L31(z[2]);
	state->c  = L31(z[1]);
	for (i=0; i<13; i++)
		next_mwc32_l3(state);
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
	uint32_t z[5];
	gen_init_state(z, 5, seed);
	state->s1 = L31(z[4]);
	state->s2 = L31(z[3]);
	state->s3 = L31(z[2]);
	state->c  = L31(z[1]);
	for (i=0; i<13; i++)
		next_mwc32_l3_r(state);
}

/* MWC32 L3 M2
 * rnd_127
 * Passes all of the tests of BigCrush for the following 32-bit ranges:
 *   03-36, 07-40, 11-44, 15-48, 19-52, 23-56, 27-60, 31-00, 35-04, 39-08,
 *   43-12, 47-16, 51-20, 55-24, 59-28, 63-32
 * No ranges failed any of the BigCrush tests.
 */
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
	uint32_t z[5];
	gen_init_state(z, 5, seed);
	state->s1 = L31(z[4]);
	state->s2 = L31(z[3]);
	state->s3 = L31(z[2]);
	state->c  = L31(z[1]);
	for (i=0; i<13; i++)
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
	uint32_t z[5];
	gen_init_state(z, 5, seed);
	state->s1 = (uint64_t)z[3] << 32 | (uint64_t)z[1];
	state->s2 = (uint64_t)z[4] << 32 | (uint64_t)z[2];
	for (i=0; i<13; i++)
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
	uint32_t z[5];
	gen_init_state(z, 5, seed);
	state->s1 = (uint64_t)z[3] << 32 | (uint64_t)z[1];
	state->s2 = (uint64_t)z[4] << 32 | (uint64_t)z[2];
	for (i=0; i<13; i++)
		next_xoroshiro(state);
}

/* MWC32 L2 MAS64 */
struct state_mwc32_l2_mas64 {
	uint32_t s1a;
	uint32_t s1b;
	uint32_t c;
	uint64_t s2;
};

static inline uint64_t next_mwc32_l2_mas64(void *s)
{
	struct state_mwc32_l2_mas64 *state = s;
	uint64_t x1 = state->s1a*A2A+state->c;
	uint64_t x2 = state->s2 + Z9;
	state->s1a = state->s1b;
	state->s1b = x1;
	state->c = H32(x1);
	state->s2 = x2;
	return x1 + x2;
}

static void init_mwc32_l2_mas64(struct state_mwc32_l2_mas64 *state,
								uint32_t seed)
{
	int i;
	uint32_t z[6];
	gen_init_state(z, 6, seed);
	state->s1a = L31(z[5]);
	state->s1b = L31(z[4]);
	state->c   = L31(z[3]);
	state->s2  = (uint64_t)z[2] << 32 | (uint64_t)z[1];
	for (i=0; i<13; i++)
		next_mwc32_l2_mas64(state);
}

/* MWC32 X2 MAS64
 * MAS - Modular Addition Sequence
 */
struct state_mwc32_x2_mas64 {
	uint64_t s1;
	uint64_t s2;
	uint64_t s3;
};

static inline uint64_t next_mwc32_x2_mas64(void *s)
{
	struct state_mwc32_x2_mas64 *state = s;
	uint64_t x1 = L32(state->s1)*A1A+H32(state->s1);
	uint64_t x2 = L32(state->s2)*A1B+H32(state->s2);
	uint64_t x3 = state->s3 + Z8;
	state->s1 = x1;
	state->s2 = x2;
	state->s3 = x3;
	return x1 + FLIP32(x2) + x3;
}

static void init_mwc32_x2_mas64(struct state_mwc32_x2_mas64 *state,
								uint32_t seed)
{
	int i;
	uint32_t z[7];
	gen_init_state(z, 7, seed);
	state->s1 = (uint64_t)L31(z[4]) << 32 | (uint64_t)L31(z[1]);
	state->s2 = (uint64_t)L31(z[5]) << 32 | (uint64_t)L31(z[2]);
	state->s3 = (uint64_t)z[6] << 32 | (uint64_t)z[3];
	for (i=0; i<13; i++)
		next_mwc32_x2_mas64(state);
}

/* MWC32 L2 X2
 * rnd_190
 * Passes all of the tests of BigCrush for the following 32-bit ranges:
 *   03-36, 07-40, 11-44, 15-48, 19-52, 23-56, 27-60, 31-00, 35-04, 39-08,
 *   43-12, 47-16, 51-20, 55-24, 59-28, 63-32
 * No ranges failed any of the BigCrush tests.
 */
struct state_mwc32_l2_x2 {
	uint32_t s1a;
	uint32_t s1b;
	uint32_t c1;
	uint32_t s2a;
	uint32_t s2b;
	uint32_t c2;
};

static inline uint64_t next_mwc32_l2_x2(void *s)
{
	struct state_mwc32_l2_x2 *state = s;
	uint64_t x1 = state->s1a*A2A+state->c1;
	state->s1a = state->s1b;
	state->s1b = x1;
	state->c1 = H32(x1);
	uint64_t x2 = state->s2a*A2B+state->c2;
	state->s2a = state->s2b;
	state->s2b = x2;
	state->c2 = H32(x2);
	return x1 + FLIP32(x2);
}

static void init_mwc32_l2_x2(struct state_mwc32_l2_x2 *state, uint32_t seed)
{
	int i;
	uint32_t z[7];
	gen_init_state(z, 7, seed);
	state->s1a = L31(z[6]);
	state->s1b = L31(z[5]);
	state->c1  = L31(z[4]);
	state->s2a = L31(z[3]);
	state->s2b = L31(z[2]);
	state->c2  = L31(z[1]);
	for (i=0; i<13; i++)
		next_mwc32_l2_x2(state);
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
	uint64_t z1 = Z1 * L32(x1);
	uint64_t z2 = Z2 * L32(x2);
	return z1 + FLIP32(z2);
}

static void init_mwc32_l2_x2_m2(struct state_mwc32_l2_x2_m2 *state,
								uint32_t seed)
{
	int i;
	uint32_t z[7];
	gen_init_state(z, 7, seed);
	state->s1a = L31(z[6]);
	state->s1b = L31(z[5]);
	state->c1  = L31(z[4]);
	state->s2a = L31(z[3]);
	state->s2b = L31(z[2]);
	state->c2  = L31(z[1]);
	for (i=0; i<13; i++)
		next_mwc32_l2_x2_m2(state);
}

/* MWC32 L3 MAS64
 * Passes all of the tests of BigCrush.
 * Three 32-bit ranges tested: 64-33, 48-17, and 32-1 (16-49 not tested)
 */
struct state_mwc32_l3_mas64 {
	uint32_t s1a;
	uint32_t s1b;
	uint32_t s1c;
	uint32_t c;
	uint64_t s2;
};

static inline uint64_t next_mwc32_l3_mas64(void *s)
{
	struct state_mwc32_l3_mas64 *state = s;
	uint64_t x1 = state->s1a*A3A+state->c;
	uint64_t x2 = state->s2 + Z9;
	state->s1a = state->s1b;
	state->s1b = state->s1c;
	state->s1c = x1;
	state->c = H32(x1);
	state->s2 = x2;
	return x1 + x2;
}

static void init_mwc32_l3_mas64(struct state_mwc32_l3_mas64 *state, uint32_t seed)
{
	int i;
	uint32_t z[7];
	gen_init_state(z, 7, seed);
	state->s1a = L31(z[6]);
	state->s1b = L31(z[5]);
	state->s1c = L31(z[4]);
	state->c   = L31(z[3]);
	state->s2  = (uint64_t)z[2] << 32 | (uint64_t)z[1];
	for (i=0; i<13; i++)
		next_mwc32_l3_mas64(state);
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
	uint32_t z[9];
	gen_init_state(z, 9, seed);
	state->n = z[1] % 6;
	state->c = L31(z[2]);
	for (i=0; i<6; i++)
		state->s[i] = L31(z[i+3]);
	for (i=0; i<13; i++)
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
	uint32_t z[9];
	gen_init_state(z, 9, seed);
	state->s1 = (uint64_t)L31(z[3]) << 32 | (uint64_t)L31(z[1]);
	state->s2 = (uint64_t)L31(z[4]) << 32 | (uint64_t)L31(z[2]);
	state->s3 = (uint64_t)L31(z[7]) << 32 | (uint64_t)L31(z[5]);
	state->s4 = (uint64_t)L31(z[8]) << 32 | (uint64_t)L31(z[6]);
	for (i=0; i<13; i++)
		next_mwc32_x4(state);
}

/* MWC32 L3 X2
 * rnd_254
 * Passes all of the tests of BigCrush for the following 32-bit ranges:
 *   03-36, 07-40, 11-44, 15-48, 19-52, 23-56, 27-60, 31-00, 35-04, 39-08,
 *   43-12, 47-16, 51-20, 55-24, 59-28, 63-32
 * No ranges failed any of the BigCrush tests.
*/
struct state_mwc32_l3_x2 {
	uint32_t s1a;
	uint32_t s1b;
	uint32_t s1c;
	uint32_t c1;
	uint32_t s2a;
	uint32_t s2b;
	uint32_t s2c;
	uint32_t c2;
};

static inline uint64_t next_mwc32_l3_x2(void *s)
{
	struct state_mwc32_l3_x2 *state = s;
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
	return x1 + FLIP32(x2);
}

static void init_mwc32_l3_x2(struct state_mwc32_l3_x2 *state, uint32_t seed)
{
	int i;
	uint32_t z[9];
	gen_init_state(z, 9, seed);
	state->s1a = L31(z[8]);
	state->s1b = L31(z[7]);
	state->s1c = L31(z[6]);
	state->c1  = L31(z[5]);
	state->s2a = L31(z[4]);
	state->s2b = L31(z[3]);
	state->s2c = L31(z[2]);
	state->c2  = L31(z[1]);
	for (i=0; i<13; i++)
		next_mwc32_l3_x2(state);
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
	uint64_t z1 = Z1 * L32(x1);
	uint64_t z2 = Z2 * L32(x2);
	return z1 + FLIP32(z2);
}

static void init_mwc32_l3_x2_m2(struct state_mwc32_l3_x2_m2 *state, 
								int32_t seed)
{
	int i;
	uint32_t z[9];
	gen_init_state(z, 9, seed);
	state->s1a = L31(z[8]);
	state->s1b = L31(z[7]);
	state->s1c = L31(z[6]);
	state->c1  = L31(z[5]);
	state->s2a = L31(z[4]);
	state->s2b = L31(z[3]);
	state->s2c = L31(z[2]);
	state->c2  = L31(z[1]);
	for (i=0; i<13; i++)
		next_mwc32_l3_x2_m2(state);
}

/* MWC32 L7 M2
 * rnd_255
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
	uint32_t z[10];
	gen_init_state(z, 10, seed);
	state->n = z[1] % 7;
	state->c = L31(z[2]);
	for (i=0; i<7; i++)
		state->s[i] = L31(z[i+3]);
	for (i=0; i<17; i++)
		next_mwc32_l7_m2(state);
}

/* MWC32 L8
 *
 * Passes all of the tests of BigCrush for the following 32-bit ranges:
 *   31-00, 35-04, 39-08, 43-12, 47-16, 48-17, 49-18, 50-19
 * The following ranges fail the listed tests:
 *   51-20: 65  SumCollector
 *   52-21: 34  Gap, r = 0
 *          51  SampleProd, t = 16
 *          52  SampleProd, t = 24
 *          55  SampleCorr, k = 1
 *          62  WeightDistrib, r = 0
 *          65  SumCollector
 *         101  Run of bits, r = 0
 *
 * Bits 63-51 are biased.
 */
struct state_mwc32_l8 {
	uint8_t n;
	uint32_t c;
	uint32_t s[8];
};

static inline uint64_t next_mwc32_l8(void *s)
{
	struct state_mwc32_l8 *state = s;
	uint8_t n = 0x7&state->n;
	uint64_t x = state->s[n]*A8A+state->c;
	state->s[n] = x;
	state->c = H32(x);
	state->n++;
	return x;
}

static void init_mwc32_l8(struct state_mwc32_l8 *state, uint32_t seed)
{
	int i;
	uint32_t z[11];
	gen_init_state(z, 11, seed);
	state->n = z[1] & 0x7;
	state->c = L31(z[2]);
	for (i=0; i<8; i++)
		state->s[i] = L31(z[i+3]);
	for (i=0; i<17; i++)
		next_mwc32_l8(state);
}

/* MWC32 L8 M2
 * rnd_287
 * Passes all of the tests of BigCrush for the following 32-bit ranges:
 *   03-36, 07-40, 11-44, 15-48, 19-52, 23-56, 27-60, 31-00, 35-04, 39-08,
 *   43-12, 47-16, 51-20, 55-24, 59-28, 63-32
 * No ranges failed any of the BigCrush tests.
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
	uint32_t z[11];
	gen_init_state(z, 11, seed);
	state->n = z[1] & 0x7;
	state->c = L31(z[2]);
	for (i=0; i<8; i++)
		state->s[i] = L31(z[i+3]);
	for (i=0; i<17; i++)
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
	uint32_t z[11];
	gen_init_state(z, 11, seed);
	state->s1 = L59((uint64_t)z[3] << 32 | (uint64_t)z[1]);
	state->s2 = L59((uint64_t)z[4] << 32 | (uint64_t)z[2]);
	state->s3 = L59((uint64_t)z[7] << 32 | (uint64_t)z[5]);
	state->s4 = L59((uint64_t)z[8] << 32 | (uint64_t)z[6]);
	state->c  = L59((uint64_t)z[10] << 32 | (uint64_t)z[9]);
	for (i=0; i<13; i++)
		next_mwc60_l4_m2(state);
}

/* MWC32 L8 MAS32 M2
 * MAS - Modular Addition Sequence
 */
struct state_mwc32_l8_mas_m2 {
	uint32_t s1[8];
	uint32_t s2;
	uint32_t c;
};

static inline uint64_t next_mwc32_l8_mas_m2(void *s)
{
	struct state_mwc32_l8_mas_m2 *state = s;
	uint32_t n = state->s2 & 0x7;
	uint64_t x1 = state->s1[n]*A8A+state->c;
	uint32_t x2 = state->s2 + Z6;
	state->s1[n] = x1;
	state->c = H32(x1);
	state->s2 = x2;
	uint64_t z1 = Z1 * (H32(x1) + x2);
	uint64_t z2 = Z2 * L32(x1);
	return z1 + FLIP32(z2);
}

static void init_mwc32_l8_mas_m2(struct state_mwc32_l8_mas_m2 *state,
								 uint32_t seed)
{
	int i;
	uint32_t z[11];
	gen_init_state(z, 11, seed);
	state->s2 = z[1];
	state->c = L31(z[2]);
	for (i=0; i<8; i++)
		state->s1[i] = L31(z[i+3]);
	for (i=0; i<17; i++)
		next_mwc32_l8_mas_m2(state);
}

/* MWC32 L8 MAS64
 * MAS - Modular Addition Sequence
 */
struct state_mwc32_l8_mas64 {
	uint32_t s1[8];
	uint32_t c;
	uint64_t s2;
};

static inline uint64_t next_mwc32_l8_mas64(void *s)
{
	struct state_mwc32_l8_mas64 *state = s;
	uint32_t n = state->s2 & 0x7;
	uint64_t x1 = state->s1[n]*A8A+state->c;
	uint64_t x2 = state->s2 + Z9;
	state->s1[n] = x1;
	state->c = H32(x1);
	state->s2 = x2;
	return x1 + x2;
}

static void init_mwc32_l8_mas64(struct state_mwc32_l8_mas64 *state,
								 uint32_t seed)
{
	int i;
	uint32_t z[12];
	gen_init_state(z, 12, seed);
	state->s2 = (uint64_t)z[2] << 32 | (uint64_t)z[1];
	state->c = L31(z[3]);
	for (i=0; i<8; i++)
		state->s1[i] = L31(z[i+4]);
	for (i=0; i<17; i++)
		next_mwc32_l8_mas64(state);
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


	TEST_GEN(mwc32, "MWC32", 63);
	TEST_GEN(mwc32_r, "MWC32 R", 63);
	TEST_GEN(mwc32_m2_32, "MWC32 *2 (32)", 63);
	TEST_GEN(mwc32_m2_64, "MWC32 *2 (64)", 63);
	TEST_GEN(lcg64, "LCG64", 64);
	TEST_GEN(smix64, "SMIX64", 64);
	TEST_GEN(xorshift64, "XORShift64", 64);
	TEST_GEN(mwc32_l2_u2, "MWC32 L2 U2", 94);
	TEST_GEN(mwc32_l2, "MWC32 L2", 95);
	TEST_GEN(mwc32_l2_r, "MWC32 L2 R", 95);
	TEST_GEN(mwc32_l2_m2, "MWC32 L2 *2", 95);
	TEST_GEN(mwc32_x2, "MWC32 X2", 126);
	TEST_GEN(mwc32_x2_m2, "MWC32 X2 *2", 126);
	TEST_GEN(mwc32_mas64, "MWC32 MAS64", 127);
	TEST_GEN(mwc32_l3, "MWC32 L3", 127);
	TEST_GEN(mwc32_l3_r, "MWC32 L3 R", 127);
	TEST_GEN(mwc32_l3_m2, "MWC32 L3 *2", 127);
	TEST_GEN(xorshift128p, "XORShift128+", 128);
	TEST_GEN(xoroshiro, "xoroshiro128plus", 128);
	TEST_GEN(mwc32_l2_mas64, "MWC32 L2 MAS64", 159);
	TEST_GEN(mwc32_x2_mas64, "MWC32 X2 MAS64", 190);
	TEST_GEN(mwc32_l2_x2, "MWC32 L2 X2", 190);
	TEST_GEN(mwc32_l2_x2_m2, "MWC32 L2 X2 *2", 190);
	TEST_GEN(mwc32_l3_mas64, "MWC32 L3 MAS64", 191);
	TEST_GEN(mwc32_l6_m2, "MWC32 L6 *2", 223);
	TEST_GEN(mwc32_x4, "MWC32 X4", 252);
	TEST_GEN(mwc32_l3_x2, "MWC32 L3 X2", 254);
	TEST_GEN(mwc32_l3_x2_m2, "MWC32 L3 X2 *2", 254);
	TEST_GEN(mwc32_l7_m2, "MWC32 L7 *2", 255);
	TEST_GEN(mwc32_l8, "MWC32 L8", 287);
	TEST_GEN(mwc32_l8_m2, "MWC32 L8 *2", 287);
	TEST_GEN(mwc60_l4_m2, "MWC60 L4 *2", 299);
	TEST_GEN(mwc32_l8_mas_m2, "MWC32 L8 MAS *2", 319);
	TEST_GEN(mwc32_l8_mas64, "MWC32 L8 MAS64", 351);

	printf("\ntotal = %llu (So compiler won't optimize away the loops)\n",
	       grand_total);
	return 0;
}
