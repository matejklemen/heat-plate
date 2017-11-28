#include "pthread_algorithm.h"
#include "heat_plate.h"
#include "main.h"
#include <stdio.h>
#include <math.h>
#include <pthread.h>

#define NUM_THREADS 4

static int h, w;
static float eps;
static float **first_plate, **second_plate;
static int iterations;

static pthread_t thread[NUM_THREADS];
static pthread_barrier_t barrier;
static float max_thread_diff[NUM_THREADS];

static void *thread_work(void *arg)
{
	int index = (int)arg;
	
	int row_start = 1 + (int)((double)index * (h - 2) / NUM_THREADS);
	int row_end = 1 + (int)((double)(index + 1) * (h - 2) / NUM_THREADS);
	
	iterations = 0;
	
	while(1)
	{
		max_thread_diff[index] = 0.0;
		
		for(int i = row_start; i < row_end; i++)
		{
			for(int j = 1; j < w - 1; j++)
			{
				first_plate[i][j] = calc_heat_point(second_plate, i, j);
				
				float curr_diff = fabs(first_plate[i][j] - second_plate[i][j]);
				
				if(curr_diff > max_thread_diff[index])
					max_thread_diff[index] = curr_diff;
			}
		}
		
		pthread_barrier_wait(&barrier);
		
		if(index == 0)
		{
			swap_pointers(&first_plate, &second_plate);
			iterations++;
		}
		
		float max_diff = 0.0;
		for(int i = 0; i < NUM_THREADS; i++)
		{
			if(max_thread_diff[i] > max_diff)
				max_diff = max_thread_diff[i];
		}
		
		if(max_diff < eps)
			break;
		
		pthread_barrier_wait(&barrier);
	}
	
	return NULL;
}

float **calc_heat_plate_pthread(int height, int width, float epsilon)
{
	h = height;
	w = width;
	eps = epsilon;
	
	first_plate = alloc_plate(height, width);
	second_plate = alloc_plate(height, width);
	
	init_plate(first_plate, height, width);
	init_plate(second_plate, height, width);
	
	pthread_barrier_init(&barrier, NULL, NUM_THREADS);
	
	for(int i = 0; i < NUM_THREADS; i++)
	{
		pthread_create(&thread[i], NULL, thread_work, (void *)i);
	}
		
	for(int i = 0; i < NUM_THREADS; i++)
	{
		pthread_join(thread[i], NULL);
	}
	
	pthread_barrier_destroy(&barrier);
	
	free_plate(first_plate, height, width);
	
#ifndef TIME_MEASUREMENTS
	
	printf("%d iterations.\n", iterations);
	
#endif
	
	return second_plate;
}
