#
# Copyright (c) 2016 National Tsing Hua University. 
# author Tzu-Hsiang Lin <thlin@pllab.cs.nthu.edu.tw>
#

if ( ANDROID AND EXPERIMENTAL_USE_FASTCV )

	# set target name
	set( TARGET_NAME openvx-fastcv-lib )

	include_directories( BEFORE
						 ${CMAKE_CURRENT_SOURCE_DIR}
						 ${CMAKE_SOURCE_DIR}/include
						 ${CMAKE_SOURCE_DIR}/debug
						 ${CMAKE_CURRENT_SOURCE_DIR}/inc)

	FIND_SOURCES()

	# add a target named ${TARGET_NAME}
	add_library (${TARGET_NAME} ${SOURCE_FILES})
	
	if (BUILD_X64)
		set(FASTCV_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib64/libfastcv.a )
	else (BUILD_X64)
		set(FASTCV_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib32/libfastcv.a )
	endif (BUILD_X64)

	target_link_libraries( ${TARGET_NAME} openvx ${FASTCV_LIB_PATH})

	install ( TARGETS ${TARGET_NAME} 
			  RUNTIME DESTINATION bin
			  ARCHIVE DESTINATION bin
			  LIBRARY DESTINATION bin )

	set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${KERNELS_FOLDER} )

endif ( ANDROID AND EXPERIMENTAL_USE_FASTCV )