#ifndef HEAT_PLATE_H
#define HEAT_PLATE_H

float **alloc_plate(int height, int width);
void free_plate(float **plate, int height, int width);
void init_plate(float **plate, int height, int width);
float calc_heat_point(float **plate, int y, int x);
void swap_pointers(float ***first, float ***second);
float **calc_heat_plate(int height, int width, float epsilon);

#endif
