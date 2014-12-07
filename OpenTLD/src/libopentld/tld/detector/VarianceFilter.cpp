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
//    double* ii1 = (double*)img_integral.data;
//    double* ii2 = (double*)img_integral_squared.data;
//	int* ii1 = (int*)img_integral.data;
//	int* ii2 = (int*)img_integral_squared.data;

//    double mX2  = (ii2[cols + 1 + off[3]/(cols-1) + off[3]]
//                 - ii2[cols + 1 + off[2]/(cols-1) + off[2]]
//                 - ii2[cols + 1 + off[1]/(cols-1) + off[1]]
//                 + ii2[cols + 1 + off[0]/(cols-1) + off[0]]) / (double) off[5];
//    return (float)((mX2 - mX * mX)*100.0);
//
    //int *ii1 = integralImg->data;
    long long *ii2 = integralImg_squared->data;
    int* ii1 = (int*)img_integral.data;
    int cols = img_integral.cols;
    double mX  = (ii1[cols + 1 + off[3]/(cols-1) + off[3]]
                - ii1[cols + 1 + off[2]/(cols-1) + off[2]]
                - ii1[cols + 1 + off[1]/(cols-1) + off[1]]
                + ii1[cols + 1 + off[0]/(cols-1) + off[0]]) / (double) off[5];
    //float mX  = (ii1[off[3]] - ii1[off[2]] - ii1[off[1]] + ii1[off[0]]) / (float) off[5]; //Sum of Area divided by area
    float mX2 = (ii2[off[3]] - ii2[off[2]] - ii2[off[1]] + ii2[off[0]]) / (float) off[5];
    return mX2 - mX * mX;
    //return (float)((mX2 - mX * mX)*100.0);
}

void VarianceFilter::nextIteration(const cv::Mat &img)
{
    if(!enabled) return;

    release();

    tick_t procInit, procFinal;

    getCPUTick(&procInit);

//    Mat temp = img;
//    printf("temp is: %d\n", temp.data[0]);
//    temp.convertTo(temp, CV_8U);
//    temp /= 8;
//    temp.convertTo(temp, CV_8UC1);

//    printf("temp is: %d\n", temp.data[0]);
    ocl::oclMat img_ocl;
    img_ocl.upload(img);
    ocl::oclMat img_integral_ocl;
    ocl::integral(img_ocl, img_integral_ocl);
    img_integral_ocl.download(img_integral);
    //integralImg = new IntegralImage<int> (img.size());
    //integralImg->calcIntImg(img, false);
    integralImg_squared = new IntegralImage<long long> (img.size());
    integralImg_squared->calcIntImg(img, true);

//    img_integral = img_integral.rowRange(Range(1, 721));
//    img_integral = img_integral.t();
//    img_integral = img_integral.rowRange(Range(1, 1281));
//    img_integral = img_integral.t();
//    img_integral_squared = img_integral_squared.rowRange(Range(1, 721));
//    img_integral_squared = img_integral_squared.t();
//    img_integral_squared = img_integral_squared.rowRange(Range(1, 1281));
//    img_integral_squared = img_integral_squared.t();


//    for(int i = 0; i < img.cols * img.rows - 1; i ++)
//    {
//    	integralImg->data[i] = (int)((int*)(img_integral.data))[i]*8;
//    	integralImg_squared->data[i] = (long long)((int*)(img_integral_squared.data))[i]*64;
//    }


    getCPUTick(&procFinal);
    PRINT_TIMING("Variance Calculation Time: ", procInit, procFinal, "\n");
//    int r = 719;
//    int l = 1279;
//    printf("111111111: %lli, 222222222: %lli\n", 64*((int*)img_integral_squared.data)[(0+r)*1280+(0+l)], integralImg_squared->data[r*1280+l]);

    //std::cout <<img_integral_squared.rows << std::endl;

}

void VarianceFilter::nextIteration(const cv::Mat &img, const ocl::oclMat &img_ocl)
{
    if(!enabled) return;

    release();

    tick_t procInit, procFinal;

    getCPUTick(&procInit);

    ocl::oclMat img_integral_ocl;
    ocl::integral(img_ocl, img_integral_ocl);
    img_integral_ocl.download(img_integral);
    //integralImg = new IntegralImage<int> (img.size());
    //integralImg->calcIntImg(img, false);
    integralImg_squared = new IntegralImage<long long> (img.size());
    integralImg_squared->calcIntImg(img, true);

//    img_integral = img_integral.rowRange(Range(1, 721));
//    img_integral = img_integral.t();
//    img_integral = img_integral.rowRange(Range(1, 1281));
//    img_integral = img_integral.t();
//    img_integral_squared = img_integral_squared.rowRange(Range(1, 721));
//    img_integral_squared = img_integral_squared.t();
//    img_integral_squared = img_integral_squared.rowRange(Range(1, 1281));
//    img_integral_squared = img_integral_squared.t();


//    for(int i = 0; i < img.cols * img.rows - 1; i ++)
//    {
//    	integralImg->data[i] = (int)((int*)(img_integral.data))[i]*8;
//    	integralImg_squared->data[i] = (long long)((int*)(img_integral_squared.data))[i]*64;
//    }


    getCPUTick(&procFinal);
    PRINT_TIMING("Variance Calculation Time: ", procInit, procFinal, "\n");
//    int r = 719;
//    int l = 1279;
//    printf("111111111: %lli, 222222222: %lli\n", 64*((int*)img_integral_squared.data)[(0+r)*1280+(0+l)], integralImg_squared->data[r*1280+l]);

    //std::cout <<img_integral_squared.rows << std::endl;

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
