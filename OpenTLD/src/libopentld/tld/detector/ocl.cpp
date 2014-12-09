#include <iostream>
#include <cassert>
#include <stdio.h>
#include <opencv/cv.h>
#include "ocl.h"
using std::cout;
using std::endl;

//opencl::opencl()
//{
//	gpu_init();
//}

//opencl::~opencl()
//{
//	gpu_release();
//}

void opencl::gpu_init(void)
{
	// OpenCL Environment variables
	num_entries = 1;
	//platform_id;
	//num_platforms;

	device_type = CL_DEVICE_TYPE_GPU;
	//device_id;
	//num_devices;

	//context;
	//context_properties;
	//void* pfn_notify = NULL;
	//void* user_data = NULL;
	//queue;
	command_queue_properties = 0;
	error = CL_SUCCESS;

	// Initializing the basic OpenCL environment
	error = clGetPlatformIDs(num_entries, &platform_id, &num_platforms);
	assert(error == CL_SUCCESS);

	error = clGetDeviceIDs(platform_id, device_type, num_entries, &device_id, &num_devices);
	assert(error == CL_SUCCESS);

	//Print device information
	//Device Name
	char* device_name;
	size_t device_name_size = 0;
	clGetDeviceInfo(device_id, CL_DEVICE_NAME, 0, NULL, &device_name_size);
	device_name = new char[device_name_size];
	clGetDeviceInfo(device_id, CL_DEVICE_NAME, device_name_size, device_name, NULL);
	printf("Device Name: %s\n", device_name);
	delete[] device_name;
	//Device Vendor
	char* device_vendor;
	size_t device_vendor_size = 0;
	clGetDeviceInfo(device_id, CL_DEVICE_VENDOR, 0, NULL, &device_vendor_size);
	device_vendor = new char[device_vendor_size];
	clGetDeviceInfo(device_id, CL_DEVICE_VENDOR, device_vendor_size, device_vendor, NULL);
	printf("Device Vendor: %s\n", device_vendor);
	delete[] device_vendor;
	//Device Version
	char* device_version;
	size_t device_version_size = 0;
	clGetDeviceInfo(device_id, CL_DEVICE_VERSION, 0, NULL, &device_version_size);
	device_version = new char[device_version_size];
	clGetDeviceInfo(device_id, CL_DEVICE_VERSION, device_version_size, device_version, NULL);
	printf("Device Version: %s\n", device_version);
	delete[] device_version;
	//Device Profile
	char* device_profile;
	size_t device_profile_size = 0;
	clGetDeviceInfo(device_id, CL_DEVICE_PROFILE, 0, NULL, &device_profile_size);
	device_profile = new char[device_profile_size];
	clGetDeviceInfo(device_id, CL_DEVICE_PROFILE, device_profile_size, device_profile, NULL);
	printf("Device Profile: %s\n", device_profile);
	delete[] device_profile;
	//Device Extensions
	char* device_extensions;
	size_t device_extensions_size = 0;
	clGetDeviceInfo(device_id, CL_DEVICE_EXTENSIONS, 0, NULL, &device_extensions_size);
	device_extensions = new char[device_extensions_size];
	clGetDeviceInfo(device_id, CL_DEVICE_EXTENSIONS, device_extensions_size, device_extensions, NULL);
	printf("Device Extensions: %s\n", device_extensions);
	delete[] device_extensions;
	//Device Platform
//	char* device_platform;
//	cl_ulong device_platform_size = 0;
//	clGetDeviceInfo(device_id, CL_DEVICE_PLATFORM, 0, NULL, &device_platform_size);
//	device_platform = new char[device_platform_size];
//	clGetDeviceInfo(device_id, CL_DEVICE_PLATFORM, device_platform_size, device_platform, NULL);
//	printf("Device Platform: %s\n", device_platform);
//	delete[] device_platform;


	//Print computing abilities
	//Maximum Clock Frequency
	cl_ulong max_clock_frequency = 0;
	clGetDeviceInfo(device_id, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(max_clock_frequency), &max_clock_frequency, NULL);
	printf("Maximum Clock Frequency: %lu MHz\n", max_clock_frequency);
	//Maximum Computing Units
	cl_uint max_compute_units = 0;
	clGetDeviceInfo(device_id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(max_compute_units), &max_compute_units, NULL);
	printf("Maximum Computing Units: %u\n", max_compute_units);
	//Maximum Work Items Per Group
	size_t max_work_group_size = 0;
	clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(max_work_group_size), &max_work_group_size, NULL);
	printf("Maximum Work Items Per Group: %u\n", max_work_group_size);
	//Global Memory Size
	cl_ulong global_mem_size = 0;
	clGetDeviceInfo(device_id, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(global_mem_size), &global_mem_size, NULL);
	printf("Global Memory Size: %lu MB\n", global_mem_size / (1024 * 1024));
	//Local Memory Size
	cl_ulong local_mem_size = 0;
	clGetDeviceInfo(device_id, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(local_mem_size), &local_mem_size, NULL);
	printf("Local Memory Size: %lu KB\n", local_mem_size / 1024);


	context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &error);
	assert(error == CL_SUCCESS);

	queue = clCreateCommandQueue(context, device_id, command_queue_properties, &error);
	assert(error == CL_SUCCESS);
}

void opencl::vector_add_gpu(float* const src_a_h,
					float* const src_b_h,
					float* const res_h,
					const int size)
{
	double freq = cvGetTickFrequency()*1000.0; //kHz
	double tic = 0.0;
	double toc = 0.0;
	double time_io = 0.0;
	double time_calc = 0.0;

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
	time_io += toc - tic;



	// Creates the program
	const char* path = "/home/slilylsu/Desktop/project-repo/OpenTLD/src/libopentld/tld/detector/kernels/vector_add_kernel.cl";
	FILE* fp;
	fp = fopen(path, "r");
	if (!fp)
	{
		fprintf(stderr, "Failed to load kernel: 'vector_add_kernel.cl'.\n");
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
	//cout << build_log << endl;
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

	// Launching kernel
	const size_t local_worksize = 1024;	// Number of work-items per work-group
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
	time_calc += toc - tic;

	// Reading back
//	float* check = new float[size];
//	clEnqueueReadBuffer(queue, res_d, CL_TRUE, 0, mem_size, check, num_events_in_wait_list, NULL, NULL);
//
//	// Checking with the CPU results;
//	vector_add_cpu(src_a_h, src_b_h, res_h, size);
//	for (int i = 0; i < size; i++)
//		assert(check[i] == res_h[i]);
//	cout << "Congratulations, it's working!" << endl;
//	delete[] check;

	tic = cvGetTickCount();
	clEnqueueReadBuffer(queue, res_d, CL_TRUE, 0, mem_size, res_h, num_events_in_wait_list, NULL, NULL);
	toc = cvGetTickCount();
	time_io += toc - tic;

	cout << "GPU IO Time: " << time_io/(double)freq << " ms" << endl;
	cout << "GPU Calculation Time: " << time_calc/(double)freq << " ms" << endl;

	//Release
	clReleaseMemObject(src_a_d);
	clReleaseMemObject(src_b_d);
	clReleaseMemObject(res_d);
	clReleaseKernel(vector_add_gpu_kernel);

}

void opencl::vector_add_cpu(const float* const src_a,
					const float* const src_b,
					float* const res,
					const int size)
{
	for (int i = 0; i < size; i++) {
		res[i] = src_a[i] + src_b[i];
	}
}

void opencl::gpu_release(void)
{
	clReleaseCommandQueue(queue);
	clReleaseContext(context);
}
