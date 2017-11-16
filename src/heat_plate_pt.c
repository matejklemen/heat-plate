#include "heat_plate.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#define NUM_THREADS 4

static int h, w, iter;
static double **first_plate, **second_plate;
static double max_thread_diff[NUM_THREADS];

static void *thread_work(void *arg)
{
	int index = (int)arg;
	
	int row_start = 1 + (int)((double)index * (h - 2) / NUM_THREADS);
	int row_end = 1 + (int)((double)(index + 1) * (h - 2) / NUM_THREADS);
	
	for(int k = 0; k < iter; i++)
	{
		max_diff[index] = 0.0;
		
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
		
		// TODO: barrier
		
		if(index == 0)
		{
			swap_pointers(&first_plate, &second_plate);
		}
		
		/*
		double max_diff = 0.0;
		
		for(int i = 0; i < NUM_THREADS; i++)
		{
			if(max_thread_diff[i] > max_diff)
				max_diff = max_thread_diff[i];
		}
		
		if(max_diff < EPS)
			return NULL;
		*/
		
		// TODO: barrier
	}
	
	return NULL;
}

double **calc_heat_plate_pt(int height, int width, int iterations)
{
	// argumenti naj bodo globalni
	h = height;
	w = width;
	iter = iterations;
	
	// naslove alociranih plosc tudi shranimo globalno
	first_plate = alloc_plate(height, width);
	second_plate = alloc_plate(height, width);
	
	// inicializacija plosc
	init_plate(first_plate, height, width);
	init_plate(second_plate, height, width);
	
	pthread_t thread[NUM_THREADS];
	
	// inicializacija niti in racunanje
	for(int i = 0; i < NUM_THREADS; i++)
		pthread_create(&thread[i], NULL, thread_work, (void *)i);
		
	// cakanje na niti
	for(int i = 0; i < NUM_THREADS; i++)
		pthread_join(thread[i], NULL);
	
	// ne rabimo vec prve plosce
	free_plate(first_plate, height, width);
	
	if(!TIME_MEASUREMENT)
	{
		double max_diff = 0.0;
		
		for(int i = 0; i < NUM_THREADS; i++)
		{
			if(max_thread_diff[i] > max_diff)
				max_diff = max_thread_diff[i];
		}
		
		printf("Maximum heat difference calculated in the last iteration was %lf.\n", max_diff);
	}
	
	return second_plate;
}
