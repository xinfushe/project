all: clean ocl_hello ocl_vector

ocl_hello:
	g++ main_hello.cpp `pkg-config --cflags --libs opencv` -lcl -o ocl_hello -std=c++11

ocl_vector:
	g++ main_vector.cpp `pkg-config --cflags --libs opencv` -lcl -o ocl_vector -std=c++11

clean:
	rm ocl_hello ocl_vector
