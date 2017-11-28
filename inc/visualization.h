#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <cv.h>

IplImage *get_image(float **plate, int h, int w);
void show_image(IplImage *img);
void save_image(IplImage *img, const char *file_name);
void release_image(IplImage *img);

#endif
