#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

float **alloc_plate(int height, int width)
{
	float *linear_buff = (float *) malloc(sizeof(float) * height * width);
	
	float **plate = (float **) malloc(sizeof(float *) * height);
	for(int i = 0; i < height; i++)
		plate[i] = linear_buff + (i * width);
	
	return plate;
}

void free_plate(float **plate, int height, int width)
{
	free(plate[0]);
	free(plate);
}

void init_plate(float **plate, int height, int width)
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
float calc_heat_point(float **plate, int y, int x)
{
	return (plate[y - 1][x] + plate[y + 1][x] + plate[y][x - 1] + plate[y][x + 1]) / 4;
}

void swap_pointers(float ***first, float ***second)
{
	float **tmp = *first;
	*first = *second;
	*second = tmp;
}

int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);
	
	// argumenti
	int height = strtol(argv[1], (char **)NULL, 10) + 2;
	int width = strtol(argv[2], (char **)NULL, 10) + 2;
	float epsilon = strtof(argv[3], (char **)NULL);
	
	// id procesa in stevilo vseh procesov
	int id, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	// zracunamo stevilo procesov na stranico in zakljucimo odvecne procese
	int n = floor(sqrt(size));
	if(id >= n*n)
	{
		MPI_Finalize();
		return 0;
	}
	
	// vsak proces si pripravi dve celotni plosci (racuna pa le na svojem delu)
	float **first_plate = alloc_plate(height, width);
	float **second_plate = alloc_plate(height, width);
	init_plate(first_plate, height, width);
	init_plate(second_plate, height, width);
	
	// meje za racunanje
	int x_start = 1 + (int)((double)(id % n) * (width - 2) / n);
	int x_end = 1 + (int)((double)((id % n) + 1) * (width - 2) / n);
	int y_start = 1 + (int)((double)(id / n) * (height - 2) / n);
	int y_end = 1 + (int)((double)((id / n) + 1) * (height - 2) / n);
	
	int iterations = 0;
	
	while(1)
	{
		float local_max_diff = 0.0;
		float global_max_diff = 0.0;
		
		for(int i = y_start; i < y_end; i++)
		{
			for(int j = x_start; j < x_end; j++)
			{
				first_plate[i][j] = calc_heat_point(second_plate, i, j);
				
				float curr_diff = fabs(first_plate[i][j] - second_plate[i][j]);
				
				if(curr_diff > local_max_diff)
					local_max_diff = curr_diff;
			}
		}
		
		swap_pointers(&first_plate, &second_plate);
		iterations++;
		
		MPI_Allreduce(&local_max_diff, &global_max_diff, 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD);
		
		if(global_max_diff < epsilon)
			break;
		
		// TO-DO: izmenjaj robove od second plate
		
		// TO-DO: mogoce barrier? (odvisno kasno bo posiljanje)
	}
	
	if(id == 0)
	{
		for(int p = 1; p < n*n; p++)
		{
			x_start = 1 + (int)((double)(p % n) * (width - 2) / n);
			x_end = 1 + (int)((double)((p % n) + 1) * (width - 2) / n);
			y_start = 1 + (int)((double)(p / n) * (height - 2) / n);
			y_end = 1 + (int)((double)((p / n) + 1) * (height - 2) / n);
			for(int i = y_start; i < y_end; i++)
			{
				MPI_Recv(&second_plate[i][x_start], x_end - x_start, MPI_FLOAT, p, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
		}
		
		FILE* fp = fopen("plate.data", "wb");
		if(fp)
		{
		    fwrite(second_plate[0], sizeof(float), height * width, fp);
		}
		fclose(fp);
	}
	else
	{
		for(int i = y_start; i < y_end; i++)
		{
			MPI_Send(&second_plate[i][x_start], x_end - x_start, MPI_FLOAT, 0, i, MPI_COMM_WORLD);
		}
	}
	
	MPI_Barrier(MPI_COMM_WORLD);
	
	free_plate(first_plate, height, width);
	free_plate(second_plate, height, width);
	
	MPI_Finalize();
	
	return 0;
}
