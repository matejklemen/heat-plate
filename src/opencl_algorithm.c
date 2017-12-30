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
	cl_int ret;
	
	// branje datoteke
	FILE *fp;
	char *source_str;
	size_t source_size;
	
	fp = fopen("src/kernel.cl", "r");
	if (!fp) 
	{
		fprintf(stderr, "Error reading the file\n");
		exit(1);
	}
	source_str = (char *) malloc(MAX_SOURCE_SIZE);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	source_str[source_size] = '\0';
	fclose(fp);
	
	// inicializacija plosc
	float **first_plate = alloc_plate(height, width);
	float **second_plate = alloc_plate(height, width);
	
	init_plate(first_plate, height, width);
	init_plate(second_plate, height, width);
	
	/*
		Za shranjevanje delnih max temperaturnih razlik. S tem je implementirana
		redukcija -> namesto, da bi sli vedno na koncu iteracije cez cel array
		(height * width operacij), naredi graficna delne rezultate - max
		razlike za posamezno nit, mi pa na hostu najdemo max izmed teh delnih
		rezultatov (MAX_WORKGROUP_SIZE operacij).
	*/
	float *temp_max_diff = (float *) malloc(MAX_WORKGROUP_SIZE * sizeof(float));
	
	// podatki o platformi
	cl_platform_id platform_id[10];
	cl_uint ret_num_platforms;
	
	ret = clGetPlatformIDs(10, platform_id, &ret_num_platforms);
	
	// podatki o napravi
	cl_device_id device_id[10];
	cl_uint ret_num_devices;
	
	ret = clGetDeviceIDs(platform_id[0], CL_DEVICE_TYPE_GPU, 10,	
	                     device_id, &ret_num_devices);
	
	printf("[Getting device IDs] %s\n", getErrorString(ret));
	
	/*
		kontekst - okolje za izvajanje scepca, upravljanje s pomnilnikom,...
		(NULL = privzeti kontekst)
	*/
	cl_context context = clCreateContext(NULL, 1, &device_id[0], NULL, NULL, &ret);
	
	printf("[Creating context]: %s\n", getErrorString(ret));
 	
	// ukazna vrsta
	cl_command_queue command_queue = clCreateCommandQueue(context, device_id[0], 0, &ret);											
	
	printf("[Creating command queue]: %s\n", getErrorString(ret));
	
	// obseg dela, ki ga bo delala ena skupina
	size_t local_item_size = MAX_WORKGROUP_SIZE;
	
	size_t num_groups = (height * width - 1) / local_item_size + 1;
	size_t global_item_size = num_groups * local_item_size;
	
	printf("Number of groups: %lu, size of group: %lu\n", num_groups, local_item_size);
	
	/*
		Alokacija pomnilnika na graficni kartici.
		Oznaceno z "g_", ker se podatki nahajajo na graficni kartici.
	*/	
	cl_mem g_first_plate = clCreateBuffer(context,
	                                      CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
	                                      height * width * sizeof(float), first_plate[0], &ret);
	
	printf("[Allocating (g_) first plate]: %s\n", getErrorString(ret));
	
	cl_mem g_second_plate = clCreateBuffer(context,
	                                       CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
	                                       height * width * sizeof(float), second_plate[0], &ret);
	
	printf("[Allocating (g_) second plate]: %s\n", getErrorString(ret));
	
	cl_mem g_temp_max_diff = clCreateBuffer(context,
	                                        CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
	                                        MAX_WORKGROUP_SIZE * sizeof(float), temp_max_diff, &ret);
	
	printf("[Allocating (g_) temp_max_diff]: %s\n", getErrorString(ret));
	
	// priprava programa
	cl_program program = clCreateProgramWithSource(context,	1,
	                                               (const char **)&source_str, NULL, &ret);
	
	printf("[Preparing program]: %s\n", getErrorString(ret));
	
	// prevajanje
	ret = clBuildProgram(program, 1, &device_id[0], NULL, NULL, NULL);
	
	// log
	size_t build_log_len;
	char *build_log;
	ret = clGetProgramBuildInfo(program, device_id[0], CL_PROGRAM_BUILD_LOG, 
	                            0, NULL, &build_log_len);
	
	build_log =(char *)malloc(sizeof(char)*(build_log_len+1));
	ret = clGetProgramBuildInfo(program, device_id[0], CL_PROGRAM_BUILD_LOG, 
	                            build_log_len, build_log, NULL);
	printf("%s\n", build_log);
	free(build_log);
	
	// scepec: priprava objekta
	cl_kernel kernel = clCreateKernel(program, "calculate_point", &ret);
	
	size_t buf_size_t;
	clGetKernelWorkGroupInfo(kernel, device_id[0], CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,sizeof(buf_size_t), &buf_size_t, NULL);
	printf("veckratnik niti = %lu\n", buf_size_t);
	
	cl_mem *curr_state = &g_first_plate;
	cl_mem *prev_state = &g_second_plate;
	
	int num_iterations = 0;
	
	/*
		1. Izracunamo novo stanje in delne max temperaturne razlike (na graficni).
		2. Izracunamo max temperaturno razliko v trenutni iteraciji (na hostu).
		3. Pogledamo, ce je max temperaturna razlika < epsilon.
	*/
	while(1)
	{
		num_iterations++;
		float curr_max = 0;
		
		// scepec: argumenti
		ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)curr_state);
		ret |= clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)prev_state);
		ret |= clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&g_temp_max_diff);
		ret |= clSetKernelArg(kernel, 3, sizeof(cl_int), (void *)&height);
		ret |= clSetKernelArg(kernel, 4, sizeof(cl_int), (void *)&width);
		
		// scepec: zagon
		ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL,
		                             &global_item_size, &local_item_size, 0, NULL, NULL);
		
		// kopiranje maksimimalnih razlik v temperaturi
		ret = clEnqueueReadBuffer(command_queue, g_temp_max_diff, CL_TRUE, 0,		
		                          MAX_WORKGROUP_SIZE * sizeof(float), temp_max_diff, 0, NULL, NULL);	    
		
		printf("[Copying results]: %s\n", getErrorString(ret));
		
		// redukcija (max)
		for(int i = 0; i < MAX_WORKGROUP_SIZE; i++)
		{
			if(temp_max_diff[i] > curr_max)
				curr_max = temp_max_diff[i];
		}
		
		printf("Current max diff is %.3f\n", curr_max);
		
		if(curr_max < epsilon)
			break;
		
		cl_mem *tmp = curr_state;
		curr_state = prev_state;
		prev_state = tmp;
	}
	
	// kopiranje rezultatov
	ret = clEnqueueReadBuffer(command_queue, *prev_state, CL_TRUE, 0,		
	                          height * width * sizeof(float), second_plate[0], 0, NULL, NULL);
	
	// brisanje podatkov graficne kartice
	ret = clFlush(command_queue);
	ret = clFinish(command_queue);
	ret = clReleaseKernel(kernel);
	ret = clReleaseProgram(program);
	ret = clReleaseMemObject(g_first_plate);
	ret = clReleaseMemObject(g_second_plate);
	ret = clReleaseCommandQueue(command_queue);
	ret = clReleaseContext(context);
	
	// brisanje podatkov na hostu
	free_plate(first_plate, height, width);
	free(temp_max_diff);
	
	return second_plate;
}
