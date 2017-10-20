#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define EPSILON (double) 0.00001

// koliko iteracij rabimo, da stvar 'skonvergira'
int no_iterations = 0;

void print_plate(double **plate, int height, int width)
{
	for(int i = 0; i < height; i++) {
		for(int j = 0; j < width; j++)
			printf("[%.2lf] ", plate[i][j]);

		printf("\n");
	}
}

double **init_plate(double **plate, int height, int width)
{
	// robni pogoji: 3 stranice (zgornja, leva, desna) segrete na 100 stopinj, 1 stranica (spodnja) 0 stopinj
	for(int i = 0; i < width; i++) {
		plate[0][i] = 100.0; 
		plate[height - 1][i] = 0.0;
	}

	for(int i = 1; i < height - 1; i++) {
		plate[i][0] = 100.0;
		plate[i][width - 1] = 100.0;

		// sicer vzamemo povprecje (leva + povsem desna + zgornja + povsem sp) / 4
		for(int j = 1; j < width - 1; j++)
			plate[i][j] = (plate[i][j - 1] + plate[i][width - 1] + plate[i - 1][j] + plate[height - 1][j]) / 4;
	}

	return plate;

}

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

/*
	Izracuna novo toploto na tocki (x, y) plosce 'first' iz podatkov plosce 'second'.
	Funkcija predpostavlja, da x in y ne morata iti izven okvirov tabel 'first' in 'second' (=> to preglej v glavni funkciji).
	Vrne novo temperaturo.
*/
double calc_heat_point(double **second, int x, int y)
{
	return (second[y - 1][x] + second[y + 1][x] + second[y][x - 1] + second[y][x + 1]) / 4;
}

void swap_pointers(double ***first, double ***second)
{
	double **tmp = *first;
	*first = *second;
	*second = tmp;
}

double **calc_heat_plate(int height, int width)
{
	/*
		first_plate... plosca, za katero bomo v tej iteraciji racunali nove vrednosti
		second_plate... plosca, iz katere bomo vzemali vrednosti za izracun novih vrednosti plosce 'first_plate'
	*/
	double **first_plate = alloc_plate(height, width);
	double **second_plate = alloc_plate(height, width);

	first_plate = init_plate(first_plate, height, width);
	second_plate = init_plate(second_plate, height, width);	

	while(1) {
		double diff = 0.0;

		// v vsaki (razen v robnih) tocki izracunaj novo temperaturo na podlagi starih
		for(int i = 1; i < height - 1; i++) {
			for(int j = 1; j < width - 1; j++) {
				first_plate[i][j] = calc_heat_point(second_plate, j, i);

				double curr_diff = fabs(first_plate[i][j] - second_plate[i][j]);

				if(curr_diff > diff)
					diff = curr_diff;
			}
		}
		no_iterations++;

		if(diff < EPSILON)
			break;

		swap_pointers(&first_plate, &second_plate);
	}

	// ne rabimo vec druge plosce
	free_plate(second_plate, height, width);

	printf("Skonvergiralo v %d korakih.\n", no_iterations);

	return first_plate;
}

int main(int argc, char *argv[])
{
	int width, height;
	if(argc == 3) {
		height = strtol(argv[1], (char **)NULL, 10);
		width = strtol(argv[2], (char **)NULL, 10);

		width += 2; height += 2;

		double **solution_plate = calc_heat_plate(height, width);
		
		// TODO: visualize plate

		free_plate(solution_plate, height, width);
	}
	else
		printf("Usage: \"%s <height> <width>\"\n", argv[0]);

	return 0;
}