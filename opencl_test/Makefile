all: main ocl_hello ocl_vector 

ocl_hello:
	g++ main_hello.cpp `pkg-config --cflags --libs opencv` -lcl -o ocl_hello -std=c++11

ocl_vector:
	g++ main_vector.cpp `pkg-config --cflags --libs opencv` -lcl -o ocl_vector -std=c++11

main: main.cpp ocl.cpp
	g++ main.cpp  -std=c++11 -g3 -Wall -o main

clean:
	rm main ocl_hello ocl_vector