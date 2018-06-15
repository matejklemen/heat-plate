# heat-plate
Authors: Rok Grmek, Matej Klemen

## What is this?
This repository contains the code for various implementations of a program that calculates the heat distribution on a heat plate (under certain assumptions):
- sequential algorithm and
- four parallel implementations (Pthreads, OpenMP, OpenCL and OpenMPI).

The whole project was made during a Distributed systems course in 2017. It is mainly written in C (and also a bit of C++ for visualizing the calculated distribution).

## Benchmarks
During the benchmark we tried to ensure there was as little overhead from other programs as possible (to minimize non-necessary context switching). We have measured the average run time of 100 runs and the standard error of these 100 measurements in relation to the size of the heat plate (heat plate was of size 'width' x 'width'. We only mention speedups (relative to the sequential algorithm) here because it tells more than just run time.

##### Parallel algorithm (Pthreads) 
max around 1.8x speedup
##### Parallel algorithm (OpenMP) 
max around 1.8x speedup
##### Parallel algorithm (OpenCL) 
max around 17x speedup (!)
##### Parallel algorithm (MPI) 
max around 1.8x speedup

## Libraries
Visualization: [OpenCV](https://opencv.org/)
Parallelization: [Pthreads](https://en.wikipedia.org/wiki/POSIX_Threads), [OpenMP](http://www.openmp.org/), [OpenCL](https://en.wikipedia.org/wiki/OpenCL), [OpenMPI](https://www.open-mpi.org/)

## Compilation
\[Linux\]```mpic++ src/node_src/mpi_program.c -o mpi_program.o && g++ src/*.c -Iinc -o heat_plate.o `pkg-config --cflags --libs opencv` -lpthread -fpermissive -fopenmp -lOpenCL```

## Example
$ ./heat_plate.o 400 400 0.0005
34955 iterations.

![400x400](https://github.com/matejklemen/temperaturna-plosca/blob/master/img/heat_plate_400_400_0.0005.png)
