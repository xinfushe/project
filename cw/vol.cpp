#include <getopt.h>             /* getopt_long() */
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <linux/videodev2.h>
//#include <libv4l2.h>
#include <math.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <libdrm/i915_drm.h>

#include <inttypes.h>
#include <ctype.h>
#include <CL/cl.h>

#include <va/va.h>
#include <va/va_dec_jpeg.h>
#include <va/va_x11.h>
#include <va/va_drmcommon.h>
#include "tinyjpeg-internal.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>
/*#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>*/
#include <opencv2/ocl/ocl.hpp>

using namespace cv;
using namespace std;
//cv::ocl::oclMat om(480, 640, CV_8UC3); 
//cv::ocl::oclMat om; 
Mat m;

#define BUFFER_QUANT_DEFAULT 3
#define VIDEO_NODE_DEFAULT "/dev/video0"
#define WIDTH_DEFAULT 640
#define HEIGHT_DEFAULT 480

#define PERFORMANCE_TUNING 1
#define VOL_DEBUG 0
#define SHOW_BY_LIBVA 1
#define SHOW_BY_CV 0

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
#define ARRAY_ELEMS(a) (sizeof(a) / sizeof((a)[0]))

#define CHECK_CLSTATUS(status,func)                                  \
    if (status != CL_SUCCESS) {                                   \
        printf("status = %d, %s:%s (%d) failed,exit\n", status, __func__, func, __LINE__); \
        exit(1);                                                            \
    }

#define CHECK_VASTATUS(va_status,func)                                  \
    if (va_status != VA_STATUS_SUCCESS) {                                   \
        fprintf(stderr,"%s:%s (%d) failed,exit\n", __func__, func, __LINE__); \
        exit(1);                                                            \
    }


static Display *x11_display;
static Window   x11_window;
VAConfigID config_id;
VADisplay va_dpy;

int dev_fd;
uint64_t image_size;
cl_int status;  
cl_platform_id *platforms = NULL;
cl_device_id *devices = NULL;
cl_context context = NULL;
cl_command_queue cmdQueue;
cl_program program;
cl_mem *input_buffer = NULL;
typedef cl_int (OCLGETMEMOBJECTFD)(cl_context, cl_mem, int *);
OCLGETMEMOBJECTFD *oclGetMemObjectFd = NULL;
typedef cl_mem (OCLCREATEBUFFERFROMLIBVAINTEL)(cl_context, unsigned int, cl_int *);
OCLCREATEBUFFERFROMLIBVAINTEL *oclCreateBufferFromLibvaIntel = NULL;

int frame_count = 0;
struct global_args{
	char *dev_name;
	int width, height;
	unsigned int spec_res : 1;
	unsigned int fmt;
	int buffer_quant;
	unsigned int do_list: 1;
} ga;
int *import_buf_fds;

static const char short_options[] = "d:r:b:lh";

static const struct option
long_options[] = {
        { "device", required_argument, NULL, 'd' },
        { "help",   no_argument,       NULL, 'h' },
        { "resolution", required_argument,       NULL, 'r' },
        { "buffer_quant",  required_argument, NULL, 'b' },
        { "list",  no_argument, NULL, 'l' },
        { 0, 0, 0, 0 }
};

ocl::OclCascadeClassifier cascade;
string cascadeName = "/root/media_related/opencv/data/haarcascades/haarcascade_frontalface_alt.xml";
double scale = 2.0;
const static Scalar colors[] =  { CV_RGB(0,0,255),
                                  CV_RGB(0,128,255),
                                  CV_RGB(0,255,255),
                                  CV_RGB(0,255,0),
                                  CV_RGB(255,128,0),
                                  CV_RGB(255,255,0),
                                  CV_RGB(255,0,0),
                                  CV_RGB(255,0,255)
                                } ;

static void usage(FILE *fp, int argc, char **argv)
{
        fprintf(fp,
                 "Usage: %s [options]\n\n"
                 "Options:\n"
                 "-d | --device=<dev>  Specify video node like /dev/video*\n"
                 "-h | --help          Print this message\n"
                 "-r | --resolution=<width,height>    Set input resolution\n"
                 "-f | --format=<fmt>  Set input pixel format\n"
                 "-c | --buffer_quant=<qnt>  Set quantity of buffers\n"
		 "-l | --list List available pixel format and resolution\n"
                 "",
                 argv[0]);
}

char* readClSource(char* kernelPath) {

	FILE *fp;
	char *source;
	long int size;

	printf("Program file is: %s\n", kernelPath);

	fp = fopen(kernelPath, "rb");
	if(!fp) {
		fprintf(stderr, "Could not open kernel file\n");
		exit(-1);
	}
	status = fseek(fp, 0, SEEK_END);
	if(status != 0) {
		fprintf(stderr, "Error seeking to end of file\n");
		exit(-1);
	}
	size = ftell(fp);
	if(size < 0) {
		fprintf(stderr, "Error getting file position\n");
		exit(-1);
	}

	rewind(fp);

	source = (char *)malloc(size + 1);

	int i;
	for (i = 0; i < size+1; i++) {
		source[i]='\0';
	}

	if(source == NULL) {
		fprintf(stderr, "Error allocating space for the kernel source\n");
		exit(-1);
	}

	fread(source, 1, size, fp);
	source[size] = '\0';

	return source;
}

static void list_resolution(){
	int ret;
	struct v4l2_capability cap;
	struct v4l2_frmsizeenum frm_size;

	dev_fd = open(ga.dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);
	if(dev_fd < 0){
		fprintf(stderr, "Failed to open %s", ga.dev_name);
		perror(" ");
		exit(1);
	}

	memset(&cap, 0, sizeof cap);
	ret = ioctl(dev_fd, VIDIOC_QUERYCAP, &cap);
	if(ret){
		perror("VIDIOC_QUERYCAP");
		exit(1);
	}

	if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)){
		fprintf(stderr, "The device is not video capture device\n");
		exit(1);
	}

	if(!(cap.capabilities & V4L2_CAP_STREAMING)){
		fprintf(stderr, "The device does not support streaming i/o\n");
		exit(1);
	}

        printf("Support resolution under pixel format 'MJPG':\n");
	frm_size.pixel_format = V4L2_PIX_FMT_MJPEG;
	frm_size.index = 0;
	while (ioctl(dev_fd, VIDIOC_ENUM_FRAMESIZES, &frm_size) == 0) {
		if (frm_size.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
			if(!(frm_size.discrete.width % 16) && !(frm_size.discrete.height % 16)){
				printf("%d,%d", frm_size.discrete.width, frm_size.discrete.height);
				printf("\n");
			}
		}
		else if (frm_size.type == V4L2_FRMSIZE_TYPE_STEPWISE) {
			printf("%d,%d - %d,%d with step %d/%d",
				frm_size.stepwise.min_width,
				frm_size.stepwise.min_height,
				frm_size.stepwise.max_width,
				frm_size.stepwise.max_height,
				frm_size.stepwise.step_width,
				frm_size.stepwise.step_height);
			continue;
		}
		frm_size.index++;
	}

	ret = close(dev_fd);
	if(ret){
		fprintf(stderr, "Failed to close %s", ga.dev_name);
		perror(" ");
		exit(1);
	}
}

static void analyse_args(int argc, char *argv[])
{
	ga.dev_name = NULL;
	ga.width = 0;
	ga.height = 0;
	ga.spec_res = 0;
	ga.fmt = V4L2_PIX_FMT_MJPEG;
	ga.buffer_quant = BUFFER_QUANT_DEFAULT;
	ga.do_list = 0;

	int c, ret, idx;
        for (;;) {

                c = getopt_long(argc, argv,
                                short_options, long_options, &idx);

                if (-1 == c)
                        break;

                switch (c) {
                case 0: /* getopt_long() flag */
                        break;

                case 'd':
                        ga.dev_name = optarg;
                        break;

		case '?':
                case 'h':
                        usage(stdout, argc, argv);
			exit(0);

                case 'r':
			ret = sscanf(optarg, "%d,%d", &ga.width, &ga.height);
			ga.spec_res = 1;
			break;

                case 'b':
                        ga.buffer_quant = strtoul(optarg, NULL, 0);
                        break;

                case 'l':
                        ga.do_list = 1;
                        break;

                default:
                        usage(stderr, argc, argv);
			exit(1);
                }
        }
	
	if(!ga.dev_name){
		printf("Haven't specify video node, use default video node: %s\n",
			VIDEO_NODE_DEFAULT);
	}
	if(!ga.dev_name)
		ga.dev_name = VIDEO_NODE_DEFAULT;
	if(ga.do_list){
		list_resolution();
		exit(0);
	}
	if(ga.spec_res){
		if(ga.width <= 0){
			fprintf(stderr, "The frame width must be larger than 0\n");
			exit(1);
		}
		if(ga.height <= 0){
			fprintf(stderr, "The frame height must be larger than 0\n");
			exit(1);
		}
		if(ga.width % 16){
			fprintf(stderr, "The frame width must be a multiple of 16\n");
			exit(1);
		}
		if(ga.height % 16){
			fprintf(stderr, "The frame height must be a multiple of 16\n");
			exit(1);
		}
	}
	else{
		printf("Haven't specify resolution, use default resolution: (width,height) = (%d, %d)\n",
			WIDTH_DEFAULT, HEIGHT_DEFAULT);
		ga.width = WIDTH_DEFAULT;
		ga.height = HEIGHT_DEFAULT;
	}
	return;
}

static void create_dmasharing_buffer(int *buf_fd, struct global_args *g, int index)
{
	if(input_buffer == NULL){
		input_buffer = (cl_mem *)malloc(sizeof(cl_mem)*g->buffer_quant);
	}
	input_buffer[index] = clCreateBuffer(context, CL_MEM_READ_WRITE, image_size, NULL, &status);

	//get the buffer's fd
	status = oclGetMemObjectFd(context, input_buffer[index], buf_fd);
	CHECK_CLSTATUS(status, "clGetMemObjectFdIntel");
}

static void init_cv(){
#if SHOW_BY_CV
	namedWindow("edges",1);
        m.create(ga.height, ga.width, CV_8UC3);
#endif 

    if( !cascade.load( cascadeName ) )
    {
        cout << "ERROR: Could not load classifier cascade: " << cascadeName << endl;
	exit(1);
        //return EXIT_FAILURE;
    }
}

static void init_cl(){
    
	cl_uint numPlatforms = 0;
	cl_uint numDevices = 1;

	status = clGetPlatformIDs(0, NULL, &numPlatforms);
	CHECK_CLSTATUS(status,"clGetPlatformIDs");

	platforms = (cl_platform_id*) malloc(numPlatforms * sizeof(cl_platform_id));
 
	status = clGetPlatformIDs(numPlatforms, platforms, NULL);
	CHECK_CLSTATUS(status,"clGetPlatformIDs");

	status = clGetDeviceIDs(
			platforms[0], 
			CL_DEVICE_TYPE_ALL, 
			0, 
			NULL, 
			&numDevices);
	CHECK_CLSTATUS(status,"clGetDeviceIDs");

	devices = (cl_device_id*) malloc(numDevices * sizeof(cl_device_id));

	status = clGetDeviceIDs(
			platforms[0], 
			CL_DEVICE_TYPE_ALL,        
			numDevices, 
			devices, 
			NULL);
	CHECK_CLSTATUS(status,"clGetDeviceIDs");


	context = clCreateContext(
			NULL, 
			numDevices, 
			devices, 
			NULL, 
			NULL, 
			&status);
	CHECK_CLSTATUS(status,"clCreateContext");

	cv::ocl::initializeContext(platforms, &context, devices);

	cmdQueue = clCreateCommandQueue(
			context, 
			devices[0], 
			0, 
			&status);
	CHECK_CLSTATUS(status,"clCreateCommandQueue");
	char* programSource;

/*
	devices = (cl_device_id*)(om.clCxt->getOpenCLDeviceIDPtr());
	cl_uint numDevices = 1;
	context = *(cl_context*)(om.clCxt->getOpenCLContextPtr());
	cmdQueue = *(cl_command_queue*)(om.clCxt->getOpenCLCommandQueuePtr());
*/

	programSource = readClSource("parse_mjpeg_header.cl");
	program = clCreateProgramWithSource(
			context, 
			1, 
			(const char**)&programSource,                                 
			NULL, 
			&status);
	CHECK_CLSTATUS(status,"clCreateProgramWithSource");

	status = clBuildProgram(
			program, 
			numDevices, 
			devices, 
			NULL, 
			NULL, 
			NULL);
	if(status == CL_BUILD_PROGRAM_FAILURE) {
		size_t log_size;
		clGetProgramBuildInfo(program, devices[0],CL_PROGRAM_BUILD_LOG,0,NULL,&log_size);
		char *log = (char*) malloc(log_size);
		clGetProgramBuildInfo(program,devices[0],CL_PROGRAM_BUILD_LOG,log_size,log,NULL);
		printf("%s\n", log);
		free(log);
	}
	CHECK_CLSTATUS(status,"clBuildProgram");

	free(programSource);

	oclCreateBufferFromLibvaIntel = NULL;
	oclCreateBufferFromLibvaIntel = (OCLCREATEBUFFERFROMLIBVAINTEL *)clGetExtensionFunctionAddress("clCreateBufferFromLibvaIntel");
	if(!oclCreateBufferFromLibvaIntel){
		fprintf(stderr, "Failed to get extension clCreateBufferFromLibvaIntel\n");
		exit(1);
	}
}

void release_cl(){
	clReleaseProgram(program);
	clReleaseCommandQueue(cmdQueue);
	for (int i = 0; i < ga.buffer_quant; ++i) {
		clReleaseMemObject(input_buffer[i]);
	}
	clReleaseContext(context);

	free(platforms);
	free(devices);
}

static void release_va(){
	vaDestroyConfig(va_dpy,config_id);
	vaTerminate(va_dpy);
}

struct jdec_private_to_cl *tinyjpeg_to_cl_init(void)
{
	struct jdec_private_to_cl *priv;

	priv = (struct jdec_private_to_cl *)calloc(1, sizeof(struct jdec_private_to_cl));
	if (priv == NULL)
		return NULL;
	return priv;
}

void tinyjpeg_to_cl_free(struct jdec_private_to_cl *priv)
{
	free(priv);
}

static VADisplay
va_open_display_x11(void)
{
	x11_display = XOpenDisplay(NULL);
	if (!x11_display) {
		fprintf(stderr, "error: can't connect to X server!\n");
		return NULL;
	}
	return vaGetDisplay(x11_display);
}

static int
ensure_window(unsigned int width, unsigned int height)
{
	Window win, rootwin;
	unsigned int black_pixel, white_pixel;
	int screen;

	if (!x11_display)
		return 0;

	if (x11_window) {
		XResizeWindow(x11_display, x11_window, width, height);
		return 1;
	}

	screen      = DefaultScreen(x11_display);
	rootwin     = RootWindow(x11_display, screen);
	black_pixel = BlackPixel(x11_display, screen);
	white_pixel = WhitePixel(x11_display, screen);

	win = XCreateSimpleWindow(x11_display,
				  rootwin,
				  0, 0, width, height,
				  1, black_pixel, white_pixel);
	if (!win)
		return 0;
	x11_window = win;

	XMapWindow(x11_display, x11_window);
	XSync(x11_display, False);
	return 1;
}

static inline int
validate_rect(const VARectangle *rect)
{
    return (rect            &&
            rect->x >= 0    &&
            rect->y >= 0    &&
            rect->width > 0 &&
            rect->height > 0);
}

static VAStatus
va_put_surface_x11(
    VADisplay          va_dpy,
    VASurfaceID        surface,
    const VARectangle *src_rect,
    const VARectangle *dst_rect
)
{
	unsigned int win_width, win_height;

	if (!va_dpy)
		return VA_STATUS_ERROR_INVALID_DISPLAY;
	if (surface == VA_INVALID_SURFACE)
		return VA_STATUS_ERROR_INVALID_SURFACE;
	if (!validate_rect(src_rect) || !validate_rect(dst_rect))
		return VA_STATUS_ERROR_INVALID_PARAMETER;

	win_width  = dst_rect->x + dst_rect->width;
	win_height = dst_rect->y + dst_rect->height;
	if (!ensure_window(win_width, win_height))
		return VA_STATUS_ERROR_ALLOCATION_FAILED;
	return vaPutSurface(va_dpy, surface, x11_window,
			    src_rect->x, src_rect->y,
			    src_rect->width, src_rect->height,
			    dst_rect->x, dst_rect->y,
			    dst_rect->width, dst_rect->height,
			    NULL, 0,
			    VA_FRAME_PICTURE |  VA_FILTER_SCALING_FAST); // Add the flag "VA_FILTER_SCALING_FAST" to 
									 //enable OpenCL VPP integrated in intel-driver
			    //VA_FRAME_PICTURE);
}

VADisplay
va_open_display(void)
{
	VADisplay va_dpy = NULL;

	va_dpy = va_open_display_x11();

	if (!va_dpy)  {
		fprintf(stderr, "error: error to initialize display\n");
		abort();
	}
	return va_dpy;
}

static void init_va(){

	VAEntrypoint entrypoints[5];
	int num_entrypoints,vld_entrypoint;
	VAConfigAttrib attrib;
	int major_ver, minor_ver;
	VAStatus va_status;

	va_dpy = va_open_display();

	va_status = vaInitialize(va_dpy, &major_ver, &minor_ver);
	CHECK_VASTATUS(va_status, "vaInitialize");

	va_status = vaQueryConfigEntrypoints(va_dpy, VAProfileJPEGBaseline, entrypoints, 
			     &num_entrypoints);
	CHECK_VASTATUS(va_status, "vaQueryConfigEntrypoints");

	for(vld_entrypoint = 0; vld_entrypoint < num_entrypoints; vld_entrypoint++) {
		if (entrypoints[vld_entrypoint] == VAEntrypointVLD)
			break;
	}
	if (vld_entrypoint == num_entrypoints) {
		/* not find VLD entry point */
		assert(0);
	}

	/* Assuming finding VLD, find out the format for the render target */
	attrib.type = VAConfigAttribRTFormat;
	vaGetConfigAttributes(va_dpy, VAProfileJPEGBaseline, VAEntrypointVLD,
			  &attrib, 1);
	if ((attrib.value & VA_RT_FORMAT_YUV420) == 0) {
		/* not find desired YUV420 RT format */
		assert(0);
	}

	va_status = vaCreateConfig(va_dpy, VAProfileJPEGBaseline, VAEntrypointVLD,
			      &attrib, 1,&config_id);
	CHECK_VASTATUS(va_status, "vaQueryConfigEntrypoints");

}

void Draw(Mat& img, vector<Rect>& faces)
{
    int i = 0;
    for( vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); r++, i++ )
    {
        Point center;
        Scalar color = colors[i%8];
        int radius;
        center.x = cvRound((r->x + r->width*0.5)*scale);
        center.y = cvRound((r->y + r->height*0.5)*scale);
        radius = cvRound((r->width + r->height)*0.25*scale);
        circle( img, center, radius, color, 3, 8, 0 );
    }
    /*if( abs(scale-1.0)>.001 )
    {
        resize(img, img, Size((int)(img.cols/scale), (int)(img.rows/scale)));
    }*/
}

static int facedetect_one_thread(ocl::oclMat &image, Mat &frameCopy)
{

    vector<Rect> faces;

    ocl::oclMat gray, smallImg( cvRound (ga.height/scale), cvRound(ga.width/scale), CV_8UC1 );
    ocl::cvtColor( image, gray, CV_BGR2GRAY );
    ocl::resize( gray, smallImg, smallImg.size(), 0, 0, INTER_LINEAR );
    ocl::equalizeHist( smallImg, smallImg );

    //cascade.detectMultiScale( smallImg, faces, 1.1,
    cascade.detectMultiScale( smallImg, faces, 1.25,
                              3, 0
                              |CV_HAAR_SCALE_IMAGE
                              , Size(30,30), Size(0, 0) );

    Draw(frameCopy, faces);

    return 0;
}

int tinyjpeg_decode(struct jdec_private_to_cl *priv, unsigned char* pr_stream, int index)
{

	VASurfaceID surface_id;
	VAContextID context_id;
	VABufferID pic_param_buf,iqmatrix_buf,huffmantable_buf,slice_param_buf,slice_data_buf;
	VAStatus va_status;
	int max_h_factor, max_v_factor;
	int putsurface=1;
	unsigned int i, j;
	int surface_type;
	char *type;
	int ChromaTypeIndex;

	VASurfaceAttrib forcc;
	forcc.type =VASurfaceAttribPixelFormat;
	forcc.flags=VA_SURFACE_ATTRIB_SETTABLE;
	forcc.value.type=VAGenericValueTypeInteger;

	VAPictureParameterBufferJPEGBaseline pic_param;
	memset(&pic_param, 0, sizeof(pic_param));
	pic_param.picture_width = priv->width;
	pic_param.picture_height = priv->height;
	pic_param.num_components = priv->nf_components;


	for (i=0; i<pic_param.num_components; i++) { // tinyjpeg support 3 components only, does it match va?
		pic_param.components[i].component_id = priv->component_infos[i].cid;
		pic_param.components[i].h_sampling_factor = priv->component_infos[i].Hfactor;
		pic_param.components[i].v_sampling_factor = priv->component_infos[i].Vfactor;
		pic_param.components[i].quantiser_table_selector = priv->component_infos[i].quant_table_index;
	}
	int h1, h2, h3, v1, v2, v3;
	h1 = pic_param.components[0].h_sampling_factor;
	h2 = pic_param.components[1].h_sampling_factor;
	h3 = pic_param.components[2].h_sampling_factor;
	v1 = pic_param.components[0].v_sampling_factor;
	v2 = pic_param.components[1].v_sampling_factor;
	v3 = pic_param.components[2].v_sampling_factor;

	if (h1 == 2 && h2 == 1 && h3 == 1 &&
	       v1 == 2 && v2 == 1 && v3 == 1) {
		//surface_type = VA_RT_FORMAT_IMC3;
		surface_type = VA_RT_FORMAT_YUV420;
		forcc.value.value.i = VA_FOURCC_IMC3;
		ChromaTypeIndex = 1;
		type = "VA_FOURCC_IMC3";
	}
	else if (h1 == 2 && h2 == 1 && h3 == 1 &&
	       v1 == 1 && v2 == 1 && v3 == 1) {
		//surface_type = VA_RT_FORMAT_YUV422H;
		surface_type = VA_RT_FORMAT_YUV422;
		forcc.value.value.i = VA_FOURCC_422H;
		ChromaTypeIndex = 2;
		type = "VA_FOURCC_422H";
	}
	else if (h1 == 1 && h2 == 1 && h3 == 1 &&
	       v1 == 1 && v2 == 1 && v3 == 1) {
		surface_type = VA_RT_FORMAT_YUV444;
		forcc.value.value.i = VA_FOURCC_444P;
		//forcc.value.value.i = VA_FOURCC_RGBP;
		ChromaTypeIndex = 3;
		type = "VA_FOURCC_444P";
	}
	else if (h1 == 4 && h2 == 1 && h3 == 1 &&
	       v1 == 1 && v2 == 1 && v3 == 1) {
		surface_type = VA_RT_FORMAT_YUV411;
		forcc.value.value.i = VA_FOURCC_411P;
		ChromaTypeIndex = 4;
		type = "VA_FOURCC_411P";
	}
	else if (h1 == 1 && h2 == 1 && h3 == 1 &&
	       v1 == 2 && v2 == 1 && v3 == 1) {
		//surface_type = VA_RT_FORMAT_YUV422V;
		surface_type = VA_RT_FORMAT_YUV422;
		forcc.value.value.i = VA_FOURCC_422V;
		ChromaTypeIndex = 5;
		type = "VA_FOURCC_422V";
	}
	else if (h1 == 2 && h2 == 1 && h3 == 1 &&
	       v1 == 2 && v2 == 2 && v3 == 2) {
		//surface_type = VA_RT_FORMAT_YUV422H;
		surface_type = VA_RT_FORMAT_YUV422;
		forcc.value.value.i = VA_FOURCC_422H;
		ChromaTypeIndex = 6;
		type = "VA_FOURCC_422H";
	}
	else if (h2 == 2 && h2 == 2 && h3 == 2 &&
	       v1 == 2 && v2 == 1 && v3 == 1) {
		//surface_type = VA_RT_FORMAT_YUV422V;
		surface_type = VA_RT_FORMAT_YUV422;
		forcc.value.value.i = VA_FOURCC_422V;
		ChromaTypeIndex = 7;
		type = "VA_FOURCC_422V";
	}
	else
	{
		surface_type = VA_RT_FORMAT_YUV400;
		forcc.value.value.i = VA_FOURCC('Y','8','0','0');
		ChromaTypeIndex = 0;
		type = "Format_400P";
	}

	va_status = vaCreateSurfaces(va_dpy,surface_type,
				     priv->width,priv->height, //alignment?
				     &surface_id, 1, &forcc, 1);
	CHECK_VASTATUS(va_status, "vaCreateSurfaces");
	VASurfaceID nv12_surface_id;
	forcc.value.value.i = VA_FOURCC_NV12;
	printf("&&&&&&&Before create vaCreateSurfaces\n");
	va_status = vaCreateSurfaces(va_dpy,surface_type,
				    priv->width,priv->height, //alignment?
				    &nv12_surface_id, 1, &forcc, 1);
	printf("&&&&&&&After create vaCreateSurfaces\n");
	CHECK_VASTATUS(va_status, "vaCreateSurfaces");

	/* Create a context for this decode pipe */
	va_status = vaCreateContext(va_dpy, config_id,
				  priv->width, priv->height, // alignment?
				  VA_PROGRESSIVE,
				  &surface_id,
				  1,
				  &context_id);
	CHECK_VASTATUS(va_status, "vaCreateContext");

	va_status = vaCreateBuffer(va_dpy, context_id,
				 VAPictureParameterBufferType, // VAPictureParameterBufferJPEGBaseline?
				 sizeof(VAPictureParameterBufferJPEGBaseline),
				 1, &pic_param,
				 &pic_param_buf);
	CHECK_VASTATUS(va_status, "vaCreateBuffer");

	VAIQMatrixBufferJPEGBaseline iq_matrix;
	const unsigned int num_quant_tables =
		MIN(COMPONENTS, ARRAY_ELEMS(iq_matrix.load_quantiser_table));
	// todo, only mask it if non-default quant matrix is used. do we need build default quant matrix?
	memset(&iq_matrix, 0, sizeof(VAIQMatrixBufferJPEGBaseline));
	for (i = 0; i < num_quant_tables; i++) {
		if (!priv->Q_tables_valid[i])
			continue;
		iq_matrix.load_quantiser_table[i] = 1;
		for (j = 0; j < 64; j++)
			iq_matrix.quantiser_table[i][j] = priv->Q_tables[i][j];
	}
	va_status = vaCreateBuffer(va_dpy, context_id,
				 VAIQMatrixBufferType, // VAIQMatrixBufferJPEGBaseline?
				 sizeof(VAIQMatrixBufferJPEGBaseline),
				 1, &iq_matrix,
				 &iqmatrix_buf );
	CHECK_VASTATUS(va_status, "vaCreateBuffer");

	VAHuffmanTableBufferJPEGBaseline huffman_table;
	const unsigned int num_huffman_tables =
	   MIN(COMPONENTS, ARRAY_ELEMS(huffman_table.load_huffman_table));
	memset(&huffman_table, 0, sizeof(VAHuffmanTableBufferJPEGBaseline));
	assert(sizeof(huffman_table.huffman_table[0].num_dc_codes) ==
		sizeof(priv->HTDC[0].bits));
	assert(sizeof(huffman_table.huffman_table[0].dc_values[0]) ==
		sizeof(priv->HTDC[0].values[0]));
	for (i = 0; i < num_huffman_tables; i++) {
		if (!priv->HTDC_valid[i] || !priv->HTAC_valid[i])
			continue;
		huffman_table.load_huffman_table[i] = 1;
		memcpy(huffman_table.huffman_table[i].num_dc_codes, priv->HTDC[i].bits,
			sizeof(huffman_table.huffman_table[i].num_dc_codes));
		memcpy(huffman_table.huffman_table[i].dc_values, priv->HTDC[i].values,
			sizeof(huffman_table.huffman_table[i].dc_values));
		memcpy(huffman_table.huffman_table[i].num_ac_codes, priv->HTAC[i].bits,
			sizeof(huffman_table.huffman_table[i].num_ac_codes));   
		memcpy(huffman_table.huffman_table[i].ac_values, priv->HTAC[i].values,
			sizeof(huffman_table.huffman_table[i].ac_values));
		memset(huffman_table.huffman_table[i].pad, 0,
			sizeof(huffman_table.huffman_table[i].pad));
	}
	va_status = vaCreateBuffer(va_dpy, context_id,
				 VAHuffmanTableBufferType, // VAHuffmanTableBufferJPEGBaseline?
				 sizeof(VAHuffmanTableBufferJPEGBaseline),
				 1, &huffman_table,
				 &huffmantable_buf );
	CHECK_VASTATUS(va_status, "vaCreateBuffer");

	// one slice for whole image?
	max_h_factor = priv->component_infos[0].Hfactor;
	max_v_factor = priv->component_infos[0].Vfactor;
	static VASliceParameterBufferJPEGBaseline slice_param;
	slice_param.slice_data_size = priv->scan_to_stream;
	slice_param.slice_data_offset = 0;
	slice_param.slice_data_flag = VA_SLICE_DATA_FLAG_ALL;
	slice_param.slice_horizontal_position = 0;    
	slice_param.slice_vertical_position = 0;    
	slice_param.num_components = priv->cur_sos.nr_components;
	for (i = 0; i < slice_param.num_components; i++) {
		slice_param.components[i].component_selector = priv->cur_sos.components[i].component_id; /* FIXME: set to values specified in SOS  */
		slice_param.components[i].dc_table_selector = priv->cur_sos.components[i].dc_selector;  /* FIXME: set to values specified in SOS  */
		slice_param.components[i].ac_table_selector = priv->cur_sos.components[i].ac_selector;  /* FIXME: set to values specified in SOS  */
	}
	slice_param.restart_interval = priv->restart_interval;
	slice_param.num_mcus = ((priv->width+max_h_factor*8-1)/(max_h_factor*8))*
			     ((priv->height+max_v_factor*8-1)/(max_v_factor*8)); // ?? 720/16? 

	va_status = vaCreateBuffer(va_dpy, context_id,
				 VASliceParameterBufferType, // VASliceParameterBufferJPEGBaseline?
				 sizeof(VASliceParameterBufferJPEGBaseline),
				 1,
				 &slice_param, &slice_param_buf);
	CHECK_VASTATUS(va_status, "vaCreateBuffer");


	//create va buffer slice_data_buf
	va_status = vaCreateBuffer(va_dpy, context_id,
				 VASliceDataBufferType,
				 priv->scan_to_stream,
				 1,
				 NULL,
				 &slice_data_buf);
	CHECK_VASTATUS(va_status, "vaCreateBuffer");
	//get slice_data_buf's global name and create a related opencl bo
	unsigned int sdb_bo_name;
	va_status = vaGetBufferGlobalName(va_dpy , 
					  slice_data_buf, 
					  &sdb_bo_name);  
	CHECK_VASTATUS(va_status, "vaGetBufferGlobalName");
	cl_mem sdb_buffer = oclCreateBufferFromLibvaIntel(context, 
							 sdb_bo_name, 
							 &status);
	CHECK_CLSTATUS(status, "clCreateBufferFromLibvaIntel");

	//use opencl to copy frame's slice data to slice_data_buf
	status = clEnqueueCopyBuffer(cmdQueue, 
				     input_buffer[index], sdb_buffer, 
				     priv->stream_to_begin, 0, 
				     priv->scan_to_stream, 
				     0, NULL, NULL);
	CHECK_CLSTATUS(status, "clEnqueueCopyBuffer");
	//printf("%d,%d\n",priv->stream_to_begin,priv->scan_to_stream);
	if (status != CL_SUCCESS) {
	  printf("%d,%d\n",priv->stream_to_begin,priv->scan_to_stream);
	  
    }
	clFinish(cmdQueue);

	/*//original way to create slice_data_buf
	va_status = vaCreateBuffer(va_dpy, context_id,
				 VASliceDataBufferType,
				 pr_stream_scan - pr_stream,
				 1,
				 (void*)priv_cpu->stream, // jpeg_clip,
				 &slice_data_buf);
	CHECK_VASTATUS(va_status, "vaCreateBuffer");*/

	va_status = vaBeginPicture(va_dpy, context_id, surface_id);
	CHECK_VASTATUS(va_status, "vaBeginPicture");   

	va_status = vaRenderPicture(va_dpy,context_id, &pic_param_buf, 1);
	CHECK_VASTATUS(va_status, "vaRenderPicture");

	va_status = vaRenderPicture(va_dpy,context_id, &iqmatrix_buf, 1);
	CHECK_VASTATUS(va_status, "vaRenderPicture");

	va_status = vaRenderPicture(va_dpy,context_id, &huffmantable_buf, 1);
	CHECK_VASTATUS(va_status, "vaRenderPicture");
   
	va_status = vaRenderPicture(va_dpy,context_id, &slice_param_buf, 1);
	CHECK_VASTATUS(va_status, "vaRenderPicture");

	va_status = vaRenderPicture(va_dpy,context_id, &slice_data_buf, 1);
	CHECK_VASTATUS(va_status, "vaRenderPicture");

	va_status = vaEndPicture(va_dpy,context_id);
	CHECK_VASTATUS(va_status, "vaEndPicture");

	va_status = vaSyncSurface(va_dpy, surface_id);
	CHECK_VASTATUS(va_status, "vaSyncSurface");

	//convert to nv12 surface, then convert to bgra vaImage
	VAImage nv12_image;
	va_status = vaDeriveImage(va_dpy, nv12_surface_id, &nv12_image);
	CHECK_VASTATUS(va_status, "vaDeriveImage");
	va_status = vaGetImage (va_dpy, surface_id, 0, 0, priv->width, priv->height, nv12_image.image_id);
	CHECK_VASTATUS(va_status, "vaGetImage");

	VAImage bgra_image;
	VAImageFormat image_format;
	image_format.fourcc = VA_FOURCC_BGRA;
        image_format.byte_order = VA_LSB_FIRST;
        image_format.bits_per_pixel = 32;
	va_status = vaCreateImage (va_dpy, &image_format, priv->width, priv->height, &bgra_image);
	va_status = vaGetImage (va_dpy, nv12_surface_id, 0, 0, priv->width, priv->height, bgra_image.image_id);


	unsigned int bi_bo_name;
	va_status = vaGetImageGlobalName(va_dpy , 
					 bgra_image.image_id, 
					 &bi_bo_name);  
	CHECK_VASTATUS(va_status, "vaGetBufferGlobalName");
	cl_mem bi_buffer = oclCreateBufferFromLibvaIntel(context, 
							 bi_bo_name, 
							 &status);
	CHECK_CLSTATUS(status, "clCreateBufferFromLibvaIntel");

#if VOL_DEBUG
	size_t bi_buffer_size;
	size_t pvs = sizeof(size_t);
	status = clGetMemObjectInfo(bi_buffer, CL_MEM_SIZE, pvs, &bi_buffer_size, NULL);
	CHECK_CLSTATUS(status, "clGetMemObjectInfo");
	printf("bi_buffer_size = %d\n", bi_buffer_size);
#endif

	size_t step = bgra_image.pitches[0];
#if VOL_DEBUG
	printf("bgra_image.pitches[0] = %d\n", bgra_image.pitches[0]);
#endif
	cv::ocl::oclMat om(ga.height, ga.width, CV_8UC4, bi_buffer, step, 0); 
	om.download(m);
#if PERFORMANCE_TUNING
	struct timeval tstart,tend;
	long int use_useconds;
#endif
#if PERFORMANCE_TUNING
	gettimeofday(&tstart,NULL);
#endif
	facedetect_one_thread(om, m);
	cv::ocl::oclMat new_om;
        new_om.upload(m);
#if PERFORMANCE_TUNING
	gettimeofday(&tend,NULL);
	use_useconds=1000000*(tend.tv_sec - tstart.tv_sec) + (tend.tv_usec - tstart.tv_usec);
	printf("facedetect_one_thread execution time: %ld ms\n",use_useconds / 1000);
#endif

#if SHOW_BY_CV
#if PERFORMANCE_TUNING
	gettimeofday(&tstart,NULL);
#endif
	imshow("edges", m);
	waitKey(1);
#if PERFORMANCE_TUNING
	gettimeofday(&tend,NULL);
	use_useconds=1000000*(tend.tv_sec - tstart.tv_sec) + (tend.tv_usec - tstart.tv_usec);
	printf("imshow & waitKey execution time: %ld ms\n",use_useconds / 1000);
#endif
#endif

	//ocl::GaussianBlur(gray, gray, Size(7,7), 1.5, 1.5);
	//Canny(gray, gray, 0, 30, 3);

#if SHOW_BY_LIBVA
	VASurfaceID gray_bgra_surface_id;
	int gray_bgra_fd;
	status = oclGetMemObjectFd(context, (cl_mem)new_om.data, &gray_bgra_fd);
	CHECK_CLSTATUS(status, "clGetMemObjectFdIntel");

	VASurfaceAttrib sa[2];
	sa[0].type = VASurfaceAttribMemoryType;
	sa[0].flags = VA_SURFACE_ATTRIB_SETTABLE;
	sa[0].value.type = VAGenericValueTypeInteger;
	sa[0].value.value.i = VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME;
	sa[1].type = VASurfaceAttribExternalBufferDescriptor;
	sa[1].flags = VA_SURFACE_ATTRIB_SETTABLE;
	sa[1].value.type = VAGenericValueTypePointer;
	VASurfaceAttribExternalBuffers sa_eb;
	sa_eb.pixel_format = VA_FOURCC_BGRA;
	sa_eb.width = new_om.cols;
	sa_eb.height = new_om.rows;
	sa_eb.data_size = new_om.step * new_om.rows;
	sa_eb.num_planes = 1;
	sa_eb.pitches[0] = new_om.step;
	sa_eb.offsets[0] = 0; //???
	sa_eb.num_buffers = 1;
	sa_eb.buffers = (unsigned long *)malloc(sizeof(unsigned long)*sa_eb.num_buffers);
	sa_eb.buffers[0] = gray_bgra_fd;
	sa_eb.flags = 0;
	sa[1].value.value.p = &sa_eb;
	va_status = vaCreateSurfaces(va_dpy, surface_type,
				     new_om.cols, new_om.rows, 
				    &gray_bgra_surface_id, 1, sa, 2);
#endif

#if 0
	VAImage gray_bgra_image;
	va_status = vaDeriveImage(va_dpy, gray_bgra_surface_id, &gray_bgra_image);
	CHECK_VASTATUS(va_status, "vaDeriveImage");
	unsigned char* gb_data;
	va_status = vaMapBuffer(va_dpy, gray_bgra_image.buf, (void **)&gb_data);
	Mat new_m(sa_eb.height, sa_eb.width, CV_8UC4, gb_data, gray_bgra.step);
	CHECK_VASTATUS(va_status, "vaMapBuffer");

	VASurfaceID gray_bgra_surface_id_tmp;
	forcc.value.value.i = VA_FOURCC_BGRA;
	va_status = vaCreateSurfaces(va_dpy,surface_type,
				    priv->width,priv->height, //alignment?
				    &gray_bgra_surface_id_tmp, 1, &forcc, 1);
	VAImage gray_bgra_image_tmp;
	va_status = vaDeriveImage(va_dpy, gray_bgra_surface_id_tmp, &gray_bgra_image_tmp);
	CHECK_VASTATUS(va_status, "vaDeriveImage");
	unsigned char* gb_data_tmp;
	va_status = vaMapBuffer(va_dpy, gray_bgra_image_tmp.buf, (void **)&gb_data_tmp);
	CHECK_VASTATUS(va_status, "vaMapBuffer");
	/*for(i = 0; i < gray_bgra_image_tmp.data_size; i++)
		gb_data_tmp[i] = gb_data[i];*/
	va_status = vaUnmapBuffer(va_dpy, gray_bgra_image_tmp.buf);
	CHECK_VASTATUS(va_status, "vaUnmapBuffer");
	va_status = vaUnmapBuffer(va_dpy, gray_bgra_image.buf);
	CHECK_VASTATUS(va_status, "vaUnmapBuffer");

	va_status = vaGetImage (va_dpy, gray_bgra_surface_id_tmp, 0, 0, priv->width, priv->height, nv12_image.image_id);
	CHECK_VASTATUS(va_status, "vaGetImage");
#endif 

#if SHOW_BY_LIBVA
	va_status = vaGetImage (va_dpy, gray_bgra_surface_id, 0, 0, priv->width, priv->height, nv12_image.image_id);
	CHECK_VASTATUS(va_status, "vaGetImage");
	VARectangle src_rect, dst_rect;

	src_rect.x      = 0;
	src_rect.y      = 0;
	src_rect.width  = priv->width;
	src_rect.height = priv->height;
	dst_rect        = src_rect;

	//va_status = va_put_surface_x11(va_dpy, surface_id, &src_rect, &dst_rect);
	va_status = va_put_surface_x11(va_dpy, nv12_surface_id, &src_rect, &dst_rect);
	CHECK_VASTATUS(va_status, "vaPutSurface");
#endif

	va_status =vaDestroyBuffer(va_dpy,pic_param_buf);
	CHECK_VASTATUS(va_status, "vaDestroyBuffer");

	va_status =vaDestroyBuffer(va_dpy,iqmatrix_buf);
	CHECK_VASTATUS(va_status, "vaDestroyBuffer");
	va_status =vaDestroyBuffer(va_dpy,huffmantable_buf);
	CHECK_VASTATUS(va_status, "vaDestroyBuffer");
	va_status =vaDestroyBuffer(va_dpy,slice_param_buf);
	CHECK_VASTATUS(va_status, "vaDestroyBuffer");
	va_status =vaDestroyBuffer(va_dpy,slice_data_buf);
	CHECK_VASTATUS(va_status, "vaDestroyBuffer");

	vaDestroySurfaces(va_dpy,&surface_id,1);
	vaDestroyImage(va_dpy, bgra_image.image_id);
	vaDestroyImage(va_dpy, nv12_image.image_id);
	vaDestroySurfaces(va_dpy,&nv12_surface_id,1);
#if SHOW_BY_LIBVA
	vaDestroySurfaces(va_dpy,&gray_bgra_surface_id,1);
#endif
	vaDestroyContext(va_dpy,context_id);
	clReleaseMemObject(sdb_buffer);
	clReleaseMemObject(bi_buffer);
#if SHOW_BY_LIBVA
	free(sa_eb.buffers);
	close(gray_bgra_fd);
#endif
    
	return 0;
}

void decode_display_buffer(int index){
	cl_kernel kernel = NULL;
	cl_kernel kernelfindeoi = NULL;

	kernel = clCreateKernel(program, "parse_mjpeg_header", &status);
	CHECK_CLSTATUS(status,"clCreateKernel");

	kernelfindeoi = clCreateKernel(program, "findEOI", &status);
	CHECK_CLSTATUS(status,"clCreateKernel");

	struct jdec_private_to_cl *jdec_tc;
	jdec_tc = tinyjpeg_to_cl_init();

	cl_mem jdec_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(struct jdec_private_to_cl), NULL, &status);
	CHECK_CLSTATUS(status,"clCreateBuffer");
	cl_mem kernel_result_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), NULL, &status);
	CHECK_CLSTATUS(status,"clCreateBuffer");
	int kernel_result;

	status = clSetKernelArg(kernel,
				0, 
				sizeof(cl_mem), 
				&input_buffer[index]);
	CHECK_CLSTATUS(status,"clSetKernelArg");
	status = clSetKernelArg(kernel,
				1, 
				sizeof(cl_mem), 
				&jdec_buffer);
	CHECK_CLSTATUS(status,"clSetKernelArg");
	status = clSetKernelArg(kernel,
				2, 
				sizeof(int), 
				&image_size);
	CHECK_CLSTATUS(status,"clSetKernelArg");
	status = clSetKernelArg(kernel,
				3, 
				sizeof(cl_mem), 
				&kernel_result_buffer);
	CHECK_CLSTATUS(status,"clSetKernelArg");

	size_t globalWorkSize[1];
	size_t localSize[1];
	globalWorkSize[0] = 1;
	globalWorkSize[0] = 1;
	localSize[0] = 1;
	//parse mjpeg header
    status = clEnqueueNDRangeKernel(cmdQueue, 
					kernel, 
					1, 
					NULL, 
					globalWorkSize, 
					localSize, 
					0, 
					NULL, 
					NULL);
	CHECK_CLSTATUS(status,"clEnqueueNDRangeKernel");

	clFinish(cmdQueue);
	CHECK_CLSTATUS(status,"clFinish");
    //read the parsing result to cpu
	clEnqueueReadBuffer(cmdQueue,
			    kernel_result_buffer, 
		 	    CL_TRUE, 
			    0, 
			    sizeof(int), 
			    &kernel_result, 
			    0, 
			    NULL, 
			    NULL);
	CHECK_CLSTATUS(status,"clEnqueueReadBuffer");
	//printf("RESULT is : %d ms\n",kernel_result); 
	if(kernel_result == -1){
		fprintf(stderr, "Error cl kernel execution result\n");
		exit(1);
	}
	
	status = clSetKernelArg(kernelfindeoi,
				0,
				sizeof(cl_mem),
				&jdec_buffer);
	CHECK_CLSTATUS(status,"clSetKernelArg");
	globalWorkSize[0] = 128;
	localSize[0] = 128;

    status = clEnqueueNDRangeKernel(cmdQueue,
					kernelfindeoi,
					1,
					NULL,
					globalWorkSize,
					localSize,
					0,
					NULL,
					NULL);
	CHECK_CLSTATUS(status,"clEnqueueNDRangeKernel");


	clEnqueueReadBuffer(cmdQueue,
			    jdec_buffer,
			    CL_TRUE,
			    0,
			    sizeof(struct jdec_private_to_cl),
			    jdec_tc,
			    0,
			    NULL,
			    NULL);
	CHECK_CLSTATUS(status,"clEnqueueReadBuffer");


	//printf("Decoding JPEG image %dx%d...\n", jdec_tc->width, jdec_tc->height);
	unsigned char* pr_stream = NULL;
	if(jdec_tc->scan_to_stream<jdec_tc->stream_length){
	  if (tinyjpeg_decode(jdec_tc, pr_stream, index) < 0){
		  fprintf(stderr, "tinyjpeg_decode error!\n");
	  }
	}
	
	tinyjpeg_to_cl_free(jdec_tc);

	clReleaseKernel(kernel);
	clReleaseKernel(kernelfindeoi);
	clReleaseMemObject(jdec_buffer);
	clReleaseMemObject(kernel_result_buffer);
}

static void init_dmabuf(void){

	int ret;
	struct v4l2_requestbuffers rqbufs;
	memset(&rqbufs, 0, sizeof(rqbufs));
	rqbufs.count = ga.buffer_quant;
	rqbufs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	rqbufs.memory = V4L2_MEMORY_DMABUF;

	ret = ioctl(dev_fd, VIDIOC_REQBUFS, &rqbufs);
	if(ret){
		perror("VIDIOC_REQBUFS");
		exit(1);
	}
	if(rqbufs.count < ga.buffer_quant){
		fprintf(stderr, "The device allocated only %u buffers (expected %d buffers)\n",
			rqbufs.count, ga.buffer_quant);
		exit(1);
	}

	import_buf_fds = (int *)malloc(sizeof(int)*ga.buffer_quant);
	
	oclGetMemObjectFd = (OCLGETMEMOBJECTFD *)clGetExtensionFunctionAddress("clGetMemObjectFdIntel");
	if(!oclGetMemObjectFd){
		fprintf(stderr, "Failed to get extension clGetMemObjectFdIntel\n");
		exit(1);
	}
	for (int i = 0; i < ga.buffer_quant; ++i)
		create_dmasharing_buffer(&import_buf_fds[i], &ga, i);
	printf("Succeed to create %d dma buffers \n", ga.buffer_quant);

}

static void init_device(void){

	int ret;
	struct v4l2_capability cap;
	struct v4l2_format fmt;

	dev_fd = open(ga.dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);
	if(dev_fd < 0){
		fprintf(stderr, "Failed to open %s", ga.dev_name);
		perror(" ");
		exit(1);
	}

	memset(&cap, 0, sizeof cap);
	ret = ioctl(dev_fd, VIDIOC_QUERYCAP, &cap);
	if(ret){
		perror("VIDIOC_QUERYCAP");
		exit(1);
	}
	if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)){
		fprintf(stderr, "The device is not video capture device\n");
		exit(1);
	}
	if(!(cap.capabilities & V4L2_CAP_STREAMING)){
		fprintf(stderr, "The device does not support streaming i/o\n");
		exit(1);
	}

	memset(&fmt, 0, sizeof fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	fmt.fmt.pix.width = ga.width;
	fmt.fmt.pix.height = ga.height;
	fmt.fmt.pix.pixelformat = ga.fmt;
        fmt.fmt.pix.field = V4L2_FIELD_ANY;

	ret = ioctl(dev_fd, VIDIOC_S_FMT, &fmt);
	if(ret){
		perror("VIDIOC_S_FMT");
		exit(1);
	}

	ret = ioctl(dev_fd, VIDIOC_G_FMT, &fmt);
	if(ret){
		perror("VIDIOC_G_FMT");
		exit(1);
	}
	if(fmt.fmt.pix.width != ga.width  || fmt.fmt.pix.height != ga.height){
		fprintf(stderr, "This resolution is not supported, please go through supported format by command './main -l'\n");
		exit(1);
	}
	if(fmt.fmt.pix.pixelformat != ga.fmt){
		fprintf(stderr, "Set %.4s format failed\n", (char*)&ga.fmt);
		exit(1);
	}
	printf("Input data format: width = %u, height = %u, pixel format = %.4s\n",
		fmt.fmt.pix.width, fmt.fmt.pix.height,
		(char*)&fmt.fmt.pix.pixelformat);
	image_size = fmt.fmt.pix.sizeimage;
}

static void start_capturing(void){
	int ret;
	for (int i = 0; i < ga.buffer_quant; ++i) {
		struct v4l2_buffer buf;
		memset(&buf, 0, sizeof buf);

		buf.index = i;
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_DMABUF;
		buf.m.fd = import_buf_fds[i];
		ret = ioctl(dev_fd, VIDIOC_QBUF, &buf);
		if(ret){
			perror("VIDIOC_QBUF");
			exit(1);
		}
	}

	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(dev_fd, VIDIOC_STREAMON, &type);
	if(ret){
		perror("VIDIOC_STREAMON");
		exit(1);
	}
}

static void mainloop(void){
	int ret;
	struct v4l2_buffer buf;
	int index;

	while (1) {
		frame_count++;
		printf("******************Frame %d\n", frame_count);
		//if(frame_count == 100)
		//	exit(0);
		fd_set fds;
		struct timeval tv;
		int r;

		FD_ZERO(&fds);
		FD_SET(dev_fd, &fds);

		/* Timeout. */
		tv.tv_sec = 2;
		tv.tv_usec = 0;


		r = select(dev_fd + 1, &fds, NULL, NULL, &tv);

		if (-1 == r) {
			if (EINTR == errno)
				continue;
			perror("select");
		}

		if (0 == r) {
			fprintf(stderr, "select timeout\n");
			exit(1);
		}


		memset(&buf, 0, sizeof buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_DMABUF;
		ret = ioctl(dev_fd, VIDIOC_DQBUF, &buf);
		if(ret){
			perror("VIDIOC_DQBUF");
			exit(1);
		}
		index = buf.index;

		//decode and show on screen by libva
#if PERFORMANCE_TUNING
		/*clock_t start_time, finish_time;
		double duration;
		start_time = clock();*/
		struct timeval tstart,tend;
		gettimeofday(&tstart,NULL);
#endif
		decode_display_buffer(index);
#if PERFORMANCE_TUNING
		gettimeofday(&tend,NULL);
		long int use_useconds=1000000*(tend.tv_sec - tstart.tv_sec) + (tend.tv_usec - tstart.tv_usec);
		printf("Execution time: %ld ms\n",use_useconds / 1000);
		/*finish_time = clock();
		duration = finish_time - start_time;
		printf("Decoding finished in %lf ms\n", duration / CLOCKS_PER_SEC * 1000);*/
#endif

		//then queue this buffer(buf.index) by QBUF
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_DMABUF;
		buf.index = index;
		buf.m.fd = import_buf_fds[index];

		ret = ioctl(dev_fd, VIDIOC_QBUF, &buf);
		if(ret){
			perror("VIDIOC_QBUF");
			exit(1);
		}
	}
}

static void stop_capturing(void)
{
	int ret;
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        ret = ioctl(dev_fd, VIDIOC_STREAMOFF, &type);
	if(ret){
		perror("VIDIOC_STREAMOFF");
		exit(1);
	}
}

static void uninit_device(void){
	int ret = close(dev_fd);
	if(ret){
		fprintf(stderr, "Failed to close %s", ga.dev_name);
		perror(" ");
		exit(1);
	}
}

int main(int argc, char *argv[])
{
	analyse_args(argc, argv);

	init_cv();
	init_device();
	init_va();
	init_cl();
	init_dmabuf();

	start_capturing();
        mainloop();

        stop_capturing();
	release_cl();
	release_va();
        uninit_device();

	return 0;
}

