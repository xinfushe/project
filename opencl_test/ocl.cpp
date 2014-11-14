#include <iostream>
#include <cassert>
#include <stdio.h>
#include <CL/cl.h>
using std::cout;
using std::endl;

//
// OpenCL Environment variables
cl_uint num_entries = 1;
cl_platform_id platform_id;
cl_uint num_platforms;

cl_device_type device_type = CL_DEVICE_TYPE_GPU;
cl_device_id device_id;
cl_uint num_devices;

cl_context context;
cl_context_properties context_properties;
//void* pfn_notify = NULL;
//void* user_data = NULL;
cl_command_queue queue;
cl_command_queue_properties command_queue_properties = 0;
cl_int error = CL_SUCCESS;

void gpu_init(void);

void vector_add_gpu(float* const src_a_h,
					float* const src_b_h,
					float* const res_h,
					const int size);

void gpu_release(void);

void vector_add_cpu(const float* const src_a,
					const float* const src_b,
					float* const res,
					const int size);
//


void vector_add_cpu(const float* const src_a,
					const float* const src_b,
					float* const res,
					const int size)
{
	for (int i = 0; i < size; i++) {
		res[i] = src_a[i] + src_b[i];
	}
}

void gpu_init(void)
{
	// Initializing the basic OpenCL environment
	error = clGetPlatformIDs(num_entries, &platform_id, &num_platforms);
	assert(error == CL_SUCCESS);

	error = clGetDeviceIDs(platform_id, device_type, num_entries, &device_id, &num_devices);
	assert(error == CL_SUCCESS);

	context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &error);
	assert(error == CL_SUCCESS);

	queue = clCreateCommandQueue(context, device_id, command_queue_properties, &error);
	assert(error == CL_SUCCESS);
}

void vector_add_gpu(float* const src_a_h,
					float* const src_b_h,
					float* const res_h,
					const int size)
{
	double freq = cvGetTickFrequency()*1000.0; //kHz
	double tic = 0.0;
	double toc = 0.0;
	double time = 0.0;

	//
	tic = cvGetTickCount();
	//
	// Initializing device memory
	const size_t mem_size = sizeof(float)*size;
	cl_mem src_a_d = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, mem_size, src_a_h, &error);
	assert(error == CL_SUCCESS);
	cl_mem src_b_d = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, mem_size, src_b_h, &error);
	assert(error == CL_SUCCESS);
	cl_mem res_d = clCreateBuffer(context, CL_MEM_WRITE_ONLY, mem_size, NULL, &error);
	assert(error == CL_SUCCESS);
	toc = cvGetTickCount();
	time += toc - tic;

	// Creates the program
	const char* path = "./kernels/vector_add_kernel.cl";
	FILE* fp;
	fp = fopen(path, "r");
	if (!fp)
	{
		fprintf(stderr, "Failed to load kernel.\n");
		exit(1);
	}
	const size_t MAX_SOURCE_SIZE = 0x1000000;
	char* source_temp = new char[MAX_SOURCE_SIZE];
	size_t source_size = 0;
	source_size = fread(source_temp, 1, MAX_SOURCE_SIZE, fp);
	const char* source;
	source = source_temp;
	fclose(fp);

	cl_uint program_count = 1;
	cl_program program = clCreateProgramWithSource(context, program_count, &source, &source_size, &error);
	assert(error == CL_SUCCESS);

	// Builds the program
	error = clBuildProgram(program, num_devices, &device_id, NULL, NULL, NULL);
	assert(error == CL_SUCCESS);

	// Shows the log
	char* build_log;
	size_t log_size = 0;

	// First call to know the proper size
	size_t param_value_size = 0;
	char* param_value = NULL;
	clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, param_value_size, &param_value, &log_size);
	build_log = new char[log_size + 1];

	// Second call to get the log
	clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size + 1, build_log, NULL);
	build_log[log_size] = '\0';
	cout << build_log << endl;
	delete[] build_log;

	// 'Extracting' the kernel
	cl_kernel vector_add_gpu_kernel = clCreateKernel(program, "vector_add_kernel", &error);
	assert(error == CL_SUCCESS);

	//
	tic = cvGetTickCount();
	//Enqueuing parameters_work
	error = clSetKernelArg(vector_add_gpu_kernel, 0, sizeof(src_a_d), &src_a_d);
	assert(error == CL_SUCCESS);
	error = clSetKernelArg(vector_add_gpu_kernel, 1, sizeof(src_b_d), &src_b_d);
	assert(error == CL_SUCCESS);
	error = clSetKernelArg(vector_add_gpu_kernel, 2, sizeof(res_d), &res_d);
	assert(error == CL_SUCCESS);
	//Note: the size of size_t is 4 here instead of 8
	error = clSetKernelArg(vector_add_gpu_kernel, 3, sizeof(size), (void *)&size);
	assert(error == CL_SUCCESS);
	toc = cvGetTickCount();
	time += toc - tic;

	tic = cvGetTickCount();
	// Launching kernel
	const size_t local_worksize = 512;	// Number of work-items per work-group
	// The smallest multiple of local_ws bigger than size
	const size_t global_worksize = ((size / local_worksize) + 1) * local_worksize;
	cl_uint work_dim = 1;
	//const size_t global_workoffset = 0;
	cl_uint num_events_in_wait_list = 0;
	//const cl_event* event_wait_list;
	//cl_event* event;
	error = clEnqueueNDRangeKernel(queue, vector_add_gpu_kernel, work_dim, NULL, &global_worksize, &local_worksize, num_events_in_wait_list, NULL, NULL);
	assert(error == CL_SUCCESS);
	toc = cvGetTickCount();
	time += toc - tic;

	// Reading back
	float* check = new float[size];
	clEnqueueReadBuffer(queue, res_d, CL_TRUE, 0, mem_size, check, num_events_in_wait_list, NULL, NULL);

	// Checking with the CPU results;
	vector_add_cpu(src_a_h, src_b_h, res_h, size);
	for (int i = 0; i < size; i++)
		assert(check[i] == res_h[i]);
	cout << "\nCongratulations, it's working!" << endl;
	delete[] check;

	tic = cvGetTickCount();
	clEnqueueReadBuffer(queue, res_d, CL_TRUE, 0, mem_size, res_h, num_events_in_wait_list, NULL, NULL);
	toc = cvGetTickCount();
	time += toc - tic;

	time /= freq;
	cout << "GPU sub Time is " << time << " ms" << endl;

	//Release
	clReleaseMemObject(src_a_d);
	clReleaseMemObject(src_b_d);
	clReleaseMemObject(res_d);
	clReleaseKernel(vector_add_gpu_kernel);

}

void gpu_release(void)
{
	clReleaseCommandQueue(queue);
	clReleaseContext(context);
}
