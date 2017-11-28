#include "visualization.h"
#include <highgui.h>
#include <math.h>

#define MAX_SIZE 800

/*
	Src: http://www.andrewnoske.com/wiki/Code_-_heatmaps_and_color_gradients
	Na naslove kazalcev zapise vrednosti med 0 in 1
*/
static void heat_to_color(float normalized_value, float *rval, float *gval, float *bval)
{
	const int NUM_COLORS = 6;
	static float color[6][3] = { {0,0,1}, {0,1,1}, {0,1,0}, {1,1,0}, {1,0.5,0}, {1,0,0} };
	int idx1, idx2;
	float fract_between = 0;
	
	if(normalized_value <= 0)
	{
		idx1 = idx2 = 0;
	}
	else if(normalized_value >= 1)
	{
		idx1 = idx2 = NUM_COLORS-1;
	}
	else
	{
		normalized_value = normalized_value * (NUM_COLORS - 1);
		idx1 = floor(normalized_value);
		idx2 = idx1 + 1;
		fract_between = normalized_value - (float)idx1;
	}
	
	*rval = (color[idx2][0] - color[idx1][0]) * fract_between + color[idx1][0];
	*gval = (color[idx2][1] - color[idx1][1]) * fract_between + color[idx1][1];
	*bval = (color[idx2][2] - color[idx1][2]) * fract_between + color[idx1][2];
}

IplImage *get_image(float **plate, int h, int w)
{
	IplImage *img = cvCreateImage(cvSize(w, h), 8, 3);
	
	// prepisi in pretvori vrednosti iz plosce v sliko
	for(int i = 0; i < h; i++)
	{
		for(int j = 0; j < w; j++)
		{
			float r, g, b;
			heat_to_color(plate[i][j] / 100, &r, &g, &b);
			
			img->imageData[i*img->widthStep + j*3 + 0] = b * 255;
			img->imageData[i*img->widthStep + j*3 + 1] = g * 255;
			img->imageData[i*img->widthStep + j*3 + 2] = r * 255;
		}
	}
	
	// ce najvecja od dimenzij presega MAX_SIZE, potem pomanjsaj sliko
	int size;
	if(h > w)
	{
		size = h;
	}
	else
	{
		size = w;
	}
	
	if(size > MAX_SIZE)
	{
		double k = (double) MAX_SIZE / size;
		int new_h = (int) round(k * img->height);
		int new_w = (int) round(k * img->width);
		IplImage *resized_img = cvCreateImage(cvSize(new_w, new_h), 8, 3);
		cvResize(img, resized_img);
		cvReleaseImage(&img);
		img = resized_img;
	}
	
	return img;
}

void show_image(IplImage *img)
{
	cvNamedWindow("Preview", CV_WINDOW_NORMAL);
	cvMoveWindow("Preview", 100, 100);
	cvResizeWindow("Preview", 500, 500);
	
	cvShowImage("Preview", img);
	
	cvWaitKey(0);
}

void save_image(IplImage *img, const char *file_name)
{
	cvSaveImage(file_name, img);
}

void release_image(IplImage *img)
{
	cvReleaseImage(&img);
}
