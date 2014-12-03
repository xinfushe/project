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
 * gui.cpp
 *
 *  Created on: Oct 18, 2011
 *      Author: clemensk
 */

#include "Gui.h"

#include "Main.h"

#include <string>


using std::string;


char gui_key;
int gui_focus;
bool gui_manualfocus;
int gui_exposure;
bool gui_manualexposure;
bool gui_manualexposure_prev;
static int image_size [2] = {0, 0};

namespace tld
{

Gui::Gui() :
    m_window_name("OpenTLD with OpenCL acceleration"),
	//
	data_window_name("Data")
	//gui_key('0'),
	//gui_focus(0)
	//
	//test_window_name("test window")
{
}

Gui::~Gui()
{
}

//Face Detection
void Button1Handler(int state, void* userdata)
{
	if (state == 0)
	{
		*(char*) userdata = 'f';
	}
}

//Apple Detection
void Button2Handler(int state, void* userdata)
{
	if (state == 0)
	{
		*(char*) userdata = 'd';
	}
}

//Select Object to Detect
void Button3Handler(int state, void* userdata)
{
	if (state == 0)
	{
		*(char*) userdata = 'r';
	}
}

//Select Point to Detect
void Button4Handler(int state, void* userdata)
{
	if (state == 0)
	{
		*(char*) userdata = 'o';
	}
}

//Focus
void Trackbar1Handler (int pos, void* userdata)
{
	const double max = 100.0;
	const double min = -10.0;

	*(int*) userdata = (int)(((double) pos/ 1000.0) * (max - min) + min);

	if(gui_manualfocus == true)
	{
		gui_key = 'p';
	}

}

//Manual Focus
void Button5Handler(int state, void* userdata)
{
	if (state == 0)
	{
		*(bool*) userdata = false;
	}
	else
	{
		*(bool*) userdata = true;
	}
	if(state == 0)
	{
		gui_key = 'p';
	}
}

//Exposure
void Trackbar2Handler (int pos, void* userdata)
{
	const double max = 2000.0;
	const double min = 0.0;

	*(int*) userdata = (int)(((double) pos/ 1000.0) * (max - min) + min);

	if(gui_manualexposure == true)
	{
		gui_key = 'u';
	}

}

//Manual Exposure
void Button6Handler(int state, void* userdata)
{

	if(state == 0 && gui_manualexposure_prev == true)
	{
		gui_key = 'u';
	}
	gui_manualexposure_prev = state;

	if (state == 0)
	{
		*(bool*) userdata = false;
	}
	else
	{
		*(bool*) userdata = true;
	}


}

//Reset
void Button7Handler(int state, void* userdata)
{
	if (state == 0)
	{
		*(char*) userdata = 'y';
	}
}

static string window_name_1;
static CvFont font_1;
static IplImage *img0_1;
//static IplImage *img1_1;
static CvPoint point_1;
static CvRect *bb_1;
static int pixel_threshold[2] = {10,10};
//static int drag_1 = 0;

static void mouseHandler1(int event, int x, int y, int flags, void *param)
{
    /* user press left button */
    if(event == CV_EVENT_LBUTTONDOWN /*&& !drag_18*/)
    {
        point_1 = cvPoint(x, y);
        //drag_1 = 1;
    }

    /* user drag the mouse */
//    if(event == CV_EVENT_MOUSEMOVE && drag_1)
//    {
//        img1_1 = (IplImage *) cvClone(img0_1);
//
//        cvRectangle(img1_1, point_1, cvPoint(x, y), CV_RGB(255, 0, 0), 1, 8, 0);
//
//        cvShowImage(window_name_1.c_str(), img1_1);
//        cvReleaseImage(&img1_1);
//    }

    /* user release left button */
    if(event == CV_EVENT_LBUTTONUP /*&& drag_1*/)
    {

    	int x1 = point_1.x - pixel_threshold[0];
    	int y1 = point_1.y - pixel_threshold[1];
    	int x2 = point_1.x + pixel_threshold[0];
    	int y2 = point_1.y + pixel_threshold[1];
    	if (x1 < 0)
    	{
    		x1 = 0;
    	}
    	if(y1 < 0)
    	{
    		y1 = 0;
    	}
    	if(x2 > image_size[0] - 1)
    	{
    		x2 = image_size[0] - 1;
    	}
    	if(y2 > image_size[1] - 1)
    	{
    		y2 = image_size[1] - 1;
    	}

        *bb_1 = cvRect(x1, y1, x2 - x1, y2 - y1);
        //drag_1 = 0;
    }

    //
}


int getPointFromUser(IplImage *img, CvRect &rect, Gui *gui)
{
    window_name_1 = gui->windowName();
    img0_1 = (IplImage *) cvClone(img);
    rect = cvRect(-1, -1, -1, -1);
    bb_1 = &rect;
    bool correctBB = false;
    cvInitFont(&font_1, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 1, 8);

    cvSetMouseCallback(window_name_1.c_str(), mouseHandler1, NULL);
    cvPutText(img0_1, "Click on a point and press Enter", cvPoint(0, 60),
              &font_1, cvScalar(255, 255, 0));
    cvShowImage(window_name_1.c_str(), img0_1);

    while(!correctBB)
    {
        char key = cvWaitKey(0);

        if(tolower(key) == 'q')
        {
            return PROGRAM_EXIT;
        }

        if(((key == '\n') || (key == '\r') || (key == '\r\n')) && (bb_1->x != -1) && (bb_1->y != -1))
        {
            correctBB = true;
        }
    }

//    if(rect.width < 0)
//    {
//        rect.x += rect.width;
//        rect.width = abs(rect.width);
//    }
//
//    if(rect.height < 0)
//    {
//        rect.y += rect.height;
//        rect.height = abs(rect.height);
//    }

    cvSetMouseCallback(window_name_1.c_str(), NULL, NULL);

    cvReleaseImage(&img0_1);
    //cvReleaseImage(&img1_1);

    return SUCCESS;
}

int Gui::getFocus(void)
{
	//std::cout << gui_focus << std::endl;
	return gui_focus;
}

int Gui::getExposure(void)
{
	//std::cout << gui_exposure << std::endl;
	return gui_exposure;
}

bool Gui::getManualExposure(void)
{
	return gui_manualexposure;
}

void Gui::init(ImAcq* imAcq)
{
	gui_key = '0';
	gui_focus = 0;
	gui_manualfocus = false;
	gui_exposure = 500;
	gui_manualexposure = false;
	gui_manualexposure_prev = true;

    cvNamedWindow(m_window_name.c_str(), CV_WINDOW_AUTOSIZE);
    cvMoveWindow(m_window_name.c_str(), 0, 0);
    //cvResizeWindow(m_window_name.c_str(),1800, 450);
    //
    cvNamedWindow(data_window_name.c_str(), CV_WINDOW_AUTOSIZE | CV_GUI_NORMAL);
    cvMoveWindow(data_window_name.c_str(), 870, 0);
    //cvResizeWindow(m_window_name.c_str(),imAcq->width, imAcq->height);
    //cvNamedWindow(test_window_name, CV_WINDOW_AUTOSIZE | CV_GUI_NORMAL);
    //cvMoveWindow(test_window_name, 900, 350);
    //const char* button_name = "test";
    //CvButtonCallback on_change;

    const char* button1 = "Face Detection";
    cvCreateButton(button1, Button1Handler, &gui_key, CV_PUSH_BUTTON, 1);

    const char* button2 = "Apple Detection";
    cvCreateButton(button2, Button2Handler,&gui_key,CV_PUSH_BUTTON,1);

    const char* button3 = "Select Object To Detect";
    cvCreateButton(button3, Button3Handler, &gui_key, CV_PUSH_BUTTON, 1);

    const char* button4 = "Select Point to Detect";
    cvCreateButton(button4, Button4Handler,&gui_key,CV_PUSH_BUTTON,1);

    int focus_value = 500;
    cvCreateTrackbar2("Focus", NULL, &focus_value, 1000,  Trackbar1Handler, &gui_focus);

    const char* button5 = "Manual Focus";
    cvCreateButton(button5, Button5Handler, &gui_manualfocus, CV_CHECKBOX, 0);

    int exposure_value = 500;
    cvCreateTrackbar2("Exposure", NULL, &exposure_value, 1000,  Trackbar2Handler, &gui_exposure);

    const char* button6 = "Manual Exposure";
    cvCreateButton(button6, Button6Handler, &gui_manualexposure, CV_CHECKBOX, 0);

    const char* button7 = "Reset";
    cvCreateButton(button7, Button7Handler, &gui_key, CV_PUSH_BUTTON, 1);

}

void GetImageSize(IplImage* img)
{
	image_size[0] = img->width;
	image_size[1] = img->height;
	std::cout << "width is " << image_size[0] << " and height is " << image_size[1] << std::endl;
}

void Gui::showImage(IplImage *image, IplImage *data)
{
    cvShowImage(m_window_name.c_str(), image);
    /*
    float a[8] = {1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0};
    showFloatGraph("123", a, 8);
    */
    cvShowImage(data_window_name.c_str(), data);

}

char Gui::getKey()
{
	//return cvWaitKey(10);

	char temp = cvWaitKey(10);
	if(gui_key != '0')
	{
		//printf("button key is %c", gui_key);
		temp = gui_key;
		gui_key = '0';
	}
	return temp;
}

std::string Gui::windowName()
{
    return m_window_name;
}

static string window_name;
static CvFont font;
static IplImage *img0;
static IplImage *img1;
static CvPoint point;
static CvRect *bb;
static int drag = 0;

static void mouseHandler(int event, int x, int y, int flags, void *param)
{
    /* user press left button */
    if(event == CV_EVENT_LBUTTONDOWN && !drag)
    {
        point = cvPoint(x, y);
        drag = 1;
    }

    /* user drag the mouse */
    if(event == CV_EVENT_MOUSEMOVE && drag)
    {
        img1 = (IplImage *) cvClone(img0);

        cvRectangle(img1, point, cvPoint(x, y), CV_RGB(255, 0, 0), 1, 8, 0);

        cvShowImage(window_name.c_str(), img1);
        cvReleaseImage(&img1);
    }

    /* user release left button */
    if(event == CV_EVENT_LBUTTONUP && drag)
    {
        *bb = cvRect(point.x, point.y, x - point.x, y - point.y);
        drag = 0;
    }

    //
}

// TODO: member of Gui
// --> problem: callback function mouseHandler as member!
int getBBFromUser(IplImage *img, CvRect &rect, Gui *gui)
{
    window_name = gui->windowName();
    img0 = (IplImage *) cvClone(img);
    rect = cvRect(-1, -1, -1, -1);
    bb = &rect;
    bool correctBB = false;
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 1, 8);

    cvSetMouseCallback(window_name.c_str(), mouseHandler, NULL);
    cvPutText(img0, "Draw a bounding box and press Enter", cvPoint(0, 60),
              &font, cvScalar(255, 255, 0));
    cvShowImage(window_name.c_str(), img0);

    while(!correctBB)
    {
        char key = cvWaitKey(0);

        if(tolower(key) == 'q')
        {
            return PROGRAM_EXIT;
        }

        if(((key == '\n') || (key == '\r') || (key == '\r\n')) && (bb->x != -1) && (bb->y != -1))
        {
            correctBB = true;
        }
    }

    if(rect.width < 0)
    {
        rect.x += rect.width;
        rect.width = abs(rect.width);
    }

    if(rect.height < 0)
    {
        rect.y += rect.height;
        rect.height = abs(rect.height);
    }

    cvSetMouseCallback(window_name.c_str(), NULL, NULL);

    cvReleaseImage(&img0);
    cvReleaseImage(&img1);

    return SUCCESS;
}
}
