__kernel void calculate_point(__global float *first_plate,
                              __global float *second_plate,
                              __global int *all_difs_below_eps,
                              int height,
                              int width,
                              float eps)
{
	int id = get_global_id(0);
	
	if(id > width && (id + 1) % width > 1 && id < (height - 1) * width)
	{
		float curr_temp = (second_plate[id - width] +
		                   second_plate[id - 1] +
		                   second_plate[id + 1] +
		                   second_plate[id + width]) / 4;
		
		first_plate[id] = curr_temp;
		
		if(fabs(curr_temp - second_plate[id]) > eps)
			*all_difs_below_eps = 0;
	}
}
