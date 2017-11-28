#include <omp.h>
#include "heat_plate.h"
#include "main.h"
#include <stdio.h>
#include <math.h>

// #define NUM_THREADS 4

static float **first_plate, **second_plate;
static int iterations = 0;

float **calc_heat_plate_omp(int height, int width, float epsilon)
{
	first_plate = alloc_plate(height, width);
	second_plate = alloc_plate(height, width);
	
	init_plate(first_plate, height, width);
	init_plate(second_plate, height, width);

	while(1)
	{
		float max_diff = 0.0;
		
		// v vsaki (razen v robnih) tocki izracunaj novo temperaturo na podlagi starih
		for(int i = 1; i < height - 1; i++)
		{
			for(int j = 1; j < width - 1; j++)
			{
				// TODO
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