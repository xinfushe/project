//#include <WINDOWS.H>
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
CvFont font;
IplImage* frame = 0;
CvCapture* capture;

using namespace std;

int main()
{
    CvCapture* capture=cvCaptureFromCAM(1);
    cvNamedWindow("Cam Feed",CV_WINDOW_NORMAL);

    while(1)
    {
        //Getting the present frame
        //frame=cvQueryFrame(capture);

    	capture = cvCaptureFromCAM(1);

        //cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH, 1920);

        //cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT, 1080 );

        //cvSetCaptureProperty(capture, CV_CAP_PROP_FPS, 24 );

        //frame = cvQueryFrame(capture);
        //printf("camera: %f x %f\n", cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH),cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT));
        //BattleMenu();

        //cvResizeWindow("Cam Feed", 1024, 768);

        //cvMoveWindow("Cam Feed", 400, 130);
        //cvWaitKey(1);
        //cvShowImage("Cam Feed",frame);
    }

    /* free memory */
    cvDestroyWindow( "Cam Feed" );
    cvReleaseCapture( &capture );

    return 0;
}
