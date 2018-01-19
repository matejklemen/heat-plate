#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*
	Vse dodatne funkcije (alloc_plate, free_plate, init_plate, calc_heat_point,
	swap_pointers) so kopirane iz datoteke heat_plate.c zaradi lažjega prevajanja
	ene same datoteke na slingu.
*/

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
	
	MPI_Group world_group;
	MPI_Comm_group(MPI_COMM_WORLD, &world_group);
	
	MPI_Group new_group;
	int ranges[3] = {0, n*n - 1, 1};
	MPI_Group_range_incl(world_group, 1, &ranges, &new_group);
	
	MPI_Comm new_world;
	MPI_Comm_create(MPI_COMM_WORLD, new_group, &new_world);
	
	if(new_world == MPI_COMM_NULL)
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
		// najprej vsak proces izracuna svoj del plosce in zabelezi lokalno maksimalno razliko
		float local_max_diff = 0.0;
		
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
		
		// procesi opravijo skupno redukcijo (ukaz sluzi tudi kot prepreka)
		float global_max_diff;
		MPI_Allreduce(&local_max_diff, &global_max_diff, 1, MPI_FLOAT, MPI_MAX, new_world);
		
		// zamenjamo plosci in povecamo stevec iteracij
		swap_pointers(&first_plate, &second_plate);
		iterations++;
		
		// ce je globalna maksimalna razlika dovolj majhna, koncamo z iteracijami
		if(global_max_diff < epsilon)
			break;
		
		// posiljanje robov (zgornji, spodnji, lev, desen)
		MPI_Request r;
		
		if(id / n > 0)
		{
			MPI_Isend(&second_plate[y_start][x_start], x_end - x_start, MPI_FLOAT, id - n, 0, new_world, &r);
		}
		if(id / n < n - 1)
		{
			MPI_Isend(&second_plate[y_end - 1][x_start], x_end - x_start, MPI_FLOAT, id + n, 0, new_world, &r);
		}
		if(id % n > 0)
		{
			float left_col_send[y_end - y_start];
			for(int i = y_start; i < y_end; i++)
				left_col_send[i - y_start] = second_plate[i][x_start];
			MPI_Isend(left_col_send, y_end - y_start, MPI_FLOAT, id - 1, 0, new_world, &r);
		}
		if(id % n < n - 1)
		{
			float right_col_send[y_end - y_start];
			for(int i = y_start; i < y_end; i++)
				right_col_send[i - y_start] = second_plate[i][x_end - 1];
			MPI_Isend(right_col_send, y_end - y_start, MPI_FLOAT, id + 1, 0, new_world, &r);
		}
		
		// prejemanje robov (zgornji, spodnji, lev, desen)
		if(id / n > 0)
		{
			MPI_Recv(&second_plate[y_start - 1][x_start], x_end - x_start, MPI_FLOAT, id - n, MPI_ANY_TAG, new_world, MPI_STATUS_IGNORE);
		}
		if(id / n < n - 1)
		{
			MPI_Recv(&second_plate[y_end][x_start], x_end - x_start, MPI_FLOAT, id + n, MPI_ANY_TAG, new_world, MPI_STATUS_IGNORE);
		}
		if(id % n > 0)
		{
			float left_col_recv[y_end - y_start];
			MPI_Recv(left_col_recv, y_end - y_start, MPI_FLOAT, id - 1, MPI_ANY_TAG, new_world, MPI_STATUS_IGNORE);
			for(int i = y_start; i < y_end; i++)
				second_plate[i][x_start - 1] = left_col_recv[i - y_start];
		}
		if(id % n < n - 1)
		{
			float right_col_recv[y_end - y_start];
			MPI_Recv(right_col_recv, y_end - y_start, MPI_FLOAT, id + 1, MPI_ANY_TAG, new_world, MPI_STATUS_IGNORE);
			for(int i = y_start; i < y_end; i++)
				second_plate[i][x_end] = right_col_recv[i - y_start];
		}
	}
	
	// glavni proces na koncu prejema posamezne dele plosce od vseh ostalih procesov
	if(id == 0)
	{
		for(int p = 1; p < n*n; p++)
		{
			// za vsak proces se izracuna njegove meje racunanja
			x_start = 1 + (int)((double)(p % n) * (width - 2) / n);
			x_end = 1 + (int)((double)((p % n) + 1) * (width - 2) / n);
			y_start = 1 + (int)((double)(p / n) * (height - 2) / n);
			y_end = 1 + (int)((double)((p / n) + 1) * (height - 2) / n);
			
			// podatke prejemamo po vrsticah
			for(int i = y_start; i < y_end; i++)
			{
				MPI_Recv(&second_plate[i][x_start], x_end - x_start, MPI_FLOAT, p, i, new_world, MPI_STATUS_IGNORE);
			}
		}
		
		// celotno plosco zapisemo v datoteko
		FILE* fp = fopen("plate.data", "wb");
		if(fp)
		{
		    fwrite(second_plate[0], sizeof(float), height * width, fp);
		}
		fclose(fp);
		
		//printf("%d iterations.\n", iterations);
	}
	
	// vsi procesi razen glavnega, posiljajo svoj del glavnemu procesu
	else
	{
		for(int i = y_start; i < y_end; i++)
		{
			MPI_Send(&second_plate[i][x_start], x_end - x_start, MPI_FLOAT, 0, i, new_world);
		}
	}
	
	// obe plosci lahko v tem trenutku sprostimo
	free_plate(first_plate, height, width);
	free_plate(second_plate, height, width);
	
	MPI_Finalize();
	
	return 0;
}
