#include <stdio.h>
#include <stdlib.h>

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

double** alloc_plate(int height, int width)
{
	double** plate = (double **) malloc(sizeof(double*) * height);
	for(int i = 0; i < width; i++)
		plate[i] = (double*) malloc(sizeof(double) * width);

	return plate;
}

double **calc_heat_plate(int height, int width)
{
	double **first_plate = alloc_plate(height, width);
	double **second_plate = alloc_plate(height, width);

	double **curr_plate = first;

	curr_plate = init_plate(curr_plate, height, width);

	// TODO: free memory from first_plate and second_plate
	return NULL;
}

int main(int argc, char *argv[])
{
	int width, height;
	if(argc == 3) {
		height = strtol(argv[1], (char **)NULL, 10);
		width = strtol(argv[2], (char **)NULL, 10);

		width += 2; height += 2;

		calc_heat_plate(height, width);
	}
	else
		printf("Usage: \"%s <height> <width>\"\n", argv[0]);

	return 0;
}