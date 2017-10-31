#include "heat_plate.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static double **alloc_plate(int height, int width)
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

static void init_plate(double **plate, int height, int width)
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
static double calc_heat_point(double **plate, int y, int x)
{
	return (plate[y - 1][x] + plate[y + 1][x] + plate[y][x - 1] + plate[y][x + 1]) / 4;
}

static void swap_pointers(double ***first, double ***second)
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
	
	for(int i = 0; i < iterations; i++)
	{
		max_diff = 0.0;
		
		// v vsaki (razen v robnih) tocki izracunaj novo temperaturo na podlagi starih
		for(int j = 1; j < height - 1; j++)
		{
			for(int k = 1; k < width - 1; k++)
			{
				first_plate[j][k] = calc_heat_point(second_plate, j, k);

				double curr_diff = fabs(first_plate[j][k] - second_plate[j][k]);

				if(curr_diff > max_diff)
					max_diff = curr_diff;
			}
		}
		
		swap_pointers(&first_plate, &second_plate);
	}
	
	// ne rabimo vec druge plosce
	free_plate(second_plate, height, width);
	
	if(!TIME_MEASUREMENT)
	{
		printf("Maximum heat difference calculated in the last iteration was %lf.\n", max_diff);
	}
	
	return first_plate;
}

void print_plate(double **plate, int height, int width)
{
	for(int i = 0; i < height; i++)
	{
		for(int j = 0; j < width; j++)
			printf("[%.2lf] ", plate[i][j]);
			
		printf("\n");
	}
}

