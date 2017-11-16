#include "main.h"
#include "heat_plate.h"
#include "heat_plate_pt.h"
#include "visualization.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_OF_SAMPLES 100

int main(int argc, char *argv[])
{
	/*
		Ce stevilo argumentov ne ustreza, izpisemo primer uporabe
		in koncamo program (vrnemo izhodni status 1).
	*/
	if(argc != 4)
	{
		printf("Usage: \"%s <height> <width> <iterations>\"\n", argv[0]);
		return 1;
	}
	
	int height = strtol(argv[1], (char **)NULL, 10) + 2;
	int width = strtol(argv[2], (char **)NULL, 10) + 2;
	int iterations = strtol(argv[3], (char **)NULL, 10);
	
	/*
		Ce ne merimo casa, enkrat izracunamo temperaturno plosco,
		jo pretvorimo v sliko, to pa prikazemo in shranimo v datoteko.
	*/
	if(!TIME_MEASUREMENT)
	{
		double **solution_plate = calc_heat_plate_pt(height, width, iterations);
		
		IplImage *img = get_image(solution_plate, height, width);
		
		show_image(img);
		
		char file_name[64];
		sprintf(file_name, "img/heat_plate_%s_%s_%s.png", argv[1], argv[2], argv[3]);
		save_image(img, file_name);
		
		release_image(img);
		
		free_plate(solution_plate, height, width);
	}
	
	/*
		Sicer, veckrat ponovimo izracun temperaturne plosce in pri vsaki
		ponovitvi izmerimo cas. Na koncu izpisemo izracunano povprecje
		in standardno napako.
	*/
	else
	{
		struct timespec start, stop;
		double samples[NUM_OF_SAMPLES];
		double avg = 0.0;
		double std_err = 0.0;
		
		for(int i = 0; i < NUM_OF_SAMPLES; i++)
		{
			clock_gettime(CLOCK_REALTIME, &start);
			
			double **solution_plate = calc_heat_plate_pt(height, width, iterations);
			free_plate(solution_plate, height, width);
			
			clock_gettime(CLOCK_REALTIME, &stop);
			
			samples[i] = (stop.tv_sec - start.tv_sec) + (stop.tv_nsec - start.tv_nsec) / 1000000000.0;
			avg += samples[i];
		}
		
		avg /= NUM_OF_SAMPLES;
		
		for(int i = 0; i < NUM_OF_SAMPLES; i++)
		{
			std_err += pow((avg - samples[i]), 2);
		}
		
		std_err = sqrt(std_err / ((NUM_OF_SAMPLES - 1) * NUM_OF_SAMPLES));
		
		printf("t = %lf s\nSE = %lf s\n", avg, std_err);
	}
	
	return 0;
}
