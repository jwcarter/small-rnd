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
#include <stdint.h>
#include <math.h>
#include <time.h> /* Use time() for initializing random seed */
#include <float.h>
#include "rnd.h"

#define TEST_SIZE 100000000

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

void print_state(rnd_t rnd)
{
	int i;
	unsigned size;
	uint32_t *state;
	char *state_str;

	state_str = rnd_state_to_string(rnd);
	printf("Out: %s [String]\n",state_str);
	rnd_free_state_str(state_str);
	state = rnd_state_to_array(rnd);
	size = rnd_get_state_size_u32();
	printf("Out: ");
	for (i=0; i<size; i++) {
		printf("%08lx",state[i]);
	}
	printf(" [Array]\n\n");
	rnd_free_state_array(state);
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

	printf("state size = %u bytes\n\n",rnd_get_state_size_bytes());

	printf("Setting state with string (4 bytes)\n");
	state_str = "12345678";
	printf("In : %s\n",state_str);
	rnd_string_to_state(rnd, state_str);
	print_state(rnd);

	printf("Setting state with string (40 bytes)\n");
	state_str = "01234567a1a2a3a4b1b2b3b4c1c2c3c4d1d2d3d4e1e2e3e4f1f2f3f4A5A6A7A8B5B6B7B8C5C6C7C8";
	printf("In : %s\n",state_str);
	rnd_string_to_state(rnd, state_str);
	print_state(rnd);

	printf("Setting state with array (4 bytes)\n");
	size = sizeof(uint32_t);
	state = malloc(size);
	state[0] = 0x12345678UL;
	printf("In : %0x\n",state[0]);
	rnd_array_to_state(rnd, state, size);
	free(state);
	print_state(rnd);

	printf("Setting state with array (40 bytes)\n");
	size = sizeof(uint32_t)*10;
	state = malloc(size);
	state[0] = 0x12345678UL;
	state[1] = 0xa1a2a3a4UL;
	state[2] = 0xb1b2b3b4UL;
	state[3] = 0xc1c2c3c4UL;
	state[4] = 0xd1d2d3d4UL;
	state[5] = 0xe1e2e3e4UL;
	state[6] = 0xf1f2f3f4UL;
	state[7] = 0xA5A6A7A8UL;
	state[8] = 0xB5B6B7B8UL;
	state[9] = 0xC5C6C7C8UL;
	printf("In : ");
	for (i=0; i<10; i++) {
		printf("%0x",state[i]);
	}
	printf("\n");
	rnd_array_to_state(rnd, state, size);
	free(state);
	print_state(rnd);

	printf("Initializing state with rnd_int() using time\n");
	rnd_init(rnd, (unsigned long)time(NULL));
	print_state(rnd);

	printf("Testing rnd_u32\n");
	dt  = 0.0;
	start = clock();
	for (i=0; i < TEST_SIZE; i++) {
		dt += rnd_u32(rnd);
	}
	stop = clock();
	printf("Avg=%.0f (Expected: %.0f) (Time: %5.3f)\n\n",
					dt/TEST_SIZE, (double)max32/2.0,
					((double)(stop-start)/CLOCKS_PER_SEC));

	printf("Testing rnd_u64\n");
	dt  = 0.0;
	start = clock();
	for (i=0; i < TEST_SIZE; i++) {
		dt += rnd_u64(rnd);
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

	printf("Testing rnd_open\n");
	dt = 0.0;
	start = clock();
	for (i=0; i < TEST_SIZE; i++) {
		dt += rnd_open(rnd);
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
