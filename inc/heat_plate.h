#ifndef HEAT_PLATE_H
#define HEAT_PLATE_H

double **alloc_plate(int height, int width);
void free_plate(double **plate, int height, int width);
void init_plate(double **plate, int height, int width);
double calc_heat_point(double **plate, int y, int x);
void swap_pointers(double ***first, double ***second);
double **calc_heat_plate(int height, int width, int iterations);

#endif
