#include "serial_algorithm.h"
#include "heat_plate.h"
#include "main.h"
#include <stdio.h>
#include <math.h>

double **calc_heat_plate_serial(int height, int width, int iterations)
{
	/*
		first_plate... plosca, za katero bomo v tej iteraciji racunali nove vrednosti
		second_plate... plosca, iz katere bomo vzemali vrednosti za izracun novih vrednosti plosce 'first_plate'
	*/
	double **first_plate = alloc_plate(height, width);
	double **second_plate = alloc_plate(height, width);
	
	init_plate(first_plate, height, width);
	init_plate(second_plate, height, width);	
	
	double max_diff;
	
	for(int k = 0; k < iterations; k++)
	{
		max_diff = 0.0;
		
		// v vsaki (razen v robnih) tocki izracunaj novo temperaturo na podlagi starih
		for(int i = 1; i < height - 1; i++)
		{
			for(int j = 1; j < width - 1; j++)
			{
				first_plate[i][j] = calc_heat_point(second_plate, i, j);

				double curr_diff = fabs(first_plate[i][j] - second_plate[i][j]);

				if(curr_diff > max_diff)
					max_diff = curr_diff;
			}
		}
		
		swap_pointers(&first_plate, &second_plate);
		
#ifdef EPSILON
		
		if(max_diff < EPSILON)
			break;
		
#endif
		
	}
	
	// ne rabimo vec prve plosce
	free_plate(first_plate, height, width);
	
#ifndef TIME_MEASUREMENTS
	
	printf("Maximum heat difference calculated in the last iteration was %lf.\n", max_diff);
	
#endif
	
	return second_plate;
}
