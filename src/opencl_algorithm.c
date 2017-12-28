#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include "clerrors.h"

// ni ravno prenosljivo - prej pognan program, ki izpise lastnosti graficne kartice
#define MAX_WORKGROUP_SIZE 1024
#define MAX_SOURCE_SIZE	16384

float *alloc_plate_1d(int height, int width)
{
	float *plate = (float *) malloc(height * width * sizeof(float));
	return plate;
}

void init_plate_1d(float *plate, int height, int width)
{
	// robni pogoji: 3 stranice (zgornja, leva, desna) segrete na 100 stopinj, 1 stranica (spodnja) 0 stopinj	
	for(int i = 0; i < width; i++)
	{
		plate[i] = 100.0;
		plate[(height - 1) * width + i] = 0.0;
	}

	for(int i = 1; i < height - 1; i++)
	{
		plate[i * width] = 100.0;
		plate[i * width + width - 1] = 100.0;

		// sicer vzamemo povprecje (leva + povsem desna + zgornja + povsem sp) / 4
		for(int j = 1; j < width - 1; j++)
			plate[i * width + j] = (
					plate[i * width + j - 1] + 
					plate[i * width + width - 1] + 
					plate[(i - 1) * width + j] + 
					plate[(height - 1) * width + j]
				) / 4;
	}
}

float *driver_ocl(int height, int width, float epsilon)
{
	cl_int ret;

    // Branje datoteke
    FILE *fp;
    char *source_str;
    size_t source_size;

    fp = fopen("kernel.cl", "r");
    if (!fp) 
	{
		fprintf(stderr, "Error reading the file\n");
        exit(1);
    }
    source_str = (char *) malloc(MAX_SOURCE_SIZE);
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	source_str[source_size] = '\0';
    fclose(fp);

	float *first_plate = alloc_plate_1d(height, width);
	float *second_plate = alloc_plate_1d(height, width);

	/*
		Za shranjevanje delnih max temperaturnih razlik. S tem je implementirana
		redukcija -> namesto, da bi sli vedno na koncu iteracije cez cel array
		(height * width operacij), naredi graficna delne rezultate - max
		razlike za posamezno nit, mi pa na hostu najdemo max izmed teh delnih
		rezultatov (MAX_WORKGROUP_SIZE operacij).
	*/
	float *temp_max_diff = (float *) malloc(MAX_WORKGROUP_SIZE * sizeof(float));

	init_plate_1d(first_plate, height, width);
	init_plate_1d(second_plate, height, width);

	// Podatki o platformi
	cl_platform_id platform_id[10];
	cl_uint ret_num_platforms;

	ret = clGetPlatformIDs(10, platform_id, &ret_num_platforms);
	
	// Podatki o napravi
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
						(height * width) * sizeof(float), first_plate, &ret);

	printf("[Allocating (g_) first plate]: %s\n", getErrorString(ret));

	cl_mem g_second_plate = clCreateBuffer(context,
						CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
						(height * width) * sizeof(float), second_plate, &ret);

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

	    cl_mem *tmp = *(&curr_state);
	    *(&curr_state) = *(&prev_state);
	    *(&prev_state) = tmp;

   	    float *tmp_plate = *(&first_plate);
	    *(&first_plate) = *(&second_plate);
	    *(&second_plate) = tmp_plate;
	}

    // kopiranje rezultatov
	ret = clEnqueueReadBuffer(command_queue, *curr_state, CL_TRUE, 0,		
						  (height * width) * sizeof(float), first_plate, 0, NULL, NULL);

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
	free(first_plate);
	free(second_plate);
	free(temp_max_diff);

	/*
		TODO: popravi ta return value, v funkciji, ki bo to klicala, bo treba freejat
	 	vrnjen objekt
	*/
	return (float *)NULL;
}