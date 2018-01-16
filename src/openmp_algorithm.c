#include "openmp_algorithm.h"
#include "heat_plate.h"
#include "main.h"
#include <stdio.h>
#include <math.h>
#include <omp.h>

#define NUM_THREADS 4

static float **first_plate, **second_plate;
static int iterations = 0;

float **calc_heat_plate_openmp(int height, int width, float epsilon)
{
	omp_set_num_threads(NUM_THREADS);

	first_plate = alloc_plate(height, width);
	second_plate = alloc_plate(height, width);
	
	init_plate(first_plate, height, width);
	init_plate(second_plate, height, width);

	while(1)
	{
		float max_diff = 0.0;
		
		// v vsaki (razen v robnih) tocki izracunaj novo temperaturo na podlagi starih
		#pragma omp parallel for schedule(static) reduction(max: max_diff)
		for(int i = 1; i < height - 1; i++)
		{
			for(int j = 1; j < width - 1; j++)
			{
				first_plate[i][j] = calc_heat_point(second_plate, i, j);
				
				float curr_diff = fabs(first_plate[i][j] - second_plate[i][j]);
				
				if(curr_diff > max_diff)
					max_diff = curr_diff;
			}
		}
		
		swap_pointers(&first_plate, &second_plate);
		iterations++;
		
		if(max_diff < epsilon)
			break;
	}
	
	free_plate(first_plate, height, width);
	
#ifndef TIME_MEASUREMENTS
	
	printf("%d iterations.\n", iterations);
	
#endif
	
	return second_plate;
}
