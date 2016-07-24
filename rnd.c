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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>

#include "rnd.h"

rnd_t rnd_new()
{
	unsigned size = rnd_get_state_size_bytes();
	rnd_t rnd = malloc(size);
	if (!rnd) {
		fprintf(stderr,"rnd: Malloc failed!");
		exit(-1);
	}
	return rnd;
}

void rnd_free(rnd_t rnd)
{
	free(rnd);
}

unsigned rnd_get_state_size_u32()
{
	return rnd_get_state_size_bytes()/(sizeof (uint32_t));
}

uint32_t *rnd_state_to_array(rnd_t rnd)
{
	unsigned size = rnd_get_state_size_bytes();
	uint32_t *state = malloc(size);
	if (!state) {
		fprintf(stderr,"rnd: Malloc failed!");
		exit(-1);
	}

	rnd_get_state(rnd, state);

	return state;
}

void rnd_array_to_state(rnd_t rnd, uint32_t state[], unsigned size)
{
	unsigned size_bytes = rnd_get_state_size_bytes();
	unsigned size_u32 = rnd_get_state_size_u32();
	unsigned s32,i,j;

	uint32_t *s = malloc(size_bytes);
	if (!s) {
		fprintf(stderr,"rnd: Malloc failed!");
		exit(-1);
	}

	s32 = size/(sizeof(uint32_t));
	if (s32 > size_u32)
		s32 = size_u32;
	if (s32 == 0) {
		fprintf(stderr,"rnd: No state passed");
		exit(-1);
	}
	i=j=0;
	while (i < size_u32) {
		s[i] = state[j];
		i++;
		j++;
		if (j >= s32)
			j=0;
	}

	rnd_set_state(rnd, s);

	free(s);
}

void rnd_free_state_array(uint32_t *state)
{
	free(state);
}

static char *put_ul(char *cur, uint32_t v)
{
	unsigned i;

	for (i=0; i<8; i++) {
		char x = (0xF0000000UL & v)>>28;
		if (x < 10) {
			*cur = x + '0';
		} else {
			*cur = x - 10 + 'a';
		}
		v = v<<4;
		cur++;
	}

	return cur;
}

char *rnd_state_to_string(rnd_t rnd)
{
	unsigned i;
	unsigned size = rnd_get_state_size_bytes();
	uint32_t *state;
	char *state_str;
	char * cur;

	if (!rnd) {
		fprintf(stderr,"rnd: No generator state!");
		exit(-1);
	}

	state = rnd_state_to_array(rnd);

	state_str = malloc(2*size+1);
	if (!state_str) {
		fprintf(stderr,"rnd: Malloc failed!");
		exit(-1);
	}

	cur = state_str;
	for (i=0; i<size/sizeof(uint32_t); i++) {
		cur = put_ul(cur, state[i]);
	}

	*cur = '\0';

	rnd_free_state_array(state);

	return state_str;
}

static char *get_ul(char *start, char *cur, uint32_t *v)
{
	unsigned i = 0;
	uint32_t value = 0;

	while (i < 8) {
		value = value<<4;
		if (*cur >= '0' && *cur <= '9') {
			value |= (unsigned long)(*cur - '0');
		} else if (*cur >= 'A' && *cur <= 'F') {
			value |= (unsigned long)(*cur - 'A' + 10);
		} else if (*cur >= 'a' && *cur <= 'f') {
			value |= (unsigned long)(*cur - 'a' + 10);
		} else if (*cur == '\0') {
			/* end of string -- wrap around */
			i--;
			value = value >>4;
		} else {
			printf("Illegal character in state string (%c)\n",*cur);
			exit(-1);
		}

		if (*cur != '\0') {
			cur++;
		} else {
			cur = start;
		}

		i++;
	}

	*v = value;

	return cur;
}

void rnd_string_to_state(rnd_t rnd, char *state_str)
{
	unsigned i;
	char * cur = state_str;
	unsigned size = rnd_get_state_size_bytes();
	unsigned size32 = rnd_get_state_size_u32();
	uint32_t *state;

	if (!rnd) {
		fprintf(stderr,"rnd: No generator state!");
		exit(-1);
	}

	if (!state_str) {
		fprintf(stderr,"rnd: No state string was passed!");
		exit(-1);
	}

	state = malloc(size);
	if (!state) {
		fprintf(stderr,"rnd: Malloc failed!");
		exit(-1);
	}

	for (i=0; i<size32; i++) {
		cur = get_ul(state_str, cur, &state[i]);
	}

	rnd_array_to_state(rnd, state, size);

	free(state);
}

void rnd_free_state_str(char *state_str)
{
	free(state_str);
}

/*
 * Max values
 */

#define UMAX32 4294967295ULL
uint32_t rnd_max32()
{
	return UMAX32;
}

#define UMAX64 18446744073709551615ULL
uint64_t rnd_max64()
{
	return UMAX64;
}

/*
 * Continuous Distributions
 */

double rnd_dist_uniform(rnd_t rnd, double low, double high)
{
/* Return double (low,high) in uniform distribution */
	return rnd_open(rnd)*(high-low)+low;
}

double rnd_dist_triangle(rnd_t rnd, double low, double high)
{
/* return double (low,high) in triangular distribution */
	return (rnd_open(rnd)+rnd_open(rnd))/2.0*(high-low)+low;
}

double rnd_dist_triangle_left(rnd_t rnd, double low, double high)
{
/* Return double (low,high) in left half of triangular distribution */
	double x = rnd_open(rnd)+rnd_open(rnd)-1;
	x = (x < 0) ? x : -x;
	return x*(high-low)+low;
}

double rnd_dist_triangle_right(rnd_t rnd, double low, double high)
{
/* Return double (low,high) in right half of triangular distribution */
	double x = rnd_open(rnd)+rnd_open(rnd)-1;
	x = (x < 0) ? -x : x;
	return x*(high-low)+low;
}

double rnd_dist_triangle_skewed(rnd_t rnd, double low, double high,
				double mode)
{
/* Return double (low,high) in skewed triangular distribution
 *   Peak of distribution is at mode.
 *   (mode-low) and (high-mode) are not necessarily equal
*/
	double x = rnd_open(rnd)+rnd_open(rnd)-1;
	double v =  (x < 0) ? (mode-low) : (high-mode);
	return v*x+mode;
}

double rnd_dist_normal(rnd_t rnd, double mean, double sd)
{
	/* Return double in normal distribution using the Marsaglia polar method */
	double u, v, s;

	do {
		u = rnd_closed(rnd)*2.0-1.0;
		v = rnd_closed(rnd)*2.0-1.0;
		s = u*u + v*v;
	} while (s == 0.0 || s >= 1.0);

	return u*sqrt(-2.0*log(s)/s)*sd + mean;
}

double rnd_dist_irwin_hall(rnd_t rnd, unsigned n, double low, double high)
{
/*
 * Return double (low,high) in Irwin-Hall distribution
 *   Median = Mean = Mode = (high-low)/2+low
 *   n=1: uniform distribution
 *   n=2: triangle distribution
 *   n=12: normal distribution (approximate)
*/
	unsigned i = n;
	double x = 0;
	for (i = 0; i<n; i++) {
		x += rnd_open(rnd);
	}
	return x/(double)n*(high-low)+low;
}

double rnd_dist_irwin_hall_left(rnd_t rnd, unsigned n, double low,
				double high)
{
/* Return double (low,high) in left half of Irwin-Hall distribution */
	unsigned i = n;
	double x = 0;
	for (i = 0; i<n; i++) {
		x += rnd_open(rnd);
	}
	x = x/(double)n*2.0-1.0;
	x = (x < 0) ? x : -x;
	return (x+1.0)*(high-low)+low;
}

double rnd_dist_irwin_hall_right(rnd_t rnd, unsigned n, double low,
				double high)
{
/* Return double (low,high) in right half of Irwin-Hall distribution */
	unsigned i = n;
	double x = 0;
	for (i = 0; i<n; i++) {
		x += rnd_open(rnd);
	}
	x = x/(double)n*2.0-1.0;
	x = (x < 0) ? -x : x;
	return x*(high-low)+low;
}

/*
 * Discrete Distributions
 */

int rnd_int(rnd_t rnd, int low, int high)
{
/* Return int [low,high] in discrete uniform distribution */
	return (int)floor(rnd_open(rnd)*(double)(high-low+1))+low;
}

unsigned rnd_roll(rnd_t rnd, unsigned n, unsigned s)
{
/* Return unsigned [n,n*s] from rolling s sided die n times */
	unsigned i = n;
	double x = 0;
	for (i = 0; i<n; i++) {
		x += rnd_open(rnd);
	}
	return (unsigned)floor(x/(double)n*(double)((s-1)*n+1))+n;
}

unsigned rnd_roll_mid(rnd_t rnd, unsigned s)
{
/* Return unsigned [1,s] that is the middle of three rolls of s sided die */
	double x;
	double x1 = rnd_open(rnd);
	double x2 = rnd_open(rnd);
	double x3 = rnd_open(rnd);
	if (x1 < x2 && x1 < x3)
		x = (x2 < x3) ? x2 : x3;
	else if (x2 < x1 && x2 < x3)
		x = (x1 < x3) ? x1 : x3;
	else
		x = (x1 < x2) ? x1 : x2;
	return (unsigned)floor(x*((double)s))+1;
}
