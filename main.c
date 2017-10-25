#include "main.h"
#include "heat_plate.h"
#include "visualization.h"
#include <stdio.h>
#include <stdlib.h>

#define EPS 0.1

int main(int argc, char *argv[])
{
	int width, height;
	
	if(argc == 3)
	{
		height = strtol(argv[1], (char **)NULL, 10);
		width = strtol(argv[2], (char **)NULL, 10);
		
		width += 2;
		height += 2;
		
		double **solution_plate = calc_heat_plate(height, width, EPS);
		
		IplImage *img = get_image(solution_plate, height, width);
		show_image(img);
		save_image(img, "plosca.png");
		release_image(img);
		
		free_plate(solution_plate, height, width);
	}
	else
	{
		printf("Usage: \"%s <height> <width>\"\n", argv[0]);
	}
	
	return 0;
}

