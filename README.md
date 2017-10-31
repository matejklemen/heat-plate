# temperaturna-plosca
Project in a Distributed systems course.

## Compiling
\[Linux\]```g++ main.c heat_plate.c visualization.c -o heat_plate.o `pkg-config --cflags --libs opencv` ```

## Libs
[OpenCv](https://opencv.org/) for visualization.

## Example
$ ./heat_plate.o 400 400 32000<br>
Maximum heat difference calculated in the last iteration was 0.000562.<br><br>
![400x400](https://github.com/matejklemen/temperaturna-plosca/blob/master/heat_plate_400_400_32000.png)

