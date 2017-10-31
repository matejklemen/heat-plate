#include "main.h"
#include "heat_plate.h"
#include "visualization.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	if(argc == 4)
	{
		// pretvori argumente v stevilske tipe
		int height = strtol(argv[1], (char **)NULL, 10) + 2;
		int width = strtol(argv[2], (char **)NULL, 10) + 2;
		int iterations = strtol(argv[3], (char **)NULL, 10);
		
		// izracunaj plosco
		double **solution_plate = calc_heat_plate(height, width, iterations);
		
		// pretvori plosco v sliko
		IplImage *img = get_image(solution_plate, height, width);
		
		// prikazi sliko
		show_image(img);
		
		// shrani sliko
		char file_name[64];
		sprintf(file_name, "heat_plate_%s_%s_%s.png", argv[1], argv[2], argv[3]);
		save_image(img, file_name);
		
		// sprosti pomnilnik zaseden s sliko
		release_image(img);
		
		// sprosti pomnilnik zaseden s plosco
		free_plate(solution_plate, height, width);
	}
	else
	{
		printf("Usage: \"%s <height> <width> <iterations>\"\n", argv[0]);
	}
	
	return 0;
}

