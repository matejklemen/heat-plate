# temperaturna-plosca
Project in a Distributed systems course.

## Compiling
gcc temp_plosca.c -o temp_plosca `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0` -lm \[Linux\]

## Libs
[Cairo](https://www.cairographics.org/) for visualization.

## Example
400 x 400, EPSILON = 0.005
![400x400](https://github.com/matejklemen/temperaturna-plosca/blob/master/plosca.png)
