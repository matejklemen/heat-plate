# temperaturna-plosca
Project in a Distributed systems course.

## Compiling
\[Linux\]```gcc temp_plosca.c -o temp_plosca `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0` -lm```
\[Linux\]```g++ temp_plosca.c visualization_opencv.c `pkg-config --cflags --libs opencv````

## Libs
[Cairo](https://www.cairographics.org/) for visualization.
[OpenCv](https://opencv.org/) for visualization.

## Example
400 x 400, EPSILON = 0.005 <br /><br />
![400x400](https://github.com/matejklemen/temperaturna-plosca/blob/master/plosca.png)
