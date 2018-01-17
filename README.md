# temperaturna-plosca
Project in a Distributed systems course.

## Compiling
\[Linux\]```mpic++ src/node_src/mpi_program.c src/heat_plate.c src/mpi_algorithm.c -Iinc -o mpi_program.o && g++ src/*.c -Iinc -o heat_plate.o `pkg-config --cflags --libs opencv` -lpthread -fpermissive -fopenmp -lOpenCL```

## Libs
Visualization: [OpenCV](https://opencv.org/)<br>
Parallelization: [Pthreads](https://en.wikipedia.org/wiki/POSIX_Threads), [OpenMP](http://www.openmp.org/), [OpenCL](https://en.wikipedia.org/wiki/OpenCL), [MPI](https://www.open-mpi.org/)

## Example
$ ./heat_plate.o 400 400 0.0005<br>
34955 iterations.<br><br>
![400x400](https://github.com/matejklemen/temperaturna-plosca/blob/master/img/heat_plate_400_400_0.0005.png)
