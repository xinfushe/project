#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <cctype>
#include <iostream>
#include <iterator>
#include <stdio.h>
#include <cmath>
#include <vector>
#include <deque>

using namespace std;
using namespace cv;

/**
 * MyCircle Object
 */
class MyCircle
{
public:
    Point center;
    int radius;
};


static void help()
{
    cout << "\nThis program demonstrates the cascade recognizer. Now you can use Haar or LBP features.\n"
            "This classifier can recognize many kinds of rigid objects, once the appropriate classifier is trained.\n"
            "It's most known use is for faces.\n"
            "Usage:\n"
            "./facedetect [--cascade=<cascade_path> this is the primary trained classifier such as frontal face]\n"
               "   [--nested-cascade[=nested_cascade_path this an optional secondary classifier such as eyes]]\n"
               "   [--scale=<image scale greater or equal to 1, try 1.3 for example>]\n"
               "   [--try-flip]\n"
               "   [filename|camera_index]\n\n"
            "see facedetect.cmd for one call:\n"
            "./facedetect --cascade=\"../../data/haarcascades/haarcascade_frontalface_alt.xml\" --nested-cascade=\"../../data/haarcascades/haarcascade_eye.xml\" --scale=1.3\n\n"
            "During execution:\n\tHit any key to quit.\n"
            "\tUsing OpenCV version " << CV_VERSION << "\n" << endl;
}

void detectAndDraw( Mat& img, CascadeClassifier& cascade,
                    CascadeClassifier& nestedCascade,
                    double scale, bool tryflip );

/**
 * Rotate an image
 */
void rotate(cv::Mat& src, double angle, cv::Mat& dst);
float pointDistance(Point a, Point b);
bool circleSimilar(MyCircle c1, MyCircle c2);
Point rotateOnPivot(Point target, Point pivot, double angle);
void adaptiveDetect(CascadeClassifier &cascade, Mat &box, vector<MyCircle> &faces, Point offset);
void mergeBoxes(vector<Rect> &target);

string cascadeName = "../../data/haarcascades/haarcascade_frontalface_alt.xml";
string nestedCascadeName = "../../data/haarcascades/haarcascade_eye_tree_eyeglasses.xmlaaa";

int main( int argc, const char** argv )
{
    CvCapture* capture = 0;
    Mat frame, frameCopy, image;
    const string scaleOpt = "--scale=";
    size_t scaleOptLen = scaleOpt.length();
    const string cascadeOpt = "--cascade=";
    size_t cascadeOptLen = cascadeOpt.length();
    const string nestedCascadeOpt = "--nested-cascade";
    size_t nestedCascadeOptLen = nestedCascadeOpt.length();
    const string tryFlipOpt = "--try-flip";
    size_t tryFlipOptLen = tryFlipOpt.length();
    string inputName;
    bool tryflip = false;

    help();

    CascadeClassifier cascade, nestedCascade;
    double scale = 1;

    for( int i = 1; i < argc; i++ )
    {
        cout << "Processing " << i << " " <<  argv[i] << endl;
        if( cascadeOpt.compare( 0, cascadeOptLen, argv[i], cascadeOptLen ) == 0 )
        {
            cascadeName.assign( argv[i] + cascadeOptLen );
            cout << "  from which we have cascadeName= " << cascadeName << endl;
        }
        else if( nestedCascadeOpt.compare( 0, nestedCascadeOptLen, argv[i], nestedCascadeOptLen ) == 0 )
        {
            if( argv[i][nestedCascadeOpt.length()] == '=' )
                nestedCascadeName.assign( argv[i] + nestedCascadeOpt.length() + 1 );
            if( !nestedCascade.load( nestedCascadeName ) )
                cerr << "WARNING: Could not load classifier cascade for nested objects" << endl;
        }
        else if( scaleOpt.compare( 0, scaleOptLen, argv[i], scaleOptLen ) == 0 )
        {
            if( !sscanf( argv[i] + scaleOpt.length(), "%lf", &scale ) || scale < 1 )
                scale = 1;
            cout << " from which we read scale = " << scale << endl;
        }
        else if( tryFlipOpt.compare( 0, tryFlipOptLen, argv[i], tryFlipOptLen ) == 0 )
        {
            tryflip = true;
            cout << " will try to flip image horizontally to detect assymetric objects\n";
        }
        else if( argv[i][0] == '-' )
        {
            cerr << "WARNING: Unknown option %s" << argv[i] << endl;
        }
        else
            inputName.assign( argv[i] );
    }

    if( !cascade.load( cascadeName ) )
    {
        cerr << "ERROR: Could not load classifier cascade" << endl;
        help();
        return -1;
    }

    if( inputName.empty() || (isdigit(inputName.c_str()[0]) && inputName.c_str()[1] == '\0') )
    {
        capture = cvCaptureFromCAM( inputName.empty() ? 0 : inputName.c_str()[0] - '0' );
        int c = inputName.empty() ? 0 : inputName.c_str()[0] - '0' ;
        if(!capture) cout << "Capture from CAM " <<  c << " didn't work" << endl;
    }
    else if( inputName.size() )
    {
        image = imread( inputName, 1 );
        if( image.empty() )
        {
            capture = cvCaptureFromAVI( inputName.c_str() );
            if(!capture) cout << "Capture from AVI didn't work" << endl;
        }
    }
    else
    {
        image = imread( "lena.jpg", 1 );
        if(image.empty()) cout << "Couldn't read lena.jpg" << endl;
    }

    cvNamedWindow( "result", 1 );

    if( capture )
    {
        cout << "In capture ..." << endl;
        for(;;)
        {
            IplImage* iplImg = cvQueryFrame( capture );
            frame = iplImg;
            if( frame.empty() )
                break;
            if( iplImg->origin == IPL_ORIGIN_TL )
                frame.copyTo( frameCopy );
            else
                flip( frame, frameCopy, 0 );

            detectAndDraw( frameCopy, cascade, nestedCascade, scale, tryflip );

            if( waitKey( 10 ) >= 0 )
                goto _cleanup_;
        }

        waitKey(0);

_cleanup_:
        cvReleaseCapture( &capture );
    }
    else
    {
        cout << "In image read" << endl;
        if( !image.empty() )
        {
            detectAndDraw( image, cascade, nestedCascade, scale, tryflip );
            waitKey(0);
        }
        else if( !inputName.empty() )
        {
            /* assume it is a text file containing the
            list of the image filenames to be processed - one per line */
            FILE* f = fopen( inputName.c_str(), "rt" );
            if( f )
            {
                char buf[1000+1];
                while( fgets( buf, 1000, f ) )
                {
                    int len = (int)strlen(buf), c;
                    while( len > 0 && isspace(buf[len-1]) )
                        len--;
                    buf[len] = '\0';
                    cout << "file " << buf << endl;
                    image = imread( buf, 1 );
                    if( !image.empty() )
                    {
                        detectAndDraw( image, cascade, nestedCascade, scale, tryflip );
                        c = waitKey(0);
                        if( c == 27 || c == 'q' || c == 'Q' )
                            break;
                    }
                    else
                    {
                        cerr << "Aw snap, couldn't read image " << buf << endl;
                    }
                }
                fclose(f);
            }
        }
    }

    cvDestroyWindow("result");

    return 0;
}

void detectAndDraw( Mat& img, CascadeClassifier& cascade,
                    CascadeClassifier& nestedCascade,
                    double scale, bool tryflip )
{
    int i = 0;
    double t = 0;
    vector<Rect> faces_v, faces_rr, faces_lr, faces2;
    vector<MyCircle> detected;
    Point img_center;
    int rotateAngle = 25;
    cout << "image size: " << img.cols << " " << img.rows << endl;
    const static Scalar colors[] =  { 
        CV_RGB(0,0,255),
        CV_RGB(0,128,255),
        CV_RGB(0,255,255),
        CV_RGB(0,255,0),
        CV_RGB(255,128,0),
        CV_RGB(255,255,0),
        CV_RGB(255,0,0),
        CV_RGB(255,0,255)} ;
    Mat gray, smallImg( cvRound (img.rows/scale), cvRound(img.cols/scale), CV_8UC1 );
    img_center.x = cvRound(img.cols/2.);
    img_center.y = cvRound(img.rows/2.);

    cvtColor( img, gray, CV_BGR2GRAY );
    // resize( gray, smallImg, smallImg.size(), 0, 0, INTER_LINEAR );

    Mat rightRotated, leftRotated;
    // rotate(smallImg, -rotateAngle, rightRotated);
    // rotate(smallImg, rotateAngle, leftRotated);

    equalizeHist( gray, gray );
    // equalizeHist( smallImg, smallImg );
    // equalizeHist( rightRotated, rightRotated );
    // equalizeHist( leftRotated, leftRotated );

    t = (double)cvGetTickCount();

    Mat blurred;
    blur( img, blurred, Size(50,50) );
    Mat hsv;
    cvtColor(blurred, hsv, CV_BGR2HSV);

    Mat bw;
    inRange(hsv, Scalar(0, 10, 60), Scalar(20, 150, 255), bw);

    vector<vector<Point> > contours, contours_l, contours_r;
    vector<Vec4i> hierarchy, hierarchy_l, hierarchy_r;
    findContours( bw, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

    vector<Rect> boxes;
    for (int i = 0; i < contours.size(); i++) 
    {
        Rect bRect = boundingRect(contours[i]);
        if (bRect.width * bRect.height > 1000) 
        {
            boxes.push_back(bRect);
        }
    }

    for (int i = 0; i < boxes.size(); i++) 
    {
        cout << "Orig " << i << ": " << boxes[i] << endl;
    }

    // TODO: Lili, put your code here for merging boxes
    // Note: boxes with area less than 1000 has already been eliminated
    // e.g.: mergingBoxes(boxes);
    // mergeBoxes(boxes);

    for (int i = 0; i < boxes.size(); i++) 
    {
        cout << "New " << i << ": " << boxes[i] << endl;
    }

    for (int i = 0; i < boxes.size(); i++) {
        Rect bRect = boxes[i];
        cout << "orig: " << bRect.x << " " << bRect.y << " " << bRect.width << " " << bRect.height << endl;
        bRect.x = max(bRect.x-cvRound(bRect.width*0.25), 0);
        bRect.y = max(bRect.y-cvRound(bRect.height*0.25), 0);
        bRect.width = min(cvRound(bRect.width*1.5), img.cols - bRect.x);
        bRect.height = min(cvRound(bRect.height*1.5), img.rows - bRect.y);
        cout << "large: " << bRect.x << " " << bRect.y << " " << bRect.width << " " << bRect.height << endl;
    
        Mat cropped = gray(bRect);
        rectangle( img, cvPoint(cvRound(bRect.x), cvRound(bRect.y)),
                   cvPoint(cvRound(bRect.x + bRect.width), cvRound(bRect.y + bRect.height)),
                   CV_RGB(0,0,255), 1, 8, 0);
        adaptiveDetect(cascade, cropped, detected, Point(bRect.x, bRect.y));
    }
    // adaptiveDetect(cascade, gray, detected, Point(0, 0));

    // adaptiveDetect(cascade, gray, detected);


    // put convert rect into circle
    

    // // detect right rotated
    // cascade.detectMultiScale( rightRotated, faces_rr,
    //     1.2, 2, 0
    //     //|CV_HAAR_FIND_BIGGEST_OBJECT
    //     //|CV_HAAR_DO_ROUGH_SEARCH
    //     |CV_HAAR_SCALE_IMAGE
    //     ,
    //     initSize);
    // for (vector<Rect>::const_iterator r = faces_rr.begin(); r != faces_rr.end(); r++)
    // {
    //     MyCircle cir;
    //     Point new_center;
    //     cir.center.x = cvRound((r->x + r->width*0.5)*scale);
    //     cir.center.y = cvRound((r->y + r->height*0.5)*scale);
    //     cir.radius = cvRound((r->width + r->height)*0.25*scale);
    //     new_center = rotateOnPivot(cir.center, img_center, -rotateAngle-5);
    //     cir.center = new_center;

    //     // search for operlaping circles
    //     bool has_overlap = false;
    //     for( vector<MyCircle>::const_iterator mc = detected.begin(); mc != detected.end(); mc++, i++ )
    //     {
    //         if (circleSimilar(*mc, cir)) 
    //         {
    //             has_overlap = true;
    //             break;
    //         }
    //     }
    //     if (!has_overlap) 
    //     {
    //         detected.push_back(cir);
    //     }
    // }

    // // detect left rotated
    // cascade.detectMultiScale( leftRotated, faces_lr,
    //     1.2, 2, 0
    //     //|CV_HAAR_FIND_BIGGEST_OBJECT
    //     //|CV_HAAR_DO_ROUGH_SEARCH
    //     |CV_HAAR_SCALE_IMAGE
    //     ,
    //     initSize);
    // for (vector<Rect>::const_iterator r = faces_lr.begin(); r != faces_lr.end(); r++)
    // {
    //     MyCircle cir;
    //     Point new_center;
    //     cir.center.x = cvRound((r->x + r->width*0.5)*scale);
    //     cir.center.y = cvRound((r->y + r->height*0.5)*scale);
    //     cir.radius = cvRound((r->width + r->height)*0.25*scale);
    //     new_center = rotateOnPivot(cir.center, img_center, rotateAngle+5);
    //     cir.center = new_center;
        
    //     // search for operlaping circles
    //     bool has_overlap = false;
    //     for( vector<MyCircle>::const_iterator mc = detected.begin(); mc != detected.end(); mc++, i++ )
    //     {
    //         if (circleSimilar(*mc, cir)) 
    //         {
    //             has_overlap = true;
    //             break;
    //         }
    //     }
    //     if (!has_overlap) 
    //     {
    //         detected.push_back(cir);
    //     }
    // }
    // for( vector<Rect>::const_iterator r = faces_lr.begin(); r != faces_lr.end(); r++ )
    // {
    //     faces_v.push_back(Rect(r->x, r->y, r->width, r->height));
    // }

    if( tryflip )
    {
        flip(smallImg, smallImg, 1);
        cascade.detectMultiScale( smallImg, faces2,
                                 1.1, 2, 0
                                 //|CV_HAAR_FIND_BIGGEST_OBJECT
                                 //|CV_HAAR_DO_ROUGH_SEARCH
                                 |CV_HAAR_SCALE_IMAGE
                                 ,
                                 Size(30, 30) );
        for( vector<Rect>::const_iterator r = faces2.begin(); r != faces2.end(); r++ )
        {
            faces_v.push_back(Rect(smallImg.cols - r->x - r->width, r->y, r->width, r->height));
        }
    }
    t = (double)cvGetTickCount() - t;
    printf( "detection time = %g ms\n", t/((double)cvGetTickFrequency()*1000.) );
    for( vector<MyCircle>::const_iterator r = detected.begin(); r != detected.end(); r++, i++ )
    {
        Mat smallImgROI;
        vector<Rect> nestedObjects;
        Point center;
        Scalar color = colors[i%8];
        int radius;

        circle( img, r->center, r->radius, color, 3, 8, 0 );


        // double aspect_ratio = (double)r->width/r->height;
        // if( 0.75 < aspect_ratio && aspect_ratio < 1.3)
        // {
        //     center.x = cvRound((r->x + r->width*0.5)*scale);
        //     center.y = cvRound((r->y + r->height*0.5)*scale);
        //     radius = cvRound((r->width + r->height)*0.25*scale);
        //     circle( img, center, radius, color, 3, 8, 0 );
        // }
        // else
        //     rectangle( img, cvPoint(cvRound(r->x*scale), cvRound(r->y*scale)),
        //                cvPoint(cvRound((r->x + r->width-1)*scale), cvRound((r->y + r->height-1)*scale)),
        //                color, 3, 8, 0);
        // if( nestedCascade.empty() )
        //     continue;
        // smallImgROI = smallImg(*r);
        // nestedCascade.detectMultiScale( smallImgROI, nestedObjects,
        //     1.1, 2, 0
        //     //|CV_HAAR_FIND_BIGGEST_OBJECT
        //     //|CV_HAAR_DO_ROUGH_SEARCH
        //     //|CV_HAAR_DO_CANNY_PRUNING
        //     |CV_HAAR_SCALE_IMAGE
        //     ,
        //     Size(30, 30) );
        // for( vector<Rect>::const_iterator nr = nestedObjects.begin(); nr != nestedObjects.end(); nr++ )
        // {
        //     center.x = cvRound((r->x + nr->x + nr->width*0.5)*scale);
        //     center.y = cvRound((r->y + nr->y + nr->height*0.5)*scale);
        //     radius = cvRound((nr->width + nr->height)*0.25*scale);
        //     circle( img, center, radius, color, 3, 8, 0 );
        // }
    }
    cv::imshow( "result", img );
}

void rotate(cv::Mat& src, double angle, cv::Mat& dst)
{
    int len = std::max(src.cols, src.rows);
    cv::Point2f pt(len/2., len/2.);
    cv::Mat r = cv::getRotationMatrix2D(pt, angle, 1.0);

    cv::warpAffine(src, dst, r, cv::Size(len, len));
}

float pointDistance(Point a, Point b) {
    return cvSqrt((a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y));
}

bool circleSimilar(MyCircle c1, MyCircle c2)
{
    float diff_rate = 0.2;
    if (pointDistance(c1.center, c2.center) < (float)(c2.radius) && 
        pointDistance(c1.center, c2.center) < (float)(c1.radius) &&
        (c1.radius-c2.radius)*(c1.radius-c2.radius)/(c1.radius*c2.radius) < diff_rate*diff_rate)
    {
        return true;
    }
    else
    {
        return false;
    }   
}

Point rotateOnPivot(Point target, Point pivot, double angle)
{
    cout << "before: " << target.x << " " << target.y << endl;
    cout << "pivot: " << pivot.x << " " << pivot.y << endl;
    Point translated;
    translated.x = target.x-pivot.x;
    translated.y = target.y-pivot.y;

    float rotated_x, rotated_y;
    rotated_x = (float)translated.x*cos(angle/57.3) - (float)translated.y*sin(angle/57.3);
    rotated_y = (float)translated.x*sin(angle/57.3) + (float)translated.y*cos(angle/57.3);

    Point ret;
    ret.x = cvRound(rotated_x + pivot.x);
    ret.y = cvRound(rotated_y + pivot.y);

    cout << "after: " << ret.x << " " << ret.y << endl;
    return ret;
}

void adaptiveDetect(CascadeClassifier &cascade, Mat &box, vector<MyCircle> &faces, Point offset)
{
    vector<Rect> facesRect;
    int initLen = cvRound(min(box.rows, box.cols)/1.5);
    initLen = max(initLen, 30);
    Size initSize(initLen, initLen);
    cascade.detectMultiScale( box, facesRect, 1.05, 2, 0|CV_HAAR_SCALE_IMAGE, initSize);
    for (vector<Rect>::const_iterator r = facesRect.begin(); r != facesRect.end(); r++)
    {
        MyCircle cir;
        cir.center.x = cvRound(r->x + r->width*0.5 + offset.x);
        cir.center.y = cvRound(r->y + r->height*0.5 + offset.y);
        cir.radius = cvRound((r->width + r->height)*0.25);
        faces.push_back(cir);
    }
}

void mergeBoxes(vector<Rect> &target) {
    deque<Rect> mergedBoxes;
    deque<Rect> unmergedBoxes(target.begin(), target.end());
    Rect mergedBox;

    while (unmergedBoxes.size()) {
        mergedBox = unmergedBoxes.front();
        unmergedBoxes.pop_front();

        int index = 0;
        while (index < mergedBoxes.size()) {
            Rect box = mergedBoxes.front();
            mergedBoxes.pop_front();
            if (max(box.x, mergedBox.x) > min(box.x + box.width, mergedBox.x + mergedBox.width) ||
                max(box.y, mergedBox.y) > min(box.y + box.height, mergedBox.y + mergedBox.height)) {
                mergedBoxes.push_back(box);
                index++;
            } else {
                Rect newMergedBox;
                
                newMergedBox.x = min(box.x, mergedBox.x);
                newMergedBox.width = max(box.x + box.width, mergedBox.x + mergedBox.width) - newMergedBox.x;
                newMergedBox.y = min(box.y, mergedBox.y);
                newMergedBox.height = max(box.y + box.height, mergedBox.y + mergedBox.height) - newMergedBox.y;
                mergedBox = newMergedBox;
                index = 0;
            }
        }
        mergedBoxes.push_back(mergedBox);
    }
    target.clear();
    target.insert(target.end(), mergedBoxes.begin(), mergedBoxes.end());
}
