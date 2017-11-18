#include "heat_plate.h"
#include <stdlib.h>
#include "serial_algorithm.h"
#include "pthread_algorithm.h"

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

double **calc_heat_plate(int height, int width, double epsilon)
{
	//return calc_heat_plate_serial(height, width, epsilon);
	return calc_heat_plate_pthread(height, width, epsilon);
}
