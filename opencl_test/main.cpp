#include <iostream>
#include <cassert>
#include <stdio.h>

#include <CL/cl.h>
#include <opencv/cv.h>

#include "ocl.cpp"

using std::cout;
using std::endl;

int main()
{
	 //Initializing host memory
	const size_t size = 12345678;
	float* src_a_h = new float[size];
	float* src_b_h = new float[size];
	float* res_h = new float[size];
	float* res_cpu = new float[size];

	for (unsigned int i = 0; i < size; i++) {
		src_a_h[i] = src_b_h[i] = 0.0f*i;
	}

	double freq = cvGetTickFrequency()*1000.0; //kHz
	double tic = 0.0;
	double toc = 0.0;
	double time = 0.0;

	gpu_init();
	tic = cvGetTickCount();
	vector_add_gpu(src_a_h, src_b_h, res_h, size);
	toc = cvGetTickCount();
	gpu_release();
	time = (toc - tic) / freq;
	cout << "GPU Time is " << time << " ms" << endl;

	tic = cvGetTickCount();
	vector_add_cpu(src_a_h, src_b_h, res_cpu, size);
	toc = cvGetTickCount();
	time = (toc - tic) / freq;
	cout << "CPU Time is " << time << " ms" << endl;

	//Release memory
	delete[] src_a_h;
	delete[] src_b_h;
	delete[] res_h;
	delete[] res_cpu;

	return 0;
}
