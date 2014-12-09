__kernel void oclfilter_kernel (__global bool* enabled_d,
							  	__global bool* state_d,
							  	__global float* p_d,
							  	__global float* v_d,
							  	__global float* minVar_d,
							  	__global int* j_d,
							  	__global int* off_d,
							  	__global int* window_offsets_d,
							  	__global int* tld_size_d,
							  	__global int* ii1_d,
							  	__global long long* ii2_d,
							  	__global float* mX_d,
							  	__global float* mX2_d,
							  	__global float* bboxvar_d,
							  	__global int* img_size_d,
							  	__global int* window_size_d)
{
    /* get_global_id(0) returns the ID of the thread in execution.
    As many threads are launched at the same time, executing the same kernel,
    each one will receive a different ID, and consequently perform a different computation.*/
    const int idx = get_global_id(0);

    /* Now each work-item asks itself: "is my ID inside the vector's range?"
    If the answer is YES, the work-item performs the corresponding computation*/
    if (idx < (*window_size_d))
    {
		if((*enabled_d) == false)
		{
			state_d[idx] = true;
			*j_d = *j_d + 1;
		}
		else
		{
			off_d = window_offsets_d + (*tld_size_d) * idx;
		    // long long *ii2 = integralImg_squared->data;
		    // int* ii1 = (int*)img_integral.data;
		    (*mX_d)  = (ii1_d[off_d[3]] - ii1_d[off_d[2]] - ii1_d[off_d[1]] + ii1_d[off_d[0]]) / (float) off_d[5]; //Sum of Area divided by area
		    (*mX2_d) = (ii2_d[off_d[3]] - ii2_d[off_d[2]] - ii2_d[off_d[1]] + ii2_d[off_d[0]]) / (float) off_d[5];
			(*bboxvar_d) = (*mX2_d) - (*mX_d) * (*mX_d);
			v_d[idx] = *bboxvar_d;

			if((*bboxvar_d) < (*minVar_d))
			{
				p_d[idx] = 0;
				state_d[idx] = false;
			}
			else
			{
				state_d[idx] = true;
				*j_d = *j_d + 1;
			}
		}
	}
}