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
	Trajectory trajectory;
    IplImage *img = imAcqGetImg(imAcq);

    //
    int fps_size = 166;
    float fps_array[166] = {0.0};
    IplImage* data;
    //

    //
    //Mat img_m = img;
    //cv::ocl::oclMat img_oclm(img_m);
    //cv::ocl::oclMat img_oclm(img);
    Mat grey(img->height, img->width, CV_8UC1);
    cvtColor(cv::Mat(img), grey, CV_BGR2GRAY);
    //Mat grey = Mat(grey_oclm);


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
        //double tic = cvGetTickCount();

        //
        img = imAcqGetImg(imAcq);
        // modify at 10.09 delete the following two lines
        //img_m = img;
        //img_oclm.upload(img_m);

        //cv::ocl::cvtColor(img_oclm, grey_oclm, CV_BGR2GRAY);

        if(!reuseFrameOnce)
        {
            //img = imAcqGetImg(imAcq);
            //cv::ocl::oclMat oclimg = cv::ocl::oclMat(img);

            if(img == NULL)
            {
                printf("current image is NULL, assuming end of input.\n");
                break;
            }

            //
            cvtColor(cv::Mat(img), grey, CV_BGR2GRAY);
        }

        double tic = cvGetTickCount();
        if(!skipProcessingOnce)
        {
        	//
        	//cv::ocl::oclMat oclimg = cv::ocl::oclMat(img);
            //getCPUTick(&procInit);
            tld->processImage(img);//
            //getCPUTick(&procFinal);
            //PRINT_TIMING("FrameProcTime", procInit, procFinal, "\n");
        }
        else
        {
            skipProcessingOnce = false;
        }
        double toc = (cvGetTickCount() - tic) / cvGetTickFrequency();

        toc = toc / 1000000;

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

        // double toc = (cvGetTickCount() - tic) / cvGetTickFrequency();

        // toc = toc / 1000000;
        std::cout << "toc is " << toc*1000.0 << std::endl;

        float fps = 1 / toc;

        //
        for(int i = 0; i < fps_size - 1; i ++)
        {
        	fps_array[i] = fps_array[i+1];
        	fps_array[fps_size - 1] = fps;
        }
        data = drawFloatGraph(fps_array, fps_size, 0, 0.0, 50.0);
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
                //
                //cv::ocl::oclMat oclimg = cv::ocl::oclMat(img);
                //Should there be a space between imgt and currRect?
                getCPUTick(&procInit);
                double sharpness = contrast_measure(cv::ocl::oclMat(imgt) (currRect));//TODO
                
            
            getCPUTick(&procFinal);
            PRINT_TIMING("FocusTime", procInit, procFinal, "\n");
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
                    const char* cascadeName = "/home/slilylsu/Desktop/project-repo/haarcascade_frontalface_alt.xml";
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
