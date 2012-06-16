#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "rnd.h"

/*
 * Continuous Distributions
 */

double rnd_dist_uniform(rnd_t rnd, double low, double high)
{
/* Return double (low,high) in uniform distribution */
	return rnd_double(rnd)*(high-low)+low;
}

double rnd_dist_triangle(rnd_t rnd, double low, double high)
{
/* return double (low,high) in triangular distribution */
	return rnd_double_2(rnd)/2.0*(high-low)+low;
}

double rnd_dist_triangle_left(rnd_t rnd, double low, double high)
{
/* Return double (low,high) in left half of triangular distribution */
	double x = rnd_double_2(rnd)-1;
	x = (x < 0) ? x : -x;
	return x*(high-low)+low;
}

double rnd_dist_triangle_right(rnd_t rnd, double low, double high)
{
/* Return double (low,high) in right half of triangular distribution */
	double x = rnd_double_2(rnd)-1;
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
	double x = rnd_double_2(rnd)-1;
	double v =  (x < 0) ? (mode-low) : (high-mode);
	return v*x+mode;
}

double rnd_dist_normal(rnd_t rnd, double mean, double sd)
{
/* Return double (-6*sd+mean,6*sd+mean) in (roughly) normal distribution */
	return rnd_double_n(rnd,12)*sd-6.0*sd+mean;
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
	return rnd_double_n(rnd,n)/(double)n*(high-low)+low;
}

double rnd_dist_irwin_hall_left(rnd_t rnd, unsigned n, double low, 
				double high)
{
/* Return double (low,high) in left half of Irwin-Hall distribution */
	double x = rnd_double_n(rnd,n)/(double)n*2.0-1.0;
	x = (x < 0) ? x : -x;
	return (x+1.0)*(high-low)+low;
}

double rnd_dist_irwin_hall_right(rnd_t rnd, unsigned n, double low, 
				double high)
{
/* Return double (low,high) in right half of Irwin-Hall distribution */
	double x = rnd_double_n(rnd,n)/(double)n*2.0-1.0;
	x = (x < 0) ? -x : x;
	return x*(high-low)+low;
}

/*
 * Discrete Distributions
 */

int rnd_int(struct rnd *rnd, int low, int high)
{
/* Return int [low,high] in discrete uniform distribution */
	return (int)floor(rnd_double(rnd)*((double)high-(double)low+1.0))+low;
}

unsigned rnd_roll(rnd_t rnd, unsigned n, unsigned s)
{
/* Return unsigned [n,n*s] from rolling s sided die n times */
	return (unsigned)floor(rnd_double_n(rnd,n)/(double)n*
					(((double)s-1.0)*(double)n+1.0))+n;
}

unsigned rnd_roll_mid(rnd_t rnd, unsigned s)
{
/* Return unsigned [1,s] that is the middle of three rolls of s sided die */
	double x;
	double x1 = rnd_double(rnd);
	double x2 = rnd_double(rnd);
	double x3 = rnd_double(rnd);
	if (x1 < x2 && x1 < x3)
		x = (x2 < x3) ? x2 : x3;
	else if (x2 < x1 && x2 < x3)
		x = (x1 < x3) ? x1 : x3;
	else
		x = (x1 < x2) ? x1 : x2;
	return (unsigned)floor(x*((double)s))+1;
}
