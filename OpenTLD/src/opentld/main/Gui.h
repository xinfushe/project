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

#ifndef GUI_H_
#define GUI_H_

#include <string>

#include <opencv/highgui.h>

//
#include "../../libopentld/imacq/ImAcq.h"
//#include "./GraphUtils.h"
//#include "./GraphUtils.cpp"

namespace tld
{

void GetImageSize(IplImage* img);


class Gui
{
public:
    Gui();
    ~Gui();
    void init(ImAcq* imAcq);
    void showImage(IplImage *image);
    void showImage(IplImage *image, IplImage *data);
    char getKey();
    int getFocus(void);
    int getExposure(void);
    bool getManualExposure(void);
    int getZoom(void);
    bool getShowData(void);
    //void Button1Handler(int state, void* userdata);
    //void Button2Handler(int state, void* userdata);
    //void Button3Handler(int state, void* userdata);
    //void Trackbar1Handler (int pos, void* userdata);
    std::string windowName();


private:
    std::string m_window_name;
    //
    std::string data_window_name;
    //
    //const char* test_window_name;

    //char gui_key;
    //int gui_focus;
};

/**
 * Get a bounding box from the user.
 * @param img image to display
 * @param rect CvRect containing the coordinates of the bounding box
 * @param gui initialized gui
 * @return PROGRAM_EXIT if 'q' or 'Q' pressed, SUCCESS if everything went right
 */
int getBBFromUser(IplImage *img, CvRect &rect, Gui *gui);
int getPointFromUser(IplImage *img, CvRect &rect, Gui *gui);

}

#endif /* GUI_H_ */
