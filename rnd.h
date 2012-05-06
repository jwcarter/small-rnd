
typedef struct rnd * rnd_t;

/*
 *  Creation, Initialization, and Destruction
 */

rnd_t rnd_new();

void rnd_init(rnd_t rnd, unsigned long seed);

void rnd_free(rnd_t rnd);

/*
 * Random Numbers
 */

/* Return double [0,1] in continuous uniform distribution */
double rnd_closed(rnd_t rnd);

/* Return double [0,1) in continuous uniform distribution */
double rnd_double(rnd_t rnd);

/* Return double (0,1) in continuous uniform distribution */
double rnd_open(rnd_t rnd);

/* Return double [0,2] in continuous triangular distribution */
double rnd_closed_2(rnd_t rnd);

/* Return double [0,2) in continuous triangular distribution */
double rnd_double_2(rnd_t rnd);

/* Return double (0,2) in continuous triangular distribution */
double rnd_open_2(rnd_t rnd);

/* Return double [0,n] in continuous irwin hall distribution */
double rnd_closed_n(rnd_t, unsigned n);

/* Return double [0,n) in continuous irwin hall distribution */
double rnd_double_n(rnd_t rnd, unsigned n);

/* Return double (0,n) in continuous irwin hall distribution */
double rnd_open_n(rnd_t rnd, unsigned n);

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
