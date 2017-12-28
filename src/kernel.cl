__kernel void vector_add(__global float *curr_state,
						 __global float *prev_state,
						 __global float *temp_max_diff,	
						 int height,				
						 int width)						
{
	int curr_cell = get_local_id(0);

	// resetiramo max temperaturno razliko
	temp_max_diff[curr_cell] = 0;

	while(curr_cell < height * width)
	{
		if((curr_cell + 1) % width > 1 &&
			curr_cell >= width &&
			curr_cell + width < height * width)
			curr_state[curr_cell] = (
				prev_state[curr_cell - width] +
				prev_state[curr_cell - 1] +
				prev_state[curr_cell + 1] +
				prev_state[curr_cell + width]				
				) / 4;

		float curr_diff = curr_state[curr_cell] - prev_state[curr_cell];
		
		int idx = get_local_id(0);
		if(curr_diff > temp_max_diff[idx])
			temp_max_diff[idx] = curr_diff;

		curr_cell += get_local_size(0);
	}
}