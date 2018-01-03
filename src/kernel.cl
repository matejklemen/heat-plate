__kernel void calculate_point(__global float *first_plate,
                              __global float *second_plate,
                              __global int *all_difs_below_eps,
                              int height,
                              int width,
                              float eps)
{
	int id = get_global_id(0);
	int size = get_global_size(0);
	
	for(int cell_idx = width + id; cell_idx < ((height - 1) * width); cell_idx += size)
	{
		if((cell_idx + 1) % width > 1)
		{
			float curr_temp = (second_plate[cell_idx - width] +
			                         second_plate[cell_idx - 1] +
			                         second_plate[cell_idx + 1] +
			                         second_plate[cell_idx + width]) / 4;
			
			float curr_diff = fabs(curr_temp - second_plate[cell_idx]);
			
			first_plate[cell_idx] = curr_temp;

			if(curr_diff > eps)
				*all_difs_below_eps = 0;
		}
	}
}
