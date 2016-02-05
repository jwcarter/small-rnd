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
	uint32_t max32 = rnd_max32();
	uint64_t max64 = rnd_max64();
	unsigned size;
	uint32_t *state;
	char *state_str;

	for (i=0; i < 20; i++)
		dist[i] = 0;

	printf("Testing the random number generator\n");
	printf("rnd max32: %u\n",max32);
	printf("rnd max64: %llu\n",max64);
	printf("\n");

	printf("state size = %u\n\n",rnd_get_state_size());

	printf("Setting state with string (Size right for MWC32 lag 2)\n");
	state_str = "345678901abcdef22fedcba1"; /* MWC60 use only lower 30bits */
	printf("In : %s\n",state_str);
	rnd_string_to_state(rnd, state_str);
	state_str = rnd_state_to_string(rnd);
	printf("Out: %s\n\n",state_str);

	printf("Setting state with string (Size right for MWC60 lag 3)\n");
	state_str = "34567890098765431abcdef22fedcba134567890098765431abcdef22fedcba1";
	printf("In : %s\n",state_str);
	rnd_string_to_state(rnd, state_str);
	state_str = rnd_state_to_string(rnd);
	printf("Out: %s\n\n",state_str);

	rnd_init(rnd, (unsigned long)time(NULL));
	printf("Getting state\n");
	state = malloc(size);
	size = rnd_get_state_size();
	rnd_get_state(rnd, state);
	printf("From array : ");
	for (i=0; i<size/sizeof(uint32_t); i++) {
		printf("%08lx",state[i]);
	}
	printf("\n");
	state_str = rnd_state_to_string(rnd);
	printf("From string: %s\n\n",state_str);

	printf("Testing rnd_unsigned32\n");
	dt  = 0.0;
	start = clock();
	for (i=0; i < TEST_SIZE; i++) {
		dt += rnd_unsigned32(rnd);
	}
	stop = clock();
	printf("Avg=%.0f (Expected: %.0f) (Time: %5.3f)\n\n", 
					dt/TEST_SIZE, (double)max32/2.0, 
					((double)(stop-start)/CLOCKS_PER_SEC));
	
	printf("Testing rnd_unsigned64\n");
	dt  = 0.0;
	start = clock();
	for (i=0; i < TEST_SIZE; i++) {
		dt += rnd_unsigned64(rnd);
	}
	stop = clock();
	printf("Avg=%.0f (Expected: %.0f) (Time: %5.3f)\n\n", 
					dt/TEST_SIZE, (double)max64/2.0, 
					((double)(stop-start)/CLOCKS_PER_SEC));

	printf("Testing rnd_closed\n");
	dt = 0.0;
	start = clock();
	for (i=0; i < TEST_SIZE; i++) {
		dt += rnd_closed(rnd);
	}
	stop = clock();
	printf("Avg=%7.5f (Expected: %7.5f) (Time: %5.3f)\n\n", 
					dt/TEST_SIZE, 0.5, 
					((double)(stop-start)/CLOCKS_PER_SEC));

	printf("Testing rnd_double\n");
	dt = 0.0;
	start = clock();
	for (i=0; i < TEST_SIZE; i++) {
		dt += rnd_double(rnd);
	}
	stop = clock();
	printf("Avg=%7.5f (Expected: %7.5f) (Time: %5.3f)\n\n", 
					dt/TEST_SIZE, 0.5, 
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

	rnd_test(rnd);

	rnd_free(rnd);
}
