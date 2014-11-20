/*  Copyright 2011 AIT Austrian Institute of Technology
*
*   This file is part of OpenTLD.
*
*   OpenTLD is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   OpenTLD is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with OpenTLD.  If not, see <http://www.gnu.org/licenses/>.
*
*/
/*
 * MainX.cpp
 *
 *  Created on: Nov 17, 2011
 *      Author: Georg Nebehay
 */

#include "Main.h"

#include "Config.h"
#include "ImAcq.h"
#include "Gui.h"
#include "TLDUtil.h"
#include "Trajectory.h"
#include "../../libopentld/tld/TLD.h"
#include "Timing.h"
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h> 
#include <linux/videodev2.h>
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

//
#include "./GraphUtils.cpp"
//

//ocl.cpp
/*
#include <iostream>
#include <cassert>
#include <stdio.h>

#include <CL/cl.h>
#include <opencv/cv.h>


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

void vector_add_gpu(float* const src_a_h,
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

void gpu_release(void)
{
	clReleaseCommandQueue(queue);
	clReleaseContext(context);
}
*/
//end ocl.cpp

#define CLEAR(x) memset(&(x), 0, sizeof(x))
using namespace tld;
using namespace cv;

double contrast_measure(const Mat& img)
{
    Mat dx,dy;
    Sobel(img,dx,CV_32F,1,0,3);
    Sobel(img,dy,CV_32F,0,1,3);
    magnitude(dx,dy,dx);
    return sum(dx)[0]/(img.cols*img.rows);
}
static int xioctl(int fh, int request, void *arg)
{
        int r;

        do {
                r = ioctl(fh, request, arg);
        } while (-1 == r && EINTR == errno);

        return r;
}
void disableAutoFocus(int fh)
{
    struct v4l2_ext_controls ext_ctrls;
    struct v4l2_ext_control ext_ctrl;
    CLEAR(ext_ctrls);
    CLEAR(ext_ctrl);
    ext_ctrl.id = V4L2_CID_FOCUS_AUTO;
    ext_ctrl.value = 0;
    ext_ctrls.ctrl_class = V4L2_CTRL_ID2CLASS(ext_ctrl.id);
    ext_ctrls.count = 1;
    ext_ctrls.controls = &ext_ctrl;
    if (-1 == xioctl (fh,VIDIOC_S_EXT_CTRLS, &ext_ctrls))
    {
    perror ("VIDIOC_S_EXT_CTRLS");
        printf("Failed disable autofocus \n");
        exit (EXIT_FAILURE);
    }
    else printf("Disable autofocus successfully\n");
}

static void setFocus(int fh,int focus)
{
    struct v4l2_ext_controls ext_ctrls;
    struct v4l2_ext_control ext_ctrl;
    CLEAR(ext_ctrls);
    CLEAR(ext_ctrl);
  
    ext_ctrl.id = V4L2_CID_FOCUS_ABSOLUTE;
    ext_ctrl.value = focus;
    ext_ctrls.ctrl_class = V4L2_CTRL_ID2CLASS(ext_ctrl.id);
    ext_ctrls.count = 1;
    ext_ctrls.controls = &ext_ctrl;
    if (-1 == xioctl (fh,VIDIOC_S_EXT_CTRLS, &ext_ctrls))
    {
        perror ("VIDIOC_S_EXT_CTRLS");
        printf("Failed set V4L2_CID_FOCUS_ABSOLUTE \n");
        exit (EXIT_FAILURE);
    }
    else printf("Set focus to %d successfully\n",focus);
}
static int getFocus(int currsize,int lastsize,int bestfocus)
{
    int res;
    if(bestfocus < 20) res = currsize-lastsize;
    else res = (float)currsize/(float)lastsize*bestfocus;
    return res < 80 ? res : 80;
    
}
void Main::doWork()
{
	//OpenCL initialize

	//end OpenCL initialize
	Trajectory trajectory;
    IplImage *img = imAcqGetImg(imAcq);

    //Create the matrices
    Mat color;
    cv::ocl::oclMat color_ocl;
    Mat grey;
    cv::ocl::oclMat grey_ocl;

    //Initialize the matirces
    color = Mat(img);
    color_ocl.upload(color);
    grey = Mat(img->height, img->width, CV_8UC1);
    cvtColor(color , grey, CV_BGR2GRAY);
    grey_ocl.upload(grey);

    //
    const int fps_size = 166;
    float fps_array[fps_size] = {0.0};
    IplImage* data;
    //

/* original code
    Mat grey(img->height, img->width, CV_8UC1);
    cvtColor(cv::Mat(img), grey, CV_BGR2GRAY);
*/


    tld->detectorCascade->setImgSize(grey.cols, grey.rows, grey.step);


	if(showTrajectory)
	{
		trajectory.init(trajectoryLength);
	}

    if(selectManually)
    {

        CvRect box;

        if(getBBFromUser(img, box, gui) == PROGRAM_EXIT)
        {
            return;
        }

        if(initialBB == NULL)
        {
            initialBB = new int[4];
        }

        initialBB[0] = box.x;
        initialBB[1] = box.y;
        initialBB[2] = box.width;
        initialBB[3] = box.height;
    }

    FILE *resultsFile = NULL;

    if(printResults != NULL)
    {
        resultsFile = fopen(printResults, "w");
    }

    bool reuseFrameOnce = false;
    bool skipProcessingOnce = false;

    if(loadModel && modelPath != NULL)
    {
        tld->readFromFile(modelPath);
        reuseFrameOnce = true;
    }
    else if(initialBB != NULL)
    {
        Rect bb = tldArrayToRect(initialBB);

        printf("Starting at %d %d %d %d\n", bb.x, bb.y, bb.width, bb.height);

        //
        tld->selectObject(grey, &bb);
        skipProcessingOnce = true;
        reuseFrameOnce = true;
    }

    //Focus init
    const char* dev_name = "/dev/video1";
    int fh = open(dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);
    disableAutoFocus(fh);
    int focus = 0;

    double bestsharpness=0,lastsharpness=0;
    int bestfocus=0,initsize,lastsize,focusCount=0,focusChange = 5,initfocus = 0,errorcount=0,focusend=200;
    bool init = false,changing = false;
    setFocus(fh,focus);

    while(imAcqHasMoreFrames(imAcq))
    {
        tick_t procInit, procFinal;
        double tic = cvGetTickCount();

        img = imAcqGetImg(imAcq);

        //update the image matrices
        color = Mat(img);
        color_ocl.upload(color);
        cvtColor(color, grey, CV_BGR2GRAY);
        grey_ocl.upload(grey);
        //Alternative
        //
        cv::ocl::cvtColor(color_ocl, grey_ocl, CV_BGR2GRAY);
        //


        if(!reuseFrameOnce)
        {


            if(img == NULL)
            {
                printf("current image is NULL, assuming end of input.\n");
                break;
            }

            //
            cvtColor(cv::Mat(img), grey, CV_BGR2GRAY);
        }

        if(!skipProcessingOnce)
        {

            getCPUTick(&procInit);
            tld->processImage(img);
            getCPUTick(&procFinal);
//             PRINT_TIMING("FrameProcTime", procInit, procFinal, "\n");
        }
        else
        {
            skipProcessingOnce = false;
        }

        if(printResults != NULL)
        {
            if(tld->currBB != NULL)
            {
                fprintf(resultsFile, "%d %.2d %.2d %.2d %.2d %f\n", imAcq->currentFrame - 1, tld->currBB->x, tld->currBB->y, tld->currBB->width, tld->currBB->height, tld->currConf);
            }
            else
            {
                fprintf(resultsFile, "%d NaN NaN NaN NaN NaN\n", imAcq->currentFrame - 1);
            }
        }

        double toc = (cvGetTickCount() - tic) / cvGetTickFrequency();

        toc = toc / 1000000;

        float fps = 1 / toc;

        //
        for(int i = 0; i < fps_size - 1; i ++)
        {
        	fps_array[i] = fps_array[i+1];
        	fps_array[fps_size - 1] = fps;
        }
        data = drawFloatGraph(fps_array, fps_size, 0, 0.0, 30.0);
        //

        int confident = (tld->currConf >= threshold) ? 1 : 0;

        if(showOutput || saveDir != NULL)
        {
            char string[128];

            char learningString[10] = "";

            if(tld->learning)
            {
                strcpy(learningString, "Learning");
            }

            //
            sprintf(string, "#%d,Posterior %.2f; fps: %.2f, #numwindows:%d, %s", imAcq->currentFrame - 1, tld->currConf, fps, tld->detectorCascade->numWindows, learningString);

            CvScalar yellow = CV_RGB(255, 255, 0);
            CvScalar blue = CV_RGB(0, 0, 255);  
            CvScalar black = CV_RGB(0, 0, 0);
            CvScalar white = CV_RGB(255, 255, 255);
            Mat imgt = img;
            Rect currRect;
            if(tld->currBB != NULL)
            {
                CvScalar rectangleColor = (confident) ? blue : yellow;
                cvRectangle(img, CvPoint(tld->currBB->tl()), CvPoint(tld->currBB->br()), rectangleColor, 8, 8, 0);
                
                currRect = *(tld->currBB);

                double sharpness = contrast_measure(imgt (currRect));//TODO
                printf("sharpness is %lf\n",sharpness);
                if(!init)
                {
                    if(lastsharpness < sharpness)
                    {   
                        printf("sharpness is %lf,lastsharpness is %lf,focus is %d,bestfocus is %d\n",sharpness,lastsharpness,focus,bestfocus);
                        if(sharpness > bestsharpness)
                        {
                            bestsharpness = sharpness;
                            bestfocus = focus;
                        }
                        lastsharpness = sharpness;
                    }
                    focusCount++;
                    if(focusCount % 3 == 0){
                    focus+=focusChange;
                    focusCount = 0;
                    }

                    if(focus == focusend || sharpness < bestsharpness -4){
                        init = true;
                        focus = bestfocus;
                        initfocus = bestfocus;
                        printf("bestfocus is %d \n",bestfocus);
                        initsize = currRect.height;
                        lastsize = initsize;
                    }
                    setFocus(fh,focus);
                }
                else
                {
                    printf("autofocus is %d,size is %d\n",focus,lastsize);
                    focusCount++;
                    errorcount+= abs(currRect.height-lastsize);
                    lastsize = currRect.height;
                    if(focusCount%10 == 0)
                    {
//                         printf("distense is %d,errorcount is %d\n",lastsize-initsize,errorcount);
                        if(errorcount < 5 && abs(lastsize-initsize)>5)
                        {
                            init = false;
                            bestsharpness = 0;
                            lastsharpness = 0;
                            if(currRect.height >initsize){
//                                 focus = initfocus + (currRect.height-initsize)/10;
//                                 focusend=focus +(currRect.height-initsize)/2;
//                               focus = getFocus(currRect.height,initsize,focus)-20;
//                               focusend = getFocus(currRect.height,initsize,focus)+20;
//                                 printf("XXXXXXXXx!focusend is %d\n",focusend);
                                focusChange = 5;
                                focusend = 200;
                            }
                            else
                            {
                                focusChange = -5;
                                focusend = 0;
                            }

                        }
                        errorcount=0;
                        focusCount=0;
                    }                    
                }
//                 {
// //                    if(abs(currRect.height-lastsize) > 5)
// //                    {
// //                        printf("XXXX!\n");
// //                        focus=getFocus(currRect.height,lastsize,initfocus,bestfocus);
// //                    }
// //                     printf("lastsize is %d",lastsize);
//                     if(focusCount %5==0)
//                     {
//                         focusChange = currRect.height < lastsize ? -1:1;
//                         focusCount = 0;
//                         focus=getFocus(currRect.height,initsize,initfocus,bestfocus);
// //                         printf("init focus is %d, best focus is %d, focus is %d\n",initfocus,bestfocus,focus);
//                         bestsharpness = sharpness;
//                     }
//                     else if(sharpness > bestsharpness)
//                     {
//                         bestfocus = focus;
//                         bestsharpness = sharpness;
//                     }
//                     focus += focusChange;
//                     focus = focus > 0? focus: 0;
//                     focusCount++;
//                         
//                     }
// //                 printf("focusCOunt is %d, focuschange is %d\n",focusCount,focusChange);
// //                 setFocus(fh,focus);
//                 }
				if(showTrajectory)
				{
					CvPoint center = cvPoint(tld->currBB->x+tld->currBB->width/2, tld->currBB->y+tld->currBB->height/2);
					cvLine(img, cvPoint(center.x-2, center.y-2), cvPoint(center.x+2, center.y+2), rectangleColor, 2);
					cvLine(img, cvPoint(center.x-2, center.y+2), cvPoint(center.x+2, center.y-2), rectangleColor, 2);
					trajectory.addPoint(center, rectangleColor);
				}
            }
			else if(showTrajectory)
			{
				trajectory.addPoint(cvPoint(-1, -1), cvScalar(-1, -1, -1));
			}

			if(showTrajectory)
			{
				trajectory.drawTrajectory(img);
			}
            CvFont font;
            cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, .5, .5, 0, 1, 8);
            cvRectangle(img, cvPoint(0, 0), cvPoint(img->width, 50), black, CV_FILLED, 8, 0);
            cvPutText(img, string, cvPoint(25, 25), &font, white);

            if(showForeground)
            {

                for(size_t i = 0; i < tld->detectorCascade->detectionResult->fgList->size(); i++)
                {
                    Rect r = tld->detectorCascade->detectionResult->fgList->at(i);
                    cvRectangle(img, CvPoint(r.tl()), CvPoint(r.br()), white, 1);
                }

            }


            if(showOutput)
            {
            	//
                gui->showImage(img, data);
                //
                char key = gui->getKey();

                if(key == 'q') break;

                /*
                if(key == 'b')
                {

                    ForegroundDetector *fg = tld->detectorCascade->foregroundDetector;

                    if(fg->bgImg.empty())
                    {
                        fg->bgImg = grey.clone();
                    }
                    else
                    {
                        fg->bgImg.release();
                    }
                }
                */
                if(key == 82)
                {   
                focus+=1;
//                 cout<<focus<<endl;
                setFocus(fh,focus);
                }
                if(key == 84)
                {
                focus-=1;
//                 cout<<focus<<endl;
                setFocus(fh,focus);
                }
                if(key == 81)
                {   
                focus-=10;
//                 cout<<focus<<endl;
                setFocus(fh,focus);
                }
                if(key == 83)
                {
                focus+=10;
//                 cout<<focus<<endl;
                setFocus(fh,focus);
                }
                if(key == 'c')
                {
                    //clear everything
                    tld->release();
                }

                if(key == 'l')
                {
                    tld->learningEnabled = !tld->learningEnabled;
                    printf("LearningEnabled: %d\n", tld->learningEnabled);
                }

                if(key == 'a')
                {
                    tld->alternating = !tld->alternating;
                    printf("alternating: %d\n", tld->alternating);
                }

                if(key == 'e')
                {
                    tld->writeToFile(modelExportFile);
                }

                if(key == 'i')
                {
                    tld->readFromFile(modelPath);
                }

                if(key == 'r')
                {
                    CvRect box;

                    if(getBBFromUser(img, box, gui) == PROGRAM_EXIT)
                    {
                        break;
                    }

                    Rect r = Rect(box);
                    currRect=r;
                    initsize = r.height;
                    init = false;
                    bestsharpness = 0;
                    lastsharpness = 0;
                    focus = 0;
                    //
                    tld->selectObject(grey, &r);
                }
                if(key == 'f')
                {
                    CvRect box;
                    const char* cascadeName = "/haarcascade_frontalface_alt.xml";
                    CascadeClassifier  cascade;
                    vector<Rect> faces;
                    if( !cascade.load( cascadeName )  )
                    {
                        printf("ERROR: Could not load classifier cascade: %s\n",cascadeName);
                    }
                    
                    cascade.detectMultiScale(grey, faces, 1.1,
                             3, 0 | CV_HAAR_SCALE_IMAGE,
                             Size(30, 30), Size(0, 0));
                    if(!faces.empty())
                    {
                        Rect r = faces[0];
                        currRect=r;
                        initsize = r.height;
                        init = false;
                        bestsharpness = 0;
                        lastsharpness = 0;
                        focus = 0;
                        //
                        tld->selectObject(grey, &r);

                    }
                }
            }

            if(saveDir != NULL)
            {
                char fileName[256];
                //
                sprintf(fileName, "%s/%.5d.png", saveDir, imAcq->currentFrame - 1);
                cvSaveImage(fileName, img);
            }
        }

        if(!reuseFrameOnce)
        {
            cvReleaseImage(&img);
        }
        else
        {
            reuseFrameOnce = false;
        }
    }

    if(exportModelAfterRun)
    {
        tld->writeToFile(modelExportFile);
    }
}
