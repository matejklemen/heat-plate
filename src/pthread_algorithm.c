#include "pthread_algorithm.h"
#include "heat_plate.h"
#include "main.h"
#include <stdio.h>
#include <math.h>
#include <pthread.h>

#define NUM_THREADS 4

static int h, w, iter;
static double **first_plate, **second_plate;

static pthread_t thread[NUM_THREADS];
static pthread_barrier_t barrier;
static double max_thread_diff[NUM_THREADS];

static void *thread_work(void *arg)
{
	int index = (int)arg;
	
	int row_start = 1 + (int)((double)index * (h - 2) / NUM_THREADS);
	int row_end = 1 + (int)((double)(index + 1) * (h - 2) / NUM_THREADS);
	
	for(int k = 0; k < iter; k++)
	{
		max_thread_diff[index] = 0.0;
		
		for(int i = row_start; i < row_end; i++)
		{
			for(int j = 1; j < w - 1; j++)
			{
				first_plate[i][j] = calc_heat_point(second_plate, i, j);
				
				double curr_diff = fabs(first_plate[i][j] - second_plate[i][j]);
				
				if(curr_diff > max_thread_diff[index])
					max_thread_diff[index] = curr_diff;
			}
		}
		
		pthread_barrier_wait(&barrier);
		
		if(index == 0)
		{
			swap_pointers(&first_plate, &second_plate);
		}
		
#ifdef EPSILON
		
		double max_diff = 0.0;
		
		for(int i = 0; i < NUM_THREADS; i++)
		{
			if(max_thread_diff[i] > max_diff)
				max_diff = max_thread_diff[i];
		}
		
		if(max_diff < EPSILON)
			break;
		
#endif
		
		pthread_barrier_wait(&barrier);
	}
	
	return NULL;
}

double **calc_heat_plate_pthread(int height, int width, int iterations)
{
	h = height;
	w = width;
	iter = iterations;
	
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
	
	double max_diff = 0.0;
	
	for(int i = 0; i < NUM_THREADS; i++)
	{
		if(max_thread_diff[i] > max_diff)
			max_diff = max_thread_diff[i];
	}
	
	printf("Maximum heat difference calculated in the last iteration was %lf.\n", max_diff);
	
#endif
	
	return second_plate;
}
