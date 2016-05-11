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

set(OPENCV_LIBS opencv_calib3d310d opencv_core310d opencv_features2d310d opencv_flann310d opencv_highgui310d opencv_imgcodecs310d opencv_imgproc310d opencv_ml310d opencv_objdetect310d opencv_photo310d opencv_shape310d opencv_stitching310d 	opencv_superres310d opencv_ts310d opencv_video310d opencv_videoio310d opencv_videostab310d)

include_directories( BEFORE
                     ${CMAKE_CURRENT_SOURCE_DIR} 
					 ${CMAKE_SOURCE_DIR}/include
					 ${OPENCV_INCLUDE_PATH})
					 
# set target name
set( TARGET_NAME_PROFILE_AND_VERIFY vx_profile_and_verify )

FIND_SOURCES()

# add a target named ${TARGET_NAME}
add_executable (${TARGET_NAME_PROFILE_AND_VERIFY} ${SOURCE_FILES} ${HEADER_FILES})

prepend(ABS_OPENCV_LIBS_PRE ${OPENCV_LIB_PATH} ${OPENCV_LIBS})
if (WIN32)
   postpend(ABS_OPENCV_LIBS ".lib" ${ABS_OPENCV_LIBS_PRE})
endif (WIN32)
target_link_libraries( ${TARGET_NAME_PROFILE_AND_VERIFY} openvx ${ABS_OPENCV_LIBS})

install ( TARGETS ${TARGET_NAME_PROFILE_AND_VERIFY} 
          RUNTIME DESTINATION bin
          ARCHIVE DESTINATION bin
          LIBRARY DESTINATION bin )
		  
set_target_properties( ${TARGET_NAME_PROFILE_AND_VERIFY} PROPERTIES FOLDER ${MYAPPS_FOLDER} )
