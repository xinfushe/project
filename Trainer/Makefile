all:
	g++ Trainer2.cpp `pkg-config --cflags --libs opencv` -o TrainApple
	./opencv_createsamples -info info.dat -vec pos.vec -bg bg.txt -num 10
