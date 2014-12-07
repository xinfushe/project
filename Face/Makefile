all:
	g++ facedetect.cpp `pkg-config --cflags --libs opencv` -lcl  -o face -O3 -std=c++11

clean:
	rm face
