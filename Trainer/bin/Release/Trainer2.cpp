//the directory to look for images in
const char *POS_IMAGES_DIR = "./bin/Release/pos/";
const char *POS_IMAGES_FOLDER = "bin/Release/pos/";
const char *NEG_IMAGES_DIR = "./bin/Release/neg/";
const char *NEG_IMAGES_FOLDER = "bin/Release/pos/";
//the file extention we use
const char *IMAGES_FORMAT = "*.jpg";
//the log file name
const char *LOG_FILE_NAME_POS = "./bin/Release/info.dat";
const char *LOG_FILE_NAME_NEG = "./bin/Release/bg.txt";
//name of the app window
const char *APP_NAME = "Personal Trainer - Image Trainer with OpenCV";
const char *WINDOW_NAME = "Personal Trainer";
const char *CAPTURE_CTA = "CLICK INSIDE TO TRAIN";

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

//include the opencv library
#include "opencv2/opencv.hpp"
//tinydir file utils - https://github.com/cxong/tinydir
#include "tinydir.h"


using namespace std;
using namespace cv;

//the index of the current image
int currentIndex = -1;
int currentLogIndex = 0;
string currentImageFile;
//we'll hold the directory data here
vector<string> posLabels;
vector<string> negLabels;
//a rectangle to masure the mouse movement
Rect mouseBox;
bool drawing_box = false;
bool train_box_on = false;
//the matrix we'll use to display the images
Mat currentImage;
Mat sourceMat;

//=================================
// std string - int conversion methods
template <typename T> string toString(T t){
    ostringstream out;
    out << t;
    return out.str();
}

template <typename T> T fromString(string t){
    T out;
    istringstream in(t);
    in >> out;
    return out;
}
//=================================

//forward decleration
void loadNextPOS();
void loadNEG();

//list all files in a directory using tinydir
//we'll use this to read all the files in the pos images folder
void listDir(const char *destination, vector<string> &labels ) {

    tinydir_dir dir;
    //check if the directory exist or return an error
	if (tinydir_open(&dir, destination) == -1){
        printf("%s", destination);
		perror("Error opening file");
		goto bail;
	}

    //loop through what we have found in the dir
	while (dir.has_next) {
		tinydir_file file;
		if (tinydir_readfile(&dir, &file) == -1){
			perror("Error getting file");
			goto bail;
		}

		if (file.is_dir){
//          ignore sub folders
//			printf("/");
		} else {
//            printf("%s", file.name);
//            push the file name into the labels array
            labels.push_back( file.name );
		}
//		printf("\n");
//        check the next file
		tinydir_next(&dir);
	}

//    we'll triger this in case of errors
    bail:
        tinydir_close(&dir);

}


//load the log file and return the value of the last saved index
//note that we are usign the lines loop to count the index
int getLastSavedLogIndex1() {

    ifstream file;
    string filename = "./bin/Release/pos.txt";
    file.open(filename.c_str());

    int index = 0;
    string line;
    while(getline(file,line)){
        if ( line !=  "" && line != "\n" ) {
            index++;
        }
    }

    return index;
}

int getLastSavedLogIndex2() {

    ifstream file;
    string filename = "./bin/Release/neg.txt";
    file.open(filename.c_str());

    int index = 0;
    string line;
    while(getline(file,line)){
        if ( line !=  "" && line != "\n" ) {
            index++;
        }
    }

    return index;
}

// save the current image in the following format
// dir/filename.ext index x y x+w y+h
// pos/img1.jpg  1  140 100 45 45
void saveCurrentImage() {
//construct a string based on the current file name, index and the mouse box rect
  string out = toString(POS_IMAGES_FOLDER) + currentImageFile +"  " + toString(currentLogIndex + 1 )+ "  " + toString(mouseBox.x) + " " + toString(mouseBox.y) + " " + toString(mouseBox.x+mouseBox.width) + " " + toString(mouseBox.y+mouseBox.height);
  ofstream myfile;
  myfile.open (LOG_FILE_NAME_POS, fstream::in | fstream::out | fstream::app);
  myfile << out + "\n";
  myfile.close();
}

//  save the neg image in the following format
// dir/filename.ext 
//neg/img1.jpg
void saveNegImage() {
//construct a string based on the current file name, index and the mouse box rect
  string out = toString(NEG_IMAGES_FOLDER) + currentImageFile +"  " + toString(currentLogIndex + 1 );
  ofstream myfile;
  myfile.open (LOG_FILE_NAME_NEG, fstream::in | fstream::out | fstream::app);
  myfile << out + "\n";
  myfile.close();
}

//highlight the selected area and display the cta on top
void highlight( Mat img, Rect mouseRect){
    rectangle(img, mouseRect, CV_RGB(255,255,0), 2, CV_AA);
    putText(img, CAPTURE_CTA, Point( mouseRect.x, mouseRect.y - 10 ),
    CV_FONT_HERSHEY_TRIPLEX, 0.35, cvScalar(255,255,255), 0, CV_AA);
}


//handle the mouse events
void onMouse(int event, int x, int y, int, void*) {

    switch( event ){
        //draw the rectangle if the mouse was clicked befor the move
		case CV_EVENT_MOUSEMOVE:
			if( drawing_box ){
				mouseBox.width = x-mouseBox.x;
				mouseBox.height = y-mouseBox.y;
			}
			break;

        //on mouse down:
        //either define a new rectangle
        //or save the current one
		case CV_EVENT_LBUTTONDOWN:
		     if (train_box_on) {
                //check if the click was inside the train box
                if ( x >= mouseBox.x && x <= mouseBox.x + mouseBox.width ) {
                    if ( y >= mouseBox.y && y <= mouseBox.y + mouseBox.height  ) {
//                            snap the pictuer and load the next image
                            saveCurrentImage();
//                            and load the next one
                            loadNextPOS();
                    }
                }
            }
            //if clicked out side the current rectangle,
            //start drawing a new one instead
            drawing_box = true;
			mouseBox = Rect( x, y, 0, 0 );
//			reset the current image to the original
            sourceMat.copyTo(currentImage);
			break;

        //on mouse up
        //finish the rect and draw it on the image
		case CV_EVENT_LBUTTONUP:
			drawing_box = false;
			if( mouseBox.width < 0 ){
				mouseBox.x += mouseBox.width;
				mouseBox.width *= -1;
			}
			if( mouseBox.height < 0 ){
				mouseBox.y += mouseBox.height;
				mouseBox.height *= -1;
			}

//            if we have a valid rect lets draw it on the image
            if (mouseBox.width > 0 && mouseBox.height > 0 ) {
                train_box_on = true;
                highlight( currentImage, mouseBox );
            } else {
                //or set the mouse rect to invald
                mouseBox  = Rect(-1,-1,-1,-1);
                train_box_on = false;
            }
			imshow(WINDOW_NAME, currentImage);
			break;
	}

}

//load an image and set it as the current one
void loadImage(String filename) {
    sourceMat = imread(filename, 1);
    sourceMat.copyTo(currentImage);
    //display the image in our app window
    imshow(WINDOW_NAME, currentImage);
    //and set the mouse callbacks
    setMouseCallback(WINDOW_NAME, onMouse, 0);
}

//load the next item on the list
//or shutdown the app if the training is completed
void loadNextPOS() {

    int len = posLabels.size();
    cout << currentLogIndex << " " << currentIndex << " - " << len << endl;
    if ( currentIndex <= len - 1 ) {
        //up the indexes
        currentLogIndex++;
        currentIndex++;
        cout << "loading image " <<  currentIndex + 1  << " of " << len << endl;
        stringstream  stream(posLabels.at(currentIndex));
        stream >> currentImageFile;
        loadImage(POS_IMAGES_DIR + currentImageFile);
    } else if ( currentIndex > len ) {
        //done with training, lets shutdown.
        cvDestroyWindow( WINDOW_NAME );
        cout << "--> Training completed for pos" << currentIndex + 1 << " of " << len << endl;
    }
}

void loadNEG() {

    int len = negLabels.size();

    while(currentIndex <= len - 1) {
        //up the indexes
        currentIndex++;
        currentLogIndex++;
        stringstream  stream(negLabels.at(currentIndex));
        stream >> currentImageFile;
        saveNegImage();
      cout << "--> Training completed for neg" << currentIndex + 1 << " of " << len << endl;

    } 

        //done with training, lets shutdown.
    //  cout << "--> Training completed for pos" << currentIndex + 1 << " of " << len << endl;

} 


int main(int argc, char *argv[]) {
    //print the app name

    cout <<  APP_NAME << endl;


    //list the files in our pos images dir
    listDir(POS_IMAGES_DIR, posLabels);
    //load the last index saved
    currentLogIndex = getLastSavedLogIndex1();
    currentIndex = currentLogIndex - 1;

    //load the next - first item
    loadNextPOS();

    currentIndex = -1;
    currentLogIndex = 0;

    listDir(NEG_IMAGES_DIR, negLabels);  
    currentLogIndex = getLastSavedLogIndex2();
    currentIndex = currentLogIndex -1;
    loadNEG();



    while (true) {
        int key = waitKey(200);
        if (key == 27) break;
    }
    
    return 0;
}
