__kernel void oclfilter_ensemble_kernel(__global int* num_d,
								 		__global bool* state_d,
								 		__global int* k_d,
								 		__global bool* enabled_d,
								 		__global int* detectfeatureVector_d,
								 		__global int* numTrees_d,
								 		__global int* windowOffsets_d,
								 		__global int* tld_size_d,
								 		__global int* featureOffsets_d,
										__global int* numFeatures_d,
								 		__global unsigned char* img_d,
								 		__global float* posteriors_d,
								 		__global int* numIndices_d,
								 		__global float* detectposteriors_d,
								 		__global int* numScales_d)
{
    /* get_global_id(0) returns the ID of the thread in execution.
    As many threads are launched at the same time, executing the same kernel,
    each one will receive a different ID, and consequently perform a different computation.*/
    const int idx = get_global_id(0);

    /* Now each work-item asks itself: "is my ID inside the vector's range?"
    If the answer is YES, the work-item performs the corresponding computation*/
    if (idx < (*num_d))
    {
		if(state_d[idx] == true)
		{
		    if(*enabled_d == false)
		    {
		    	state_d[idx] = true;
		    	*k_d = *k_d +1;
		    }
		    else
		    {
			    __global int *featureVector = detectfeatureVector_d + (*numTrees_d) * idx;
			    for(int j = 0; j < (*numTrees_d); j++)
			    {

			        int index = 0;
			        __global int *bbox = windowOffsets_d + idx * (*tld_size_d);
			        __global int *off = featureOffsets_d + bbox[4] + j * 2 * (*numFeatures_d); //bbox[4] is pointer to features for the current scale

			        for(int k = 0; k < (*numFeatures_d); k++)
			        {
			            index <<= 1;

			            int fp0 = img_d[bbox[0] + off[0]];
			            int fp1 = img_d[bbox[0] + off[1]];

			            if(fp0 > fp1)
			            {
			                index |= 1;
			            }

			            off += 2;
			        }

			        featureVector[j] = index;
			    }

			    float conf = 0.0;

			    for(int l = 0; l < (*numTrees_d); l++)
			    {
			        conf += posteriors_d[l * (*numIndices_d) + featureVector[l]];
			    }

			    detectposteriors_d[idx] = conf;

			    if(conf < 0.5)
			    {
			    	state_d[idx] = false;
			    }
			    else
			    {
			    	state_d[idx] = true;
			    	*k_d = *k_d + 1;
			    }
		    }
		}
		else
		{
			state_d[idx] = false;
		}
	}
}