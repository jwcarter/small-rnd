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

#include <stdint.h>

typedef struct rnd * rnd_t;

/*
 *  Creation, Initialization, and Destruction
 */

rnd_t rnd_new();
void rnd_free(rnd_t rnd);

void rnd_init(rnd_t rnd, unsigned long seed);

unsigned rnd_get_state_size_bytes();
unsigned rnd_get_state_size_u32();

void rnd_get_state(rnd_t rnd, uint32_t state[]);
void rnd_set_state(rnd_t rnd, uint32_t state[]);

char *rnd_state_to_string(rnd_t rnd);
void rnd_string_to_state(rnd_t rnd, char *state_str);
void rnd_free_state_str(char *state_str);

uint32_t *rnd_state_to_array(rnd_t rnd);
void rnd_array_to_state(rnd_t rnd, uint32_t state[], unsigned size);
void rnd_free_state_array();

/*
 * Random Numbers
 */

/* Returns 2^32-1 (4294967295) */
uint32_t rnd_max32();

/* Returns 2^64-1 (18446744073709551615) */
uint64_t rnd_max64();

/* Max value is 2^32-1 (4294967295) */
uint32_t rnd_u32(rnd_t rnd);

/* Max value is 2^64-1 (18446744073709551615) */
uint64_t rnd_u64(rnd_t rnd);

/* Return double [0,1] in continuous uniform distribution */
double rnd_closed(rnd_t rnd);

/* Return double (0,1) in continuous uniform distribution */
double rnd_open(rnd_t rnd);

/*
 * Continuous Distributions
 */

/* Generate double (low,high) in uniform distribution */
double rnd_dist_uniform(rnd_t rnd, double low, double high);

/* Generate double (low,high) in triangular distribution */
double rnd_dist_triangle(rnd_t rnd, double low, double high);

/* Generate double (low,high) in left half of triangular distribution */
double rnd_dist_triangle_left(rnd_t rnd, double low, double high);

/* Generate double (low,high) in right half of triangular distribution */
double rnd_dist_triangle_right(rnd_t rnd, double low, double high);

/* Generate double (low,high) in skewed triangular distribution
 *   Peak of distribution is at mode.
 *   (mode-low) and (high-mode) are not necessarily equal
*/
double rnd_dist_triangle_skewed(rnd_t rnd, double low, double high,
								double mode);

/* Generate double (-6*sd+mean,6*sd+mean) in (roughly) normal distribution */
double rnd_dist_normal(rnd_t rnd, double mean, double sd);

/*
 * Generate double (low,high) in Irwin-Hall distribution
 *   Median = Mean = Mode = (high-low)/2+low
 *   n=1: uniform distribution
 *   n=2: triangle distribution
 *   n=12: normal distribution (approximate)
*/
double rnd_dist_irwin_hall(rnd_t rnd, unsigned n, double low, double high);

/* Generate double (low,high) in left half of Irwin-Hall distribution */
double rnd_dist_irwin_hall_left(rnd_t rnd, unsigned n, double low,
								double high);

/* Generate double (low,high) in right half of Irwin-Hall distribution */
double rnd_dist_irwin_hall_right(rnd_t rnd, unsigned n, double low,
								 double high);

/*
 * Discrete Distributions
 */

/* Generate int [low,high] in uniform distribution */
int rnd_int(rnd_t rnd, int low, int high);

/* Generate int [n,n*s] from rolling s sided die n times */
unsigned rnd_roll(rnd_t rnd, unsigned n, unsigned s);

/* Generate int [1,s] that is the middle of three rolls of 2 sided die */
unsigned rnd_roll_mid(rnd_t rnd, unsigned s);
