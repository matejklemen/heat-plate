#ifndef HEAT_PLATE_H
#define HEAT_PLATE_H

void free_plate(double **plate, int height, int width);
double **calc_heat_plate(int height, int width, int iterations);

#endif

