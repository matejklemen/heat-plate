#include "heat_plate.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double **alloc_plate(int height, int width)
{
	double **plate = (double **) malloc(sizeof(double *) * height);
	for(int i = 0; i < height; i++)
		plate[i] = (double *) malloc(sizeof(double) * width);
	
	return plate;
}

void free_plate(double **plate, int height, int width)
{
	for(int i = 0; i < height; i++)
		free(plate[i]);
	
	free(plate);
}

void init_plate(double **plate, int height, int width)
{
	// robni pogoji: 3 stranice (zgornja, leva, desna) segrete na 100 stopinj, 1 stranica (spodnja) 0 stopinj
	for(int i = 0; i < width; i++)
	{
		plate[0][i] = 100.0; 
		plate[height - 1][i] = 0.0;
	}
	
	for(int i = 1; i < height - 1; i++)
	{
		plate[i][0] = 100.0;
		plate[i][width - 1] = 100.0;
		
		// sicer vzamemo povprecje (leva + povsem desna + zgornja + povsem sp) / 4
		for(int j = 1; j < width - 1; j++)
			plate[i][j] = (plate[i][j - 1] + plate[i][width - 1] + plate[i - 1][j] + plate[height - 1][j]) / 4;
	}
}

/*
	Izracuna novo toploto na tocki (x, y) plosce 'first' iz podatkov plosce 'second'.
	Funkcija predpostavlja, da x in y ne morata iti izven okvirov tabel 'first' in 'second' (=> to preglej v glavni funkciji).
	Vrne novo temperaturo.
*/
double calc_heat_point(double **plate, int y, int x)
{
	return (plate[y - 1][x] + plate[y + 1][x] + plate[y][x - 1] + plate[y][x + 1]) / 4;
}

void swap_pointers(double ***first, double ***second)
{
	double **tmp = *first;
	*first = *second;
	*second = tmp;
}

double **calc_heat_plate(int height, int width, int iterations)
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
		
		/*
		if(max_diff < EPS)
			break;
		*/
	}
	
	// ne rabimo vec prve plosce
	free_plate(first_plate, height, width);
	
	if(!TIME_MEASUREMENT)
	{
		printf("Maximum heat difference calculated in the last iteration was %lf.\n", max_diff);
	}
	
	return second_plate;
}
