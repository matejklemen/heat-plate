#include "main.h"
#include "heat_plate.h"
#include "visualization.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

int main(int argc, char *argv[])
{
 	/*
		Ce stevilo argumentov ne ustreza, izpisemo primer uporabe
		in koncamo program (vrnemo izhodni status 1).
 	*/
	
	if(argc != 4)
	{
		printf("Usage: \"%s <height> <width> <epsilon>\"\n", argv[0]);
		return 1;
	}
	
	int height = strtol(argv[1], (char **)NULL, 10) + 2;
	int width = strtol(argv[2], (char **)NULL, 10) + 2;
	float epsilon = strtof(argv[3], (char **)NULL);

#ifndef TIME_MEASUREMENTS
	
	/*
		Ce ne merimo casa, enkrat izracunamo temperaturno plosco,
		jo pretvorimo v sliko, to pa prikazemo in shranimo v datoteko.
	*/
	
	float **solution_plate = calc_heat_plate(height, width, epsilon);
	
	IplImage *img = get_image(solution_plate, height, width);
	
	show_image(img);
	
	char file_name[64];
	sprintf(file_name, "img/heat_plate_%s_%s_%s.png", argv[1], argv[2], argv[3]);
	save_image(img, file_name);
	
	release_image(img);
	
	free_plate(solution_plate, height, width);
	
#else
	
	/*
		Sicer, veckrat ponovimo izracun temperaturne plosce in pri vsaki
		ponovitvi izmerimo cas. Na koncu izpisemo izracunano povprecje
		in standardno napako.
	*/
	
	struct timespec start, stop;
	float samples[TIME_MEASUREMENTS];
	float avg = 0.0;
	float std_err = 0.0;
	
	for(int i = 0; i < TIME_MEASUREMENTS; i++)
	{
		clock_gettime(CLOCK_REALTIME, &start);
		
		float **solution_plate = calc_heat_plate(height, width, epsilon);
		free_plate(solution_plate, height, width);
		
		clock_gettime(CLOCK_REALTIME, &stop);
		
		samples[i] = (stop.tv_sec - start.tv_sec) + (stop.tv_nsec - start.tv_nsec) / 1000000000.0;
		avg += samples[i];
	}
	
	avg /= TIME_MEASUREMENTS;
	
	for(int i = 0; i < TIME_MEASUREMENTS; i++)
	{
		std_err += pow((avg - samples[i]), 2);
	}
	
	std_err = sqrt(std_err / ((TIME_MEASUREMENTS - 1) * TIME_MEASUREMENTS));
	
	printf("t = %lf s\nSE = %lf s\n", avg, std_err);
	
#endif
	
	return 0;
	
}
