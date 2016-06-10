if (DEFINED ENV{VX_OPENCV_INCLUDE_PATH})
	set( OPENCV_INCLUDE_PATH $ENV{VX_OPENCV_INCLUDE_PATH} )
	message( "Use OpenCV headers from: " ${OPENCV_INCLUDE_PATH} )
else(DEFINED ENV{VX_OPENCV_INCLUDE_PATH})
	set( OPENCV_INCLUDE_PATH )
	message( "Use OpenCV headers from system default location, in order to set specific path: please set 'VX_OPENCV_INCLUDE_PATH' env. variable and reconfigure")
endif (DEFINED ENV{VX_OPENCV_INCLUDE_PATH})

if (DEFINED ENV{VX_OPENCV_LIB_PATH})
	set( OPENCV_LIB_PATH $ENV{VX_OPENCV_LIB_PATH} )
	message( "Use OpenCV headers from: " ${OPENCV_LIB_PATH} )
else(DEFINED ENV{VX_OPENCV_LIB_PATH})
	set( OPENCV_LIB_PATH )
	message( "Use OpenCV headers from system default location, in order to set specific path: please set 'VX_OPENCV_LIB_PATH' env. variable and reconfigure")
endif (DEFINED ENV{VX_OPENCV_LIB_PATH})

include_directories( BEFORE
                     ${CMAKE_CURRENT_SOURCE_DIR} 
					 ${CMAKE_SOURCE_DIR}/include
					 ${OPENCV_INCLUDE_PATH})
					 
# set target name
set( TARGET_NAME_PROFILE_AND_VERIFY vx_profile_and_verify )
set( TARGET_NAME_MYAPP_LIB my_app_lib )

FIND_SOURCES(vx_profile_and_verify.cpp)

# add library ${TARGET_NAME_MYAPP_LIB}
add_library (${TARGET_NAME_MYAPP_LIB} STATIC ${SOURCE_FILES})

if (WIN32)
	set(OPENCV_LIBS opencv_calib3d310d opencv_core310d opencv_features2d310d opencv_flann310d opencv_highgui310d opencv_imgcodecs310d opencv_imgproc310d opencv_ml310d opencv_objdetect310d opencv_photo310d opencv_shape310d opencv_stitching310d 	opencv_superres310d opencv_ts310d opencv_video310d opencv_videoio310d opencv_videostab310d)
	prepend(ABS_OPENCV_LIBS_PRE ${OPENCV_LIB_PATH} ${OPENCV_LIBS})
	postpend(LINK_OPENCV_LIBS ".lib" ${ABS_OPENCV_LIBS_PRE})
elseif (ANDROID)
	set(OPENCV_LIBS_THIRD libIlmImf.a liblibjasper.a liblibjpeg.a liblibpng.a liblibtiff.a liblibwebp.a libtbb.a)
	prepend(THIRD_LIBS "${OPENCV_LIB_PATH}/3rdparty/libs/${ANDROID_ABI}/" ${OPENCV_LIBS_THIRD})
	
	set(OPENCV_LIBS libopencv_calib3d.a libopencv_core.a libopencv_features2d.a libopencv_flann.a libopencv_highgui.a libopencv_imgcodecs.a libopencv_imgproc.a libopencv_java3.so libopencv_ml.a libopencv_objdetect.a libopencv_photo.a libopencv_shape.a libopencv_stitching.a libopencv_superres.a libopencv_video.a libopencv_videoio.a libopencv_videostab.a)
	prepend(LINK_OPENCV_LIBS "${OPENCV_LIB_PATH}/libs/${ANDROID_ABI}/" ${OPENCV_LIBS})
	
	list(APPEND LINK_OPENCV_LIBS ${THIRD_LIBS} z dl m log)
else ()
	set(OPENCV_LIBS cv2.so libopencv_calib3d.so libopencv_contrib.so libopencv_core.so libopencv_features2d.so libopencv_flann.so libopencv_gpu.so libopencv_highgui.so libopencv_imgproc.so libopencv_legacy.so libopencv_ml.so libopencv_nonfree.so libopencv_objdetect.so libopencv_ocl.so libopencv_photo.so libopencv_stitching.so libopencv_superres.so libopencv_video.so libopencv_videostab.so)
	prepend(LINK_OPENCV_LIBS "${OPENCV_LIB_PATH}/" ${OPENCV_LIBS})
endif (WIN32)
target_link_libraries( ${TARGET_NAME_MYAPP_LIB} openvx ${LINK_OPENCV_LIBS})

# add executable ${TARGET_NAME_PROFILE_AND_VERIFY}
add_executable (${TARGET_NAME_PROFILE_AND_VERIFY} vx_profile_and_verify.cpp ${HEADER_FILES})
target_link_libraries (${TARGET_NAME_PROFILE_AND_VERIFY} ${TARGET_NAME_MYAPP_LIB})

install ( TARGETS ${TARGET_NAME_PROFILE_AND_VERIFY} 
          RUNTIME DESTINATION bin
          ARCHIVE DESTINATION bin
          LIBRARY DESTINATION bin )

set_target_properties( ${TARGET_NAME_MYAPP_LIB} PROPERTIES FOLDER ${MYAPPS_FOLDER} )
set_target_properties( ${TARGET_NAME_PROFILE_AND_VERIFY} PROPERTIES FOLDER ${MYAPPS_FOLDER} )

add_subdirectory( vx_scheduling )