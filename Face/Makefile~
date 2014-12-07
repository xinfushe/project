all:
	g++ facedetect.cpp `pkg-config --cflags --libs opencv` -lcl  -o face -O3 -std=c++11

opencv:
	g++ facedetect_opencv.cpp `pkg-config --cflags --libs opencv` -lcl  -o face_opencv -O3 -std=c++11

phase1:
	g++ ocl_facedetection.cpp `pkg-config --cflags --libs opencv` -lcl  -o face_phase1 -O3

face_other:
	g++ facedetect_other.cpp `pkg-config --cflags --libs opencv` -lcl  -o face_other -O3

cpu:
	g++ facedetect_cpu.cpp `pkg-config --cflags --libs opencv` -lcl  -o face_cpu -O3

cpu_m:
	g++ facedetect_merge_box.cpp `pkg-config --cflags --libs opencv` -lcl  -o face_cpu_m -O3

clean:
	rm face
