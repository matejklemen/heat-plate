#include "mpi.h"
#include "heat_plate.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);
	
	int id, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	int height = strtol(argv[1], (char **)NULL, 10);
	int width = strtol(argv[2], (char **)NULL, 10);
	float epsilon = strtof(argv[3], (char **)NULL);
	
	float** plate;
	
	if(id == 0)
	{
		
		plate = alloc_plate(height, width);
		init_plate(plate, height, width);
		
		// TO-DO: poslji dele plosce med ostale
	}
	
	// TO-DO: racunanje, redukcija za diff
	
	if(id == 0)
	{
		// TO-DO: zberi plosco nazaj v plate
		
		FILE* fp = fopen("plate.data", "wb");
		if(fp)
		{
		    fwrite(plate[0], sizeof(float), height * width, fp);
		}
		fclose(fp);
		
		free_plate(plate, height, width);
	}
	
	MPI_Finalize();
	
	return 0;
}
