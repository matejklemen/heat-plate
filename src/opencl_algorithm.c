#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include "clerrors.h"

// ni ravno prenosljivo - prej pognan program, ki izpise lastnosti graficne kartice
#define MAX_WORKGROUP_SIZE 1024
#define MAX_SOURCE_SIZE	16384

float *alloc_plate_1d(int height, int width)
{
	float *plate = (float *) malloc(height * width * sizeof(float));
	return plate;
}

void init_plate_1d(float *plate, int height, int width)
{
	// robni pogoji: 3 stranice (zgornja, leva, desna) segrete na 100 stopinj, 1 stranica (spodnja) 0 stopinj	
	for(int i = 0; i < width; i++)
	{
		plate[i] = 100.0;
		plate[(height - 1) * width + i] = 0.0;
	}

	for(int i = 1; i < height - 1; i++)
	{
		plate[i * width] = 100.0;
		plate[i * width + width - 1] = 100.0;

		// sicer vzamemo povprecje (leva + povsem desna + zgornja + povsem sp) / 4
		for(int j = 1; j < width - 1; j++)
			plate[i * width + j] = (
					plate[i * width + j - 1] + 
					plate[i * width + width - 1] + 
					plate[(i - 1) * width + j] + 
					plate[(height - 1) * width + j]
				) / 4;
	}
}