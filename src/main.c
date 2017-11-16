#include "main.h"
#include "heat_plate.h"
#include "heat_plate_pt.h"
#include "visualization.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

int main(int argc, char *argv[])
{
	
#ifdef EPSILON
	
	/*
		Ce je epsilon definiran, bo racunanje samodejno ustavljeno,
		zato stevila iteracij ne sprejmemo kot argument, ampak
		ga nastavimo na najvecje mozno stevilo.
	*/
	
	if(argc != 3)
	{
		printf("Usage: \"%s <height> <width>\"\n", argv[0]);
		return 1;
	}
	
	int height = strtol(argv[1], (char **)NULL, 10) + 2;
	int width = strtol(argv[2], (char **)NULL, 10) + 2;
	int iterations = INT_MAX;
	
#else
	
	/*
		Sicer, kot tretji argument zahtevamo tudi stevilo iteracij.
	*/
	
	if(argc != 4)
	{
		printf("Usage: \"%s <height> <width> <iterations>\"\n", argv[0]);
		return 1;
	}
	
	int height = strtol(argv[1], (char **)NULL, 10) + 2;
	int width = strtol(argv[2], (char **)NULL, 10) + 2;
	int iterations = strtol(argv[3], (char **)NULL, 10);
	
#endif

#ifndef TIME_MEASUREMENTS
	
	/*
		Ce ne merimo casa, enkrat izracunamo temperaturno plosco,
		jo pretvorimo v sliko, to pa prikazemo in shranimo v datoteko.
	*/
	
	double **solution_plate = calc_heat_plate_pt(height, width, iterations);
	
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
	double samples[TIME_MEASUREMENTS];
	double avg = 0.0;
	double std_err = 0.0;
	
	for(int i = 0; i < TIME_MEASUREMENTS; i++)
	{
		clock_gettime(CLOCK_REALTIME, &start);
		
		double **solution_plate = calc_heat_plate_pt(height, width, iterations);
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
