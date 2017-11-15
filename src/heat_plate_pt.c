#include "heat_plate.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#define NUM_THREADS 4

typedef struct
{
	int start_row;
	int end_row;
	double max_diff;
}
thread_data_t;

static double **first_plate, **second_plate;
static int global_height, global_width;

static void *calc_rows_pt(void *arg)
{
	thread_data_t *data_ptr = (thread_data_t *)arg;
	
	data_ptr->max_diff = 0.0;

	for(int i = data_ptr->start_row; i < data_ptr->end_row; i++)
	{
		for(int j = 1; j < global_width - 1; j++)
		{
			first_plate[i][j] = calc_heat_point(second_plate, i, j);

			double curr_diff = fabs(first_plate[i][j] - second_plate[i][j]);

			if(curr_diff > data_ptr->max_diff)
				data_ptr->max_diff = curr_diff;
		}
	}

	return NULL;
}

double **calc_heat_plate_pt(int height, int width, int iterations)
{
	/*
		first_plate... plosca, za katero bomo v tej iteraciji racunali nove vrednosti
		second_plate... plosca, iz katere bomo vzemali vrednosti za izracun novih vrednosti plosce 'first_plate'
	*/
	first_plate = alloc_plate(height, width);
	second_plate = alloc_plate(height, width);
	
	global_height = height;
	global_width = width;
	
	init_plate(first_plate, height, width);
	init_plate(second_plate, height, width);

	pthread_t thread[NUM_THREADS];
	thread_data_t thread_data[NUM_THREADS];

	// enkratno racunanje mej za deljenje dela med niti
	for(int i = 0; i < NUM_THREADS; i++)
	{
		thread_data[i].start_row = 1 + (int)((double)i * (height - 2) / NUM_THREADS);
		thread_data[i].end_row = 1 + (int)((double)(i + 1) * (height - 2) / NUM_THREADS);
	}

	double max_diff;

	for(int k = 0; k < iterations; k++)
	{
		max_diff = 0.0;

		// inicializacija niti in racunanje
		for(int i = 0; i < NUM_THREADS; i++)
			pthread_create(&thread[i], NULL, calc_rows_pt, (void *)&thread_data[i]);

		// cakanje na niti
		for(int i = 0; i < NUM_THREADS; i++)
		{
			pthread_join(thread[i], NULL);
			
			// iskanje maksimuma delnih rezultatov niti
			if(thread_data[i].max_diff > max_diff)
				max_diff = thread_data[i].max_diff;
		}
		
		swap_pointers(&first_plate, &second_plate);
	}

	// ne rabimo vec prve plosce
	free_plate(first_plate, height, width);
	
	if(!TIME_MEASUREMENT)
	{
		printf("Maximum heat difference calculated in the last iteration was %lf.\n", max_diff);
	}
	
	return second_plate;
}
