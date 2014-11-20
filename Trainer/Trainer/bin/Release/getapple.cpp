#include "cv.h"
#include "highgui.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>
#include "opencv2/opencv.hpp"

int main()
{
static CvHaarClassifierCascade* cascade = 0;
const char *cascadeName=">Desktop/Trainer/Trainer/bin/Release/data.xml";
IplImage* img=cvLoadImage("pos/pos1.jpg");
cvCvtColor(img,img,CV_BGR2RGB);
cvReleaseImage(&img);
CvHaarClassifierCascade *cascade=(CvHarrClassifierCascade*)cvLoad(cascadeName,0,0,0);
if(cascade)
return 0;
else 
return 1;
}
