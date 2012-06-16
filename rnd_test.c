#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h> /* Use time() for initializing random seed */

#include "rnd.h"

#define NUM_ROLLS 10000000
static void print_dist(int min, int max, int dist[])
{
	int i,j,k;

	j=min;
	k=min+20;
	do {
		if (k > max)
			k = max+1;
		for (i=j;i<k;i++)
			printf("%4d",i);
		printf("\n");
		for (i=j;i<k;i++)
			printf("%4.0f",floor(dist[i]*1000.0/NUM_ROLLS+0.5));
		printf("\n\n");
		j=k;
		k=k+20;
	} while (j <= max);
}

#define MAX_TEST 32
void rnd_test(rnd_t rnd)
{
	int dist[20];
	int i,r,total;
	double dr, dt;
	int low=1,high=2;
	clock_t start,stop;

	for (i=0; i < 20; i++)
		dist[i] = 0;

	printf("Testing the random number generator\n");
	printf("Testing rnd_int\n");
	total = 0;
	start = clock();
	for (i=0; i < NUM_ROLLS; i++) {
		r = rnd_int(rnd,0,19);
		dist[r]++;
		total += r;
	}
	stop = clock();
	printf("Range: %d-%d: Avg=%5.3f (Expected: %5.3f) (Time: %5.3f)\n", 
					0, 19, (double)total/NUM_ROLLS, (0+19)/2.0, 
					((double)(stop-start)/CLOCKS_PER_SEC));
	print_dist(0, 19, dist);

	printf("Testing rnd_roll\n");
	for (i=0; i < 20; i++)
		dist[i] = 0;
	total = 0;
	start = clock();
	for (i=0; i < NUM_ROLLS; i++) {
		r = rnd_roll(rnd,3,6);
		dist[r]++;
		total += r;
	}
	stop = clock();
	printf("Range: %d-%d: Avg=%5.3f (Expected: %5.3f) (Time: %5.3f)\n", 
					3, 18, (double)total/NUM_ROLLS, (3+18)/2.0, 
					((double)(stop-start)/CLOCKS_PER_SEC));
	print_dist(0, 19, dist);

	printf("Testing rnd_roll_mid\n");
	for (i=0; i < 20; i++)
		dist[i] = 0;
	total = 0;
	start = clock();
	for (i=0; i < NUM_ROLLS; i++) {
		r = rnd_roll_mid(rnd,18);
		dist[r]++;
		total += r;
	}
	stop = clock();
	printf("Range: %d-%d: Avg=%5.3f (Expected: %5.3f) (Time: %5.3f)\n", 
					1, 18, (double)total/NUM_ROLLS, (1+18)/2.0, 
					((double)(stop-start)/CLOCKS_PER_SEC));
	print_dist(0, 19, dist);

	printf("Testing rnd_dist_uniform\n");
	for (i=0; i < 20; i++)
		dist[i] = 0;
	dt = 0;
	start = clock();
	for (i=0; i < NUM_ROLLS; i++) {
		dr = rnd_dist_uniform(rnd,0,20);
		dt += dr;
		dist[(int)floor(dr)]++;
	}
	stop = clock();
	printf("Range: %d-%d: Avg=%5.3f (Expected: %5.3f) (Time: %5.3f)\n",
					0, 19, (double)dt/NUM_ROLLS, (0+20)/2.0, 
					((double)(stop-start)/CLOCKS_PER_SEC));
	print_dist(0, 19, dist);

	printf("Testing rnd_dist_normal\n");
	for (i=0; i < 20; i++)
		dist[i] = 0;
	dt = 0;
	start = clock();
	for (i=0; i < NUM_ROLLS; i++) {
		dr = rnd_dist_normal(rnd,10,1.5);
		dt += dr;
		dist[(int)floor(dr)]++;
	}
	stop = clock();
	printf("Range: %d-%d: Avg=%5.3f (Expected: %5.3f) (Time: %5.3f)\n",
					0, 19, (double)dt/NUM_ROLLS, (0+20)/2.0,
					((double)(stop-start)/CLOCKS_PER_SEC));
	print_dist(0, 19, dist);

	printf("Testing rnd_dist_triangle\n");
	for (i=0; i < 20; i++)
		dist[i] = 0;
	dt = 0;
	start = clock();
	for (i=0; i < NUM_ROLLS; i++) {
		dr = rnd_dist_triangle(rnd,0,20);
		dt += dr;
		dist[(int)floor(dr)]++;
	}
	stop = clock();
	printf("Range: %d-%d: Avg=%5.3f (Expected: %5.3f) (Time: %5.3f)\n",
					0, 19, (double)dt/NUM_ROLLS, (0+20)/2.0,
					((double)(stop-start)/CLOCKS_PER_SEC));
	print_dist(0, 19, dist);

	printf("Testing rnd_dist_irwin_hall\n");
	for (i=0; i < 20; i++)
		dist[i] = 0;
	dt = 0;
	start = clock();
	for (i=0; i < NUM_ROLLS; i++) {
		dr = rnd_dist_irwin_hall(rnd,12,0,20);
		dt += dr;
		dist[(int)floor(dr)]++;
	}
	stop = clock();
	printf("Range: %d-%d: Avg=%5.3f (Expected: %5.3f) (Time: %5.3f)\n",
					0, 19, (double)dt/NUM_ROLLS, (0+20)/2.0,
					((double)(stop-start)/CLOCKS_PER_SEC));
	print_dist(0, 19, dist);
}

int main(int argc, char **argv)
{
	rnd_t rnd = rnd_new();
	rnd_init(rnd, (unsigned long)time(NULL));

	rnd_test(rnd);

	rnd_free(rnd);
}
