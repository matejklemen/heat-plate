__kernel void calculate_point(__global float *curr_state,
                              __global float *prev_state,
                              __global float *temp_max_diff,
                              int height,
                              int width)
{
	int id = get_local_id(0);
	int size = get_local_size(0);
	
	temp_max_diff[id] = 0.0f;
	
	for(int cell_idx = width + id; cell_idx < ((height - 1) * width); cell_idx += size)
	{
		if((cell_idx + 1) % width > 1)
		{
			curr_state[cell_idx] = (prev_state[cell_idx - width] +
			                        prev_state[cell_idx - 1] +
			                        prev_state[cell_idx + 1] +
			                        prev_state[cell_idx + width]) / 4;
			
			float curr_diff = curr_state[cell_idx] - prev_state[cell_idx];
			
			if(curr_diff > temp_max_diff[id])
				temp_max_diff[id] = curr_diff;
		}
	}
}
