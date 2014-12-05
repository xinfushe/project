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
 * VarianceFilter.cpp
 *
 *  Created on: Nov 16, 2011
 *      Author: Georg Nebehay
 */

#include "VarianceFilter.h"

#include "IntegralImage.h"
#include "DetectorCascade.h"

//
#include "Timing.h"
#include <string.h>

using namespace cv;

namespace tld
{

Mat img_integral;
ocl::oclMat img_integral_ocl;
Mat img_integral_squared;
ocl::oclMat img_integral_squared_ocl;

VarianceFilter::VarianceFilter()
{
    enabled = true;
    minVar = 0;
    integralImg = NULL;
    integralImg_squared = NULL;
}

VarianceFilter::~VarianceFilter()
{
    release();
}

void VarianceFilter::release()
{
    if(integralImg != NULL) delete integralImg;

    integralImg = NULL;

    if(integralImg_squared != NULL) delete integralImg_squared;

    integralImg_squared = NULL;
}

float VarianceFilter::calcVariance(int *off)
{

    //int *ii1 = integralImg->data;
    long long *ii2 = integralImg_squared->data;

    //float mX  = (ii1[off[3]] - ii1[off[2]] - ii1[off[1]] + ii1[off[0]]) / (float) off[5]; //Sum of Area divided by area
    int * ii1 = (int*)img_integral.data;
    int cols = img_integral.cols;
    float mX  = (ii1[cols + 1 + off[3]/(cols-1) + off[3]]
               - ii1[cols + 1 + off[2]/(cols-1) + off[2]]
               - ii1[cols + 1 + off[1]/(cols-1) + off[1]]
               + ii1[cols + 1 + off[0]/(cols-1) + off[0]]) / (float) off[5];
    float mX2 = (ii2[off[3]] - ii2[off[2]] - ii2[off[1]] + ii2[off[0]]) / (float) off[5];
    return mX2 - mX * mX;
}

void VarianceFilter::nextIteration(const cv::Mat &img)
{
    if(!enabled) return;

    release();


    tick_t procInit, procFinal;
    getCPUTick(&procInit);
    //integralImg = new IntegralImage<int>(img.size());
    //integralImg->calcIntImg(img);

    integral(img, img_integral, CV_32S);
    getCPUTick(&procFinal);
    PRINT_TIMING("Variance Calculation Time: ", procInit, procFinal, "\n");
    integralImg_squared = new IntegralImage<long long>(img.size());
    integralImg_squared->calcIntImg(img, true);
    getCPUTick(&procFinal);
    PRINT_TIMING("Variance Calculation Time: ", procInit, procFinal, "\n");

    //std::cout << "Non ocl value is " << integralImg->data[1280] << std::endl;


    //tick_t procInit, procFinal;
//    getCPUTick(&procInit);
//    Mat img_integral;
//    Mat img_integral_squared;
//    ocl::oclMat img_ocl(img.size(), CV_8UC1);
//    ocl::oclMat img_integral_ocl(img.size(), CV_32S);
//    ocl::oclMat img_integral_squared_ocl(img.size(), CV_32F);
//
//    printf("Before upload is %i\n", img.data[0]);
//    //img_ocl.upload(img);
//    Mat img_c;
//    img.convertTo(img_c, CV_8UC1);
//    //img_ocl.download(img_c);
//    //img_c.data = (unsigned char*)img_c.data;
//    img_ocl.upload(img_c);
//    img_integral_ocl.upload(img_integral);
//    img_integral_squared_ocl.upload(img_integral_squared);
//    printf("After upload is %i\n", img_c.data[0]);
//    ocl::integral(img_ocl,img_integral_ocl, img_integral_squared_ocl);
//    img_integral_ocl.download(img_integral);
//    img_integral_squared_ocl.download(img_integral_squared);
    //Mat imgc(img.rows, img.cols, CV_8UC1, img.data, img.step);
//    integral(img, img_integral/*, img_integral_squared*/);
//
//    integralImg = new IntegralImage<int>(img.size());
//    //integralImg_squared = new IntegralImage<long long>(img.size());
//    //copy the data from the download ocl matrices
//    //use for loops to copy
////    int cols = img.cols;
////    int rows = img.rows;
////    for(int i = 0; i < cols; i ++)
////    {
////    	for(int j = 0; j < rows; j ++)
////    	{
////    		integralImg->data[j * cols + i] = img_integral.at<int>(1 + j, 1 + i);
////    	}
////    }
//    std::cout << "Ocl value is " << ((int*)img_integral.data)[1281*(175+1)+(134+1)] << std::endl;
////    img_integral = img_integral.colRange(1, img.cols+1);
////
////    std::cout << img_integral.cols << "x" << img_integral.rows << std::endl;
////    img_integral = img_integral.rowRange(1, img.rows+1);
//    img_integral = Mat(img_integral, Range(1,721),Range(1,1280));
//    //std::cout << "Ocl value is " << ((int*)img_integral.data)[10000] << std::endl;
//    std::cout << img_integral.cols << "x" << img_integral.rows << std::endl;
//
//
//    std::cout << "Ocl value is " << ((int*)img_integral.data)[1280] << std::endl;
//    for(int i = 0; i < img.size().width * img.size().height; i ++)
//    {
//    	integralImg->data[i] = ((int*)img_integral.data)[i];
//    	//integralImg_squared->data[i] = (long long)img_integral.data[i];
//    }
//
//    getCPUTick(&procFinal);
//    PRINT_TIMING("Variance Calculation OpenCV Time: ", procInit, procFinal, "\n");
    //std::cout << "Ocl value is " << img_integral.at<int>(1+719, 1+1279) << std::endl;


    //use memcpy to copy
//    size_t size1 = img.rows * img.cols * sizeof(int);
//    size_t size2 = img.rows * img.cols * sizeof(long long);
//    memcpy(integralImg->data, (int*)img_integral.data, size1);
//    memcpy(integralImg_squared->data, (long long *)img_integral_squared.data, size2);
}

void VarianceFilter::nextIteration(const cv::Mat &img, const ocl::oclMat &img_ocl)
{
    if(!enabled) return;

    release();


    tick_t procInit, procFinal;
    getCPUTick(&procInit);
    //integralImg = new IntegralImage<int>(img.size());
    //integralImg->calcIntImg(img);

    //ocl::integral(img_ocl, img_integral_ocl);
    //img_integral_ocl.download(img_integral);
    integral(img, img_integral, img_integral_squared);
    getCPUTick(&procFinal);
    PRINT_TIMING("Variance Calculation Time: ", procInit, procFinal, "\n");
    integralImg_squared = new IntegralImage<long long>(img.size());
    integralImg_squared->calcIntImg(img, true);
    getCPUTick(&procFinal);
    PRINT_TIMING("Variance Calculation Time: ", procInit, procFinal, "\n");

    std::cout << "Non ocl value is " << img_integral.at<int>(1,1) << std::endl;
    std::cout << "Ocl value is " << img_integral_squared.at<double>(1,1) << std::endl;

    //tick_t procInit, procFinal;
//    getCPUTick(&procInit);
//    Mat img_integral;
//    Mat img_integral_squared;
//    ocl::oclMat img_ocl(img.size(), CV_8UC1);
//    ocl::oclMat img_integral_ocl(img.size(), CV_32S);
//    ocl::oclMat img_integral_squared_ocl(img.size(), CV_32F);
//
//    printf("Before upload is %i\n", img.data[0]);
//    //img_ocl.upload(img);
//    Mat img_c;
//    img.convertTo(img_c, CV_8UC1);
//    //img_ocl.download(img_c);
//    //img_c.data = (unsigned char*)img_c.data;
//    img_ocl.upload(img_c);
//    img_integral_ocl.upload(img_integral);
//    img_integral_squared_ocl.upload(img_integral_squared);
//    printf("After upload is %i\n", img_c.data[0]);
//    ocl::integral(img_ocl,img_integral_ocl, img_integral_squared_ocl);
//    img_integral_ocl.download(img_integral);
//    img_integral_squared_ocl.download(img_integral_squared);
    //Mat imgc(img.rows, img.cols, CV_8UC1, img.data, img.step);
//    integral(img, img_integral/*, img_integral_squared*/);
//
//    integralImg = new IntegralImage<int>(img.size());
//    //integralImg_squared = new IntegralImage<long long>(img.size());
//    //copy the data from the download ocl matrices
//    //use for loops to copy
////    int cols = img.cols;
////    int rows = img.rows;
////    for(int i = 0; i < cols; i ++)
////    {
////    	for(int j = 0; j < rows; j ++)
////    	{
////    		integralImg->data[j * cols + i] = img_integral.at<int>(1 + j, 1 + i);
////    	}
////    }
//    std::cout << "Ocl value is " << ((int*)img_integral.data)[1281*(175+1)+(134+1)] << std::endl;
////    img_integral = img_integral.colRange(1, img.cols+1);
////
////    std::cout << img_integral.cols << "x" << img_integral.rows << std::endl;
////    img_integral = img_integral.rowRange(1, img.rows+1);
//    img_integral = Mat(img_integral, Range(1,721),Range(1,1280));
//    //std::cout << "Ocl value is " << ((int*)img_integral.data)[10000] << std::endl;
//    std::cout << img_integral.cols << "x" << img_integral.rows << std::endl;
//
//
//    std::cout << "Ocl value is " << ((int*)img_integral.data)[1280] << std::endl;
//    for(int i = 0; i < img.size().width * img.size().height; i ++)
//    {
//    	integralImg->data[i] = ((int*)img_integral.data)[i];
//    	//integralImg_squared->data[i] = (long long)img_integral.data[i];
//    }
//
//    getCPUTick(&procFinal);
//    PRINT_TIMING("Variance Calculation OpenCV Time: ", procInit, procFinal, "\n");
    //std::cout << "Ocl value is " << img_integral.at<int>(1+719, 1+1279) << std::endl;


    //use memcpy to copy
//    size_t size1 = img.rows * img.cols * sizeof(int);
//    size_t size2 = img.rows * img.cols * sizeof(long long);
//    memcpy(integralImg->data, (int*)img_integral.data, size1);
//    memcpy(integralImg_squared->data, (long long *)img_integral_squared.data, size2);
}

bool VarianceFilter::filter(int i)
{
    if(!enabled) return true;

    float bboxvar = calcVariance(windowOffsets + TLD_WINDOW_OFFSET_SIZE * i);

    detectionResult->variances[i] = bboxvar;

    if(bboxvar < minVar)
    {
        return false;
    }

    return true;
}

} /* namespace tld */
