#include "opencl_algorithm.h"
#include "heat_plate.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>
#include "clerrors.h"

// ni ravno prenosljivo - prej pognan program, ki izpise lastnosti graficne kartice
#define MAX_WORKGROUP_SIZE 256
#define MAX_SOURCE_SIZE	16384

float **calc_heat_plate_opencl(int height, int width, float epsilon)
{
	// branje datoteke
	FILE *fp = fopen("src/kernel.cl", "r");
	if(!fp)
	{
		fprintf(stderr, "Error reading the file\n");
		exit(1);
	}
	char *source_str = (char *) malloc(MAX_SOURCE_SIZE);
	size_t source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	source_str[source_size] = '\0';
	fclose(fp);
	
	// podatki o platformi in napravah
	cl_platform_id platform_id[10];
	cl_uint ret_num_platforms;
	clGetPlatformIDs(10, platform_id, &ret_num_platforms);
	cl_device_id device_id[10];
	cl_uint ret_num_devices;
	clGetDeviceIDs(platform_id[0], CL_DEVICE_TYPE_GPU, 10, device_id, &ret_num_devices);
	
	// kontekst, ukazna vrsta, program, prevajanje, scepec
	cl_context context = clCreateContext(NULL, 1, &device_id[0], NULL, NULL, NULL);
	cl_command_queue command_queue = clCreateCommandQueue(context, device_id[0], 0, NULL);
	cl_program program = clCreateProgramWithSource(context,	1, (const char **)&source_str, NULL, NULL);
	clBuildProgram(program, 1, &device_id[0], NULL, NULL, NULL);
	cl_kernel kernel = clCreateKernel(program, "calculate_point", NULL);
	
	/*
	// log
	size_t build_log_len;
	char *build_log;
	clGetProgramBuildInfo(program, device_id[0], CL_PROGRAM_BUILD_LOG, 0, NULL, &build_log_len);
	build_log = (char *) malloc(sizeof(char) * (build_log_len + 1));
	clGetProgramBuildInfo(program, device_id[0], CL_PROGRAM_BUILD_LOG, build_log_len, build_log, NULL);
	printf("%s\n", build_log);
	free(build_log);
	*/
	
	// inicializacija plosce in priprava pomnilnika (za dve plosci) na graficni kartici
	float **plate = alloc_plate(height, width);
	init_plate(plate, height, width);
	
	cl_mem g_first_plate = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, height * width * sizeof(float), plate[0], NULL);
	cl_mem *g_first_plate_ptr = &g_first_plate;
	
	cl_mem g_second_plate = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, height * width * sizeof(float), plate[0], NULL);
	cl_mem *g_second_plate_ptr = &g_second_plate;
	
	// inicializacija zastavice za konec iteracij in priprava pomnilnika na graficni kartici
	int all_difs_below_eps = 1;
	cl_mem g_all_difs_below_eps = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int), &all_difs_below_eps, NULL);
	
	// obseg dela
	size_t local_item_size = MAX_WORKGROUP_SIZE;
	size_t global_item_size = ((height * width - 1) / local_item_size + 1) * local_item_size;
	
	int iterations = 0;
	
	while(1)
	{
		// nastavimo zacetno vrednost zastavice in jo prenesemo v pomnilnik graficne kartice
		all_difs_below_eps = 1;
		clEnqueueWriteBuffer(command_queue, g_all_difs_below_eps, CL_TRUE, 0, sizeof(int), (void *)&all_difs_below_eps, 0, NULL, NULL);
		
		// nastavimo argumente in pozenemo racunanje ene iteracije na graficni kartici
		clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)g_first_plate_ptr);
		clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)g_second_plate_ptr);
		clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&g_all_difs_below_eps);
		clSetKernelArg(kernel, 3, sizeof(cl_int), (void *)&height);
		clSetKernelArg(kernel, 4, sizeof(cl_int), (void *)&width);
		clSetKernelArg(kernel, 5, sizeof(cl_float), (void *)&epsilon);
		
		clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
		
		// zamenjamo plosci in povecamo stevec iteracij
		cl_mem *tmp = g_first_plate_ptr;
		g_first_plate_ptr = g_second_plate_ptr;
		g_second_plate_ptr = tmp;
		
		iterations++;
		
		// preberemo zastavico iz pomnilnika graficne kartice in po potrebi zakljucimo iteracije
		clEnqueueReadBuffer(command_queue, g_all_difs_below_eps, CL_TRUE, 0, sizeof(int), (void *)&all_difs_below_eps, 0, NULL, NULL);
		
		if(all_difs_below_eps)
			break;
	}
	
	// kopiranje rezultatov iz graficne kartice
	clEnqueueReadBuffer(command_queue, g_second_plate, CL_TRUE, 0, height * width * sizeof(float), plate[0], 0, NULL, NULL);
	
	// brisanje podatkov graficne kartice
	clFlush(command_queue);
	clFinish(command_queue);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseMemObject(g_first_plate);
	clReleaseMemObject(g_second_plate);
	clReleaseMemObject(g_all_difs_below_eps);
	clReleaseCommandQueue(command_queue);
	clReleaseContext(context);
	
#ifndef TIME_MEASUREMENTS
	
	printf("%d iterations.\n", iterations);
	
#endif
	
	return plate;
}
