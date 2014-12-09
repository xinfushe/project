
link_directories(${OpenCV_LIB_DIR})

include_directories(main
    ../libopentld/imacq
	../libopentld/mftracker
	../libopentld/tld
	../libopentld/tld/detector
    ../3rdparty/cvblobs
    ${OpenCV_INCLUDE_DIRS})

if(NOT USE_SYSTEM_LIBS)
    include_directories(../3rdparty/libconfig)
endif(NOT USE_SYSTEM_LIBS)


#-------------------------------------------------------------------------------
# main
add_library(main
    main/Config.cpp
    main/Gui.cpp
    main/Main.cpp
    main/Settings.cpp
	main/Trajectory.cpp
    main/Config.h
    main/Gui.h
    main/Main.h
	main/Settings.h
	main/Trajectory.h
	#main/ocl.cpp
	#main/ocl.h
	)

target_link_libraries(main libopentld cvblobs config++ ${OpenCV_LIBS})

#-------------------------------------------------------------------------------
# opentld
add_executable(opentld_ocl
    OpenTLD.cpp)

target_link_libraries(opentld_ocl main libopentld cvblobs config++ ${OpenCV_LIBS} /usr/local/lib/beignet/libcl.so)

install(TARGETS opentld_ocl DESTINATION bin)








#-------------------------------------------------------------------------------
# qopentld
if(BUILD_QOPENTLD)
	#
	#find_package(PkgConfig REQUIRED)
	#pkg_check_modules(PC_OPENCV opencv REQUIRED)
	#if(OPENCV_FOUND)
	#endif(OPENCV_FOUND)
	
	#pkg_config(gtk+-2.0 includedir libdir linkflags cflags)
	#include_directories(OPENCV_INCLUDE_DIRS) #-I
	#LINK_DIRECTORIES(${libdir}) #-L 
	#TARGET_LINK_LIBRARIES(helloworld ${linkflags}) #-l 
	#ADD_DEFINITIONS(${cflags}) #-D
	#<XPREFIX>_FOUND          ... set to 1 if module(s) exist
	#<XPREFIX>_LIBRARIES      ... only the libraries (w/o the '-l')
	#<XPREFIX>_LIBRARY_DIRS   ... the paths of the libraries (w/o the '-L')
	#<XPREFIX>_LDFLAGS        ... all required linker flags
	#<XPREFIX>_LDFLAGS_OTHER  ... all other linker flags
	#<XPREFIX>_INCLUDE_DIRS   ... the '-I' preprocessor flags (w/o the '-I')
	#<XPREFIX>_CFLAGS         ... all required cflags
	#<XPREFIX>_CFLAGS_OTHER   ... the other compiler flags
	#
    include_directories(qopentld
    	#OPENCV_INCLUDE_DIRS
        ${CMAKE_CURRENT_BINARY_DIR})
    #link_directories(OPENCV_LIBRARY_DIRS)

    include(${QT_USE_FILE})
    add_definitions(${QT_DEFINITIONS} )

    set(QOPENTLD_SOURCES QOpenTLD.cpp qopentld/ConfigDialog.cpp)
    set(QOPENTLD_HEADERS qopentld/ConfigDialog.h)
    set(QOPENTLD_FORMS qopentld/ConfigDialog.ui)
    set(QOPENTLD_RCS ../../res/rcs.qrc)

    QT4_ADD_RESOURCES(QOPENTLD_RCS_CPP ${QOPENTLD_RCS})
    QT4_WRAP_CPP(QOPENTLD_HEADERS_MOC ${QOPENTLD_HEADERS})
    QT4_WRAP_UI(QOPENTLD_FORMS_HEADERS ${QOPENTLD_FORMS})
    
    set(QOPENTLD_SOURCES ${QOPENTLD_SOURCES} ${QOPENTLD_RCS_CPP} ${QOPENTLD_HEADERS_MOC} ${QOPENTLD_FORMS_HEADERS})

    add_executable(qopentld_ocl ${QOPENTLD_SOURCES})
    target_link_libraries(qopentld_ocl main libopentld cvblobs config++ ${OpenCV_LIBS} ${QT_LIBRARIES} /usr/local/lib/beignet/libcl.so)
    
    install(TARGETS qopentld_ocl DESTINATION bin)
endif(BUILD_QOPENTLD)

