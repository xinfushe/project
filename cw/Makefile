
CC = g++
CFLAGS	= -O2 -Wall -std=c99 `pkg-config --cflags libdrm` -I/root/media_related/v4l2_related/headers/linux-3.10.19 -g -O0
LDFLAGS	=
LIBS	= -ldrm -lOpenCL -lX11 -lva -lva-x11 `pkg-config --libs opencv`

%.o : %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

all: vol

vol: vol.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	-rm -f *.o
	-rm -f vol
	-rm -f *.clb
