# temperaturna-plosca
Project in a Distributed systems course.

## Compiling
\[Linux\]```g++ src/*.c -Iinc -fpermissive -lpthread `pkg-config --cflags --libs opencv` -o heat_plate.o```

## Libs
[OpenCv](https://opencv.org/) for visualization.

## Example
$ ./heat_plate.o 400 400 0.0005<br>
34955 iterations.<br><br>
![400x400](https://github.com/matejklemen/temperaturna-plosca/blob/master/img/heat_plate_400_400_0.0005.png)
