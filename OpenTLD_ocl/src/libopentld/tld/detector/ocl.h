#include <CL/cl.h>

class opencl
{
	public:
	// OpenCL Environment variables
	cl_uint num_entries;
	cl_platform_id platform_id;
	cl_uint num_platforms;

	cl_device_type device_type;
	cl_device_id device_id;
	cl_uint num_devices;

	cl_context context;
	cl_context_properties context_properties;
	//void* pfn_notify = NULL;
	//void* user_data = NULL;
	cl_command_queue queue;
	cl_command_queue_properties command_queue_properties;
	cl_int error;

	cl_kernel oclfilter_variance_kernel;

	bool oclfilter_variance_state;

	cl_kernel oclfilter_ensemble_kernel;

	bool oclfilter_ensemble_state;

	opencl();

	~opencl();

	void gpu_init(void);

	/*void vector_add_gpu(float* const src_a_h,
					float* const src_b_h,
					float* const res_h,
					const int size);

	void vector_add_cpu(const float* const src_a,
					const float* const src_b,
					float* const res,
					const int size);*/

	void oclfilter_variance(bool* enabled,
			  	  	  	  bool* state,
			  	  	  	  float* p,
			  	  	  	  float* v,
			  	  	  	  float* minVar,
			  	  	  	  int* j,
			  	  	  	  //int* off,
			  	  	  	  int* window_offsets,
			  	  	  	  int* tld_size,
			  	  	  	  int* ii1,
			  	  	  	  long long* ii2,
			  	  	  	  //float* mX,
			  	  	  	  //float* mX2,
			  	  	  	  //float* bboxvar,
			  	  	  	  int* img_size,
			  	  	  	  int* window_size);

	void oclfilter_ensemble(int* num,
							bool* state,
							int* k,
							bool* enabled,
							int* detectfeatureVector,
							int* numTrees,
							int* windowOffsets,
							int* tld_size,
							int* featureOffsets,
							int* numFeatures,
							unsigned char* img,
						    int* img_size,
							float* posteriors,
							int* numIndices,
							float* detectposteriors,
							int* numScales);

	void gpu_release(void);
};
