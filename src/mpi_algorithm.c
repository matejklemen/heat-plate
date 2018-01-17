#include "mpi_algorithm.h"
#include "heat_plate.h"
#include <stdio.h>
#include <stdlib.h>

#define NUM_PROCESSES 4

float **calc_heat_plate_mpi(int height, int width, float epsilon)
{
	char cmd[128];
	sprintf(cmd, "mpirun -np %d mpi_program.o %d %d %f", NUM_PROCESSES, height, width, epsilon);
	system(cmd);
	
	float **plate = alloc_plate(height, width);
	FILE* fp = fopen("plate.data", "rb");
	if(fp)
	{
	    fread(plate[0], sizeof(float), height * width, fp);
	}
	fclose(fp);
	system("rm plate.data");
	
	return plate;
}
