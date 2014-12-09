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
#include "opencv2/features2d/features2d.hpp"

//
#include "ocl.h"
#include "./GraphUtils.cpp"
//

#define CLEAR(x) memset(&(x), 0, sizeof(x))
using namespace tld;
using namespace cv;

//double contrast_measure(const Mat& img)
//{
//    Mat dx,dy;
//    Sobel(img,dx,CV_32F,1,0,3);
//    Sobel(img,dy,CV_32F,0,1,3);
//    magnitude(dx,dy,dx);
//    return sum(dx)[0]/(img.cols*img.rows);
//}

double contrast_measure(const ocl::oclMat& img)
{
    ocl::oclMat dx,dy;
    ocl::Sobel(img,dx,CV_32F,1,0,3);
    ocl::Sobel(img,dy,CV_32F,0,1,3);
    ocl::magnitude(dx,dy,dx);
    return ocl::sum(dx)[0]/(img.cols*img.rows);
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

void disableAutoExposure(int fh, bool &auto_exposure)
{
    struct v4l2_ext_controls ext_ctrls;
    struct v4l2_ext_control ext_ctrl;
    CLEAR(ext_ctrls);
    CLEAR(ext_ctrl);
    ext_ctrl.id = V4L2_CID_EXPOSURE_AUTO;
    ext_ctrl.value = 1;
    ext_ctrls.ctrl_class = V4L2_CTRL_ID2CLASS(ext_ctrl.id);
    ext_ctrls.count = 1;
    ext_ctrls.controls = &ext_ctrl;
    if (-1 == xioctl (fh,VIDIOC_S_EXT_CTRLS, &ext_ctrls))
    {
    	perror ("VIDIOC_S_EXT_CTRLS");
        printf("Failed disable auto exposure \n");
        auto_exposure = true;
        exit (EXIT_FAILURE);
    }
    else
    {
    	printf("Disable auto exposure successfully\n");
    	auto_exposure = false;
    }
}

void enableAutoExposure(int fh, bool &auto_exposure)
{
    struct v4l2_ext_controls ext_ctrls;
    struct v4l2_ext_control ext_ctrl;
    CLEAR(ext_ctrls);
    CLEAR(ext_ctrl);
    ext_ctrl.id = V4L2_CID_EXPOSURE_AUTO;
    ext_ctrl.value = 3;
    ext_ctrls.ctrl_class = V4L2_CTRL_ID2CLASS(ext_ctrl.id);
    ext_ctrls.count = 1;
    ext_ctrls.controls = &ext_ctrl;
    if (-1 == xioctl (fh,VIDIOC_S_EXT_CTRLS, &ext_ctrls))
    {
    	perror ("VIDIOC_S_EXT_CTRLS");
        printf("Failed enable auto exposure \n");
        auto_exposure = false;
        exit (EXIT_FAILURE);
    }
    else
    {
    	printf("Enable auto exposure successfully\n");
    	auto_exposure = true;
    }
}

static void setExposure(int fh, int exposure)
{
    struct v4l2_ext_controls ext_ctrls;
    struct v4l2_ext_control ext_ctrl;
    CLEAR(ext_ctrls);
    CLEAR(ext_ctrl);

    ext_ctrl.id = V4L2_CID_EXPOSURE_ABSOLUTE;
    ext_ctrl.value = exposure;
    ext_ctrls.ctrl_class = V4L2_CTRL_ID2CLASS(ext_ctrl.id);
    ext_ctrls.count = 1;
    ext_ctrls.controls = &ext_ctrl;
    if (-1 == xioctl (fh,VIDIOC_S_EXT_CTRLS, &ext_ctrls))
    {
        perror ("VIDIOC_S_EXT_CTRLS");
        printf("Failed set V4L2_CID_EXPOSURE_ABSOLUTE \n");
        exit (EXIT_FAILURE);
    }
    else printf("Set exposure to %d successfully\n",exposure);
}

static void setZoom(int fh, int zoom)
{
    struct v4l2_ext_controls ext_ctrls;
    struct v4l2_ext_control ext_ctrl;
    CLEAR(ext_ctrls);
    CLEAR(ext_ctrl);

    ext_ctrl.id = V4L2_CID_ZOOM_ABSOLUTE;
    ext_ctrl.value = zoom;
    ext_ctrls.ctrl_class = V4L2_CTRL_ID2CLASS(ext_ctrl.id);
    ext_ctrls.count = 1;
    ext_ctrls.controls = &ext_ctrl;
    if (-1 == xioctl (fh,VIDIOC_S_EXT_CTRLS, &ext_ctrls))
    {
        perror ("VIDIOC_S_EXT_CTRLS");
        printf("Failed set V4L2_CID_ZOOM_ABSOLUTE \n");
        exit (EXIT_FAILURE);
    }
    else printf("Set zoom to %d successfully\n",zoom);
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
//	opencl* gpu = new opencl();
//	gpu->gpu_init();

	Trajectory trajectory;
    IplImage *img = imAcqGetImg(imAcq);

    //Get the image size
    //GetImageSize(img);

    //Initialize OpenCL
//    ocl::getOpenCLDevices()
    //Create the matrices
    Mat color;
    cv::ocl::oclMat color_ocl;
    Mat grey;
    cv::ocl::oclMat grey_ocl;

    //Initialize the matirces
    color = Mat(img, false);
    color_ocl.upload(color);
    ocl::cvtColor(color_ocl , grey_ocl, CV_BGR2GRAY);
    //ocl::cvtColor(color_ocl , grey_ocl, CV_RGB2GRAY);
    grey_ocl.download(grey);

    //
    const int fps_size = 150;
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
    double bestsharpness = 0;
    double lastsharpness = 0;
    int bestfocus = 0;
    int initsize;
    int lastsize;
    int focusCount = 0;
    int focusThreshold = 5;
    int focusChange = 5;
    int initfocus = 0;
    int errorCount = 0;
    int focusend = 200;
    bool init = false;
    bool changing = false;
    setFocus(fh,focus);

    //Exposure init
    bool auto_exposure = true;
    //disableAutoExposure(fh, auto_exposure);
    int exposure = 500;
    //setExposure(fh, exposure);

    while(imAcqHasMoreFrames(imAcq))
    {

        std::cout << "Focus value is " << focus << std::endl;

        tick_t procInit, procFinal;
        double tic = cvGetTickCount();

        double freq = cvGetTickFrequency()*1000.0;

        getCPUTick(&procInit);
        img = imAcqGetImg(imAcq);
        getCPUTick(&procFinal);
        PRINT_TIMING("Update Time", procInit, procFinal, "\n");


        //update the image matrices
        getCPUTick(&procInit);
        color = Mat(img, false);
        color_ocl.upload(color);
        ocl::oclMat grey_rgb_ocl;
        Mat grey_rgb;
        ocl::cvtColor(color_ocl, grey_rgb_ocl, CV_RGB2GRAY);
        ocl::cvtColor(color_ocl, grey_ocl, CV_BGR2GRAY);
        grey_rgb_ocl.download(grey_rgb);
        grey_ocl.download(grey);
        getCPUTick(&procFinal);
        PRINT_TIMING("Upload/Download Time", procInit, procFinal, "\n");



        if(!reuseFrameOnce)
        {


            if(img == NULL)
            {
                printf("current image is NULL, assuming end of input.\n");
                break;
            }

            //
            //cvtColor(cv::Mat(img), grey, CV_BGR2GRAY);
        }



        if(!skipProcessingOnce)
        {

            //getCPUTick(&procInit);
            //tld->processImage(img);
            tld->processImage(grey_rgb, grey_rgb_ocl);
        	//tld->processImage(color, color_ocl, grey, grey_ocl);
            //getCPUTick(&procFinal);
            //PRINT_TIMING("FrameProcTime", procInit, procFinal, "\n");

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


        std::cout << "toc is " << toc*1000.0 << std::endl;

        float fps = 1 / toc;

        /*
        for(int i = 0; i < fps_size - 1; i ++)
        {
        	fps_array[i] = fps_array[i+1];
        	fps_array[fps_size - 1] = fps;
        }
        data = drawFloatGraph(fps_array, fps_size, 0, 0.0, 50.0);
        */

        int confident = (tld->currConf >= threshold) ? 1 : 0;

        if(showOutput || saveDir != NULL)
        {
        	/*
            char string[128];
            char learningString[10] = "";
            if(tld->learning)
            {
                strcpy(learningString, "Learning");
            }
            //sprintf(string, "#%d,Posterior %.2f; fps: %.2f, #numwindows:%d, %s", imAcq->currentFrame - 1, tld->currConf, fps, tld->detectorCascade->numWindows, learningString);
            sprintf(string, "Frame: %d, Confidence %.2f; Frame Process Time: %.2f ms, Number of Windows: %d, %s", imAcq->currentFrame - 1, tld->currConf, toc, tld->detectorCascade->numWindows, learningString);
            */
            CvScalar yellow = CV_RGB(255, 255, 0);
            CvScalar blue = CV_RGB(0, 0, 255);  
            CvScalar black = CV_RGB(0, 0, 0);
            CvScalar white = CV_RGB(255, 255, 255);
            //Mat imgt = img;
            Rect currRect;
            if(tld->currBB != NULL)
            {
                CvScalar rectangleColor = (confident) ? blue : yellow;
                //define the rectangle of the detected object
                cvRectangle(img, CvPoint(tld->currBB->tl()), CvPoint(tld->currBB->br()), rectangleColor, 4, 8, 0);
                
                currRect = *(tld->currBB);

                //Focus procedure

                getCPUTick(&procInit);
                //double sharpness = contrast_measure(imgt (currRect));//TODO
                double sharpness = contrast_measure(color_ocl (currRect));
                getCPUTick(&procFinal);
                PRINT_TIMING("Contrast Measure Time", procInit, procFinal, ",");

                printf("sharpness is %lf\n",sharpness);

                if(!init)
                {
//
                	if(sharpness > lastsharpness)
                    {   
                        //printf("sharpness is %lf,lastsharpness is %lf,focus is %d,bestfocus is %d\n",sharpness,lastsharpness,focus,bestfocus);
                        if(sharpness > bestsharpness)
                        {
                            bestsharpness = sharpness;
                            bestfocus = focus;
                        }
                        lastsharpness = sharpness;
                    }
                    focusCount++;
                    if(focusCount % 3 == 0)
                    {
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
                    errorCount+= abs(currRect.height-lastsize);
                    lastsize = currRect.height;
                    if(focusCount%10 == 0)
                    {
//                         printf("distense is %d,errorcount is %d\n",lastsize-initsize,errorcount);
                        if(errorCount < 5 && abs(lastsize - initsize) > 5)
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
                                focusChange = focusThreshold;
                                focusend = 200;
                            }
                            else
                            {
                                focusChange = -focusThreshold;
                                focusend = 0;
                            }

                        }
                        errorCount=0;
                        focusCount=0;
                    }                    
                }


                //setExposure(fh, exposure);
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
			/*
            CvFont font;
            cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, .5, .5, 0, 1, 8);
            cvRectangle(img, cvPoint(0, 0), cvPoint(img->width, 50), black, CV_FILLED, 8, 0);
            cvPutText(img, string, cvPoint(25, 25), &font, white);
            */

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
            	CvFont font;
            	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 1, 8);
            	//
                //gui->showImage(img, data);
            	if(gui->getShowData() == true)
            	{
            		char string[128];
            		char learningString[10] = "";
            		if(tld->learning)
            		{
            			strcpy(learningString, "Learning");
            		}
            		sprintf(string, "Frame: %d, Confidence %.2f; Frame Process Time: %.2f ms, FPS: %.2f, Number of Windows: %d, %s", imAcq->currentFrame - 1, tld->currConf, toc*1000, fps, tld->detectorCascade->numWindows, learningString);
            		cvRectangle(img, cvPoint(0, 0), cvPoint(img->width, 40), black, CV_FILLED, 8, 0);
            		cvPutText(img, string, cvPoint(25, 25), &font, white);

            		for(int i = 0; i < fps_size - 1; i ++)
            		{
            			fps_array[i] = fps_array[i+1];
            		    fps_array[fps_size - 1] = fps;
            		}
            		data = drawFloatGraph(fps_array, fps_size, 0, 0.0, 50.0);

            		gui->showImage(img,data);
            	}
            	else
            	{

//            		if(currRect.width != 0 && currRect.height != 0)
//            		{
//                		Size br = Size(100,100);
//                		cv::blur(Mat(img)(Range(0, currRect.y), Range::all()), Mat(img)(Range(0, currRect.y), Range::all()), br);
//                		cv::blur(Mat(img)(Range(currRect.y+currRect.height, img->height-1), Range::all()), Mat(img)(Range(currRect.y+currRect.height, img->height-1), Range::all()), br);
//                		cv::blur(Mat(img)(Range(currRect.y, currRect.y+currRect.height), Range(0, currRect.x)), Mat(img)(Range(currRect.y, currRect.y+currRect.height), Range(0, currRect.x)), br);
//                		cv::blur(Mat(img)(Range(currRect.y, currRect.y+currRect.height), Range(currRect.x+currRect.width, img->width-1)), Mat(img)(Range(currRect.y, currRect.y+currRect.height), Range(currRect.x+currRect.width, img->width-1)), br);
//                		gui->showImage(img);
//                    	//gui->showImage(img);
//            		}
//            		else
//            		{
            			gui->showImage(img);
            		//}

            	}
                //
                char key = gui->getKey();

                if(key == 'q') break;
                if(key == 'p')
                {
                	//std::cout << "Focus is " << focus << "!!!!!!!!!!!!!!!!!!" << std::endl;
                	focus = gui->getFocus();
                	setFocus(fh,focus);
                }

                if(key == 'y')
                {
                	focusThreshold = gui->getFocusAccuracy();
                	std::cout << "Focus accuracy is " << focusThreshold << "!!!!!!!!!!!!!!!!!!" << std::endl;
                }

                if(key == 'u')
                {
                	//If manual adjust exposure enabled
                	if(gui->getManualExposure() == true)
                	{
                		if(auto_exposure == true)
                		{
                			disableAutoExposure(fh, auto_exposure);
                		}
                		else
                		{

                		}
                		setExposure(fh, gui->getExposure());
                	}
                	else
                	{
                		enableAutoExposure(fh, auto_exposure);
                	}
                }

                if(key == 'z')
                {
                	//std::cout << "Focus is " << focus << "!!!!!!!!!!!!!!!!!!" << std::endl;
                    setZoom(fh, gui->getZoom());
                }

                if(key == 'o')
                {
                    CvRect box;

                    if(getPointFromUser(img, box, gui) == PROGRAM_EXIT)
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
                    const char* cascadeName = "/home/slilylsu/Desktop/project-repo/haarcascade_frontalface_alt2.xml";
                    //const char* cascadeName = "/home/slilylsu/Desktop/project-repo/haarcascade_smile.xml";
                    //CascadeClassifier  cascade;
                    ocl::OclCascadeClassifier cascade;
                    vector<Rect> faces;
                    if( !cascade.load( cascadeName )  )
                    {
                        printf("ERROR: Could not load classifier cascade: %s\n",cascadeName);
                    }
                    
//                    cascade.detectMultiScale(grey, faces, 1.1,
//                             3, 0 | CV_HAAR_SCALE_IMAGE,
//                             Size(30, 30), Size(0, 0));
                    cascade.detectMultiScale(grey_ocl, faces, 1.1,
                                                 3, 0 /*| CV_HAAR_SCALE_IMAGE*/,
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
                if(key == 'd')
                {
                	CvRect box;
                	const char* cascadeName = "/home/slilylsu/Desktop/project-repo/apple_23_1.xml";
                	ocl::OclCascadeClassifier cascade;
                	vector<Rect> faces;
                	if( !cascade.load( cascadeName )  )
                	{
                		printf("ERROR: Could not load classifier cascade: %s\n",cascadeName);
                	}
                	cascade.detectMultiScale(grey_ocl, faces, 1.1,
                                             3,0,
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
                if(key == 'g')
                {
                	CvRect box;
                	const char* cascadeName = "/home/slilylsu/Desktop/project-repo/orange.xml";
                	ocl::OclCascadeClassifier cascade;
                	vector<Rect> faces;
                	if( !cascade.load( cascadeName )  )
                	{
                		printf("ERROR: Could not load classifier cascade: %s\n",cascadeName);
                	}
                	cascade.detectMultiScale(grey_ocl, faces, 1.1,
                                             3,0,
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
