#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <time.h> /* Use time() for initializing random seed */

#include "rnd.h"

#define TEST_SIZE 10000000

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
			printf("%4.0f",floor(dist[i]*1000.0/TEST_SIZE+0.5));
		printf("\n\n");
		j=k;
		k=k+20;
	} while (j <= max);
}

#define MAX_TEST 32
void rnd_test(rnd_t rnd)
{
	int dist[20];
	int i,r;
	uint64_t total;
	double dr, dt;
	clock_t start,stop;
	uint64_t max = rnd_number_max();

	for (i=0; i < 20; i++)
		dist[i] = 0;

	printf("Testing the random number generator\n");
	printf("rnd_number max: %llu\n",max);
	printf("\n");

	printf("Testing rnd_number\n");
	total = 0;
	start = clock();
	for (i=0; i < TEST_SIZE; i++) {
		total += rnd_number(rnd);
	}
	stop = clock();
	printf("Avg=%.0f (Expected: %.0f) (Time: %5.3f)\n\n", 
					(double)total/TEST_SIZE, (double)max/2.0, 
					((double)(stop-start)/CLOCKS_PER_SEC));

	printf("Testing rnd_closed\n");
	dt = 0.0;
	start = clock();
	for (i=0; i < TEST_SIZE; i++) {
		dt += rnd_closed(rnd);
	}
	stop = clock();
	printf("Avg=%7.5f (Expected: %7.5f) (Time: %5.3f)\n\n", 
					(double)dt/TEST_SIZE, 0.5, 
					((double)(stop-start)/CLOCKS_PER_SEC));

	printf("Testing rnd_double\n");
	dt = 0.0;
	start = clock();
	for (i=0; i < TEST_SIZE; i++) {
		dt += rnd_double(rnd);
	}
	stop = clock();
	printf("Avg=%7.5f (Expected: %7.5f) (Time: %5.3f)\n\n", 
					(double)dt/TEST_SIZE, 0.5, 
					((double)(stop-start)/CLOCKS_PER_SEC));

	printf("Testing rnd_int\n");
	total = 0;
	start = clock();
	for (i=0; i < TEST_SIZE; i++) {
		r = rnd_int(rnd,0,19);
		dist[r]++;
		total += r;
	}
	stop = clock();
	printf("Range: %d-%d: Avg=%5.3f (Expected: %5.3f) (Time: %5.3f)\n", 
					0, 19, (double)total/TEST_SIZE, (0+19)/2.0, 
					((double)(stop-start)/CLOCKS_PER_SEC));
	print_dist(0, 19, dist);

	printf("Testing rnd_roll\n");
	for (i=0; i < 20; i++)
		dist[i] = 0;
	total = 0;
	start = clock();
	for (i=0; i < TEST_SIZE; i++) {
		r = rnd_roll(rnd,3,6);
		dist[r]++;
		total += r;
	}
	stop = clock();
	printf("Range: %d-%d: Avg=%5.3f (Expected: %5.3f) (Time: %5.3f)\n", 
					3, 18, (double)total/TEST_SIZE, (3+18)/2.0, 
					((double)(stop-start)/CLOCKS_PER_SEC));
	print_dist(0, 19, dist);

	printf("Testing rnd_roll_mid\n");
	for (i=0; i < 20; i++)
		dist[i] = 0;
	total = 0;
	start = clock();
	for (i=0; i < TEST_SIZE; i++) {
		r = rnd_roll_mid(rnd,18);
		dist[r]++;
		total += r;
	}
	stop = clock();
	printf("Range: %d-%d: Avg=%5.3f (Expected: %5.3f) (Time: %5.3f)\n", 
					1, 18, (double)total/TEST_SIZE, (1+18)/2.0, 
					((double)(stop-start)/CLOCKS_PER_SEC));
	print_dist(0, 19, dist);

	printf("Testing rnd_dist_uniform\n");
	for (i=0; i < 20; i++)
		dist[i] = 0;
	dt = 0;
	start = clock();
	for (i=0; i < TEST_SIZE; i++) {
		dr = rnd_dist_uniform(rnd,0,20);
		dt += dr;
		dist[(int)floor(dr)]++;
	}
	stop = clock();
	printf("Range: %d-%d: Avg=%5.3f (Expected: %5.3f) (Time: %5.3f)\n",
					0, 19, (double)dt/TEST_SIZE, (0+20)/2.0, 
					((double)(stop-start)/CLOCKS_PER_SEC));
	print_dist(0, 19, dist);

	printf("Testing rnd_dist_normal\n");
	for (i=0; i < 20; i++)
		dist[i] = 0;
	dt = 0;
	start = clock();
	for (i=0; i < TEST_SIZE; i++) {
		dr = rnd_dist_normal(rnd,10,1.0);
		dt += dr;
		dist[(int)floor(dr)]++;
	}
	stop = clock();
	printf("Range: %d-%d: Avg=%5.3f (Expected: %5.3f) (Time: %5.3f)\n",
					0, 19, (double)dt/TEST_SIZE, (0+20)/2.0,
					((double)(stop-start)/CLOCKS_PER_SEC));
	print_dist(0, 19, dist);

	printf("Testing rnd_dist_triangle\n");
	for (i=0; i < 20; i++)
		dist[i] = 0;
	dt = 0;
	start = clock();
	for (i=0; i < TEST_SIZE; i++) {
		dr = rnd_dist_triangle(rnd,0,20);
		dt += dr;
		dist[(int)floor(dr)]++;
	}
	stop = clock();
	printf("Range: %d-%d: Avg=%5.3f (Expected: %5.3f) (Time: %5.3f)\n",
					0, 19, (double)dt/TEST_SIZE, (0+20)/2.0,
					((double)(stop-start)/CLOCKS_PER_SEC));
	print_dist(0, 19, dist);

	printf("Testing rnd_dist_irwin_hall\n");
	for (i=0; i < 20; i++)
		dist[i] = 0;
	dt = 0;
	start = clock();
	for (i=0; i < TEST_SIZE; i++) {
		dr = rnd_dist_irwin_hall(rnd,12,4,16);
		dt += dr;
		dist[(int)floor(dr)]++;
	}
	stop = clock();
	printf("Range: %d-%d: Avg=%5.3f (Expected: %5.3f) (Time: %5.3f)\n",
					0, 19, (double)dt/TEST_SIZE, (0+20)/2.0,
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
