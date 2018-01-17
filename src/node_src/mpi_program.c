#include "mpi.h"
#include "heat_plate.h"
#include <stdio.h>

int main(void)
{
	MPI_Init(NULL, NULL);
	
	int id, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	if(id == 0)
	{
		printf("%d\n", size);
		
	}
	
	MPI_Finalize();
	
	return 0;
}
