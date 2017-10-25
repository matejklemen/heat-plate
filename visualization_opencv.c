#include "visualization_opencv.h"
#include <highgui.h>
#include <math.h>

#define MAX_SIZE 1000

IplImage *get_image(double **plate, int h, int w)
{
    IplImage *img = cvCreateImage(cvSize(w, h), 8, 3);
    
    // prepisi in pretvori vrednosti iz plosce v sliko
    for(int i=0; i<h; i++)
    {
        for(int j=0; j<w; j++)
        {
            if(plate[i][j] < 33)
            {
                int val = (int) round((plate[i][j] - 0) / 33 * 255);
                img->imageData[i*img->widthStep + j*3 + 0] = 255;       // blue
                img->imageData[i*img->widthStep + j*3 + 1] = val;       // green
                img->imageData[i*img->widthStep + j*3 + 2] = 0;         // red
            }
            else if(plate[i][j] < 66)
            {
                int val = (int) round((plate[i][j] - 33) / 33 * 255);
                img->imageData[i*img->widthStep + j*3 + 0] = 255 - val; // blue
                img->imageData[i*img->widthStep + j*3 + 1] = 255;       // green
                img->imageData[i*img->widthStep + j*3 + 2] = val;       // red
            }
            else
            {
                int val = (int) round((plate[i][j] - 66) / 34 * 255);
                img->imageData[i*img->widthStep + j*3 + 0] = 0;         // blue
                img->imageData[i*img->widthStep + j*3 + 1] = 255 - val; // green
                img->imageData[i*img->widthStep + j*3 + 2] = 255;       // red
            }
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

