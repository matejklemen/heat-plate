#include "heat_plate.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#define NUM_THREADS 4

struct thread_params
{
	int start_row;
	int end_row;
	// max sprememba temperature, ki jo je nasla posamezna nit
	double curr_max_diff;
};

// niti in parametri
pthread_t workers[NUM_THREADS];
struct thread_params *worker_params[NUM_THREADS];

// lastnosti plosce (globalne, da poenostavimo zadevo)
double **first_plate, **second_plate;
int global_height, global_width;

static void* calc_rows_pt(void *arg)
{
	struct thread_params* params = (thread_params *)arg;

	for(int i = params->start_row; i < params->end_row; i++) {
		for(int j = 1; j < global_width - 1; j++)
		{
			first_plate[i][j] = calc_heat_point(second_plate, i, j);
			double curr_diff = fabs(first_plate[i][j] - second_plate[i][j]);

			// vsaka nit si zapise lokalni maksimum spremembe temperature
			if(curr_diff > params->curr_max_diff)
				params->curr_max_diff = curr_diff;
		}
	}
}

double **calc_heat_plate_pt(int height, int width, int iterations)
{
	global_height = height;
	global_width = width;

	int start_height = 1;
	int end_height = height - 1;

	/*
		first_plate... plosca, za katero bomo v tej iteraciji racunali nove vrednosti
		second_plate... plosca, iz katere bomo vzemali vrednosti za izracun novih vrednosti plosce 'first_plate'
	*/
	first_plate = alloc_plate(height, width);
	second_plate = alloc_plate(height, width);
	
	init_plate(first_plate, height, width);
	init_plate(second_plate, height, width);

	double max_diff = 0;

	// TODO: razdelitev dela
	// Kako razdeliti preostanek (ki se ne da razdeliti povsem pravicno) - sproti (dodaten parameter strukture)
	// ali na koncu (cakanje na niti, ponovno poganjanje niti)?
	for(int i = 0; i < NUM_THREADS; i++)
	{
		struct thread_params *p = (struct thread_params *) malloc(sizeof(struct thread_params));
		p->start_row = i + 1;
		p->end_row = i + 2;
		p->curr_max_diff = 0;

		worker_params[i] = p;
	}

	for(int k = 0; k < iterations; k++)
	{
		max_diff = 0.0;

		// inicializacija niti in racunanje
		for(int i = 0; i < NUM_THREADS; i++)
			pthread_create(&workers[i], NULL, calc_rows_pt, (void *)worker_params[i]);

		// cakanje na niti
		for(int i = 0; i < NUM_THREADS; i++)
		{
			pthread_join(workers[i], NULL);
			// iskanje maksimuma delnih rezultatov niti
			if(worker_params[i]->curr_max_diff > max_diff)
				max_diff = worker_params[i]->curr_max_diff;

			worker_params[i]->curr_max_diff = 0;
		}
		
		swap_pointers(&first_plate, &second_plate);
	}

	// pocistimo strukture za prenasanje parametrov niti
	for(int i = 0; i < NUM_THREADS; i++)
		free(worker_params[i]);
	
	// ne rabimo vec prve plosce
	free_plate(first_plate, height, width);
	
	if(!TIME_MEASUREMENT)
	{
		printf("Maximum heat difference calculated in the last iteration was %lf.\n", max_diff);
	}
	
	return second_plate;
}

