#
# Copyright (c) 2016 National Tsing Hua University. 
# author Tzu-Hsiang Lin <thlin@pllab.cs.nthu.edu.tw>
#

if ( ANDROID AND EXPERIMENTAL_USE_FASTCV )
    # set target name
    set( TARGET_NAME openvx-fastcv )

    include_directories( BEFORE
                     ${CMAKE_CURRENT_SOURCE_DIR}
					 ${CMAKE_CURRENT_SOURCE_DIR}/../../include
					 ${CMAKE_SOURCE_DIR}/include
					 ${CMAKE_SOURCE_DIR}/kernels/fastcv
					 ${CMAKE_SOURCE_DIR}/debug
                     ${OPENCL_INCLUDE_PATH})

    FIND_SOURCES()

    # add a target named ${TARGET_NAME}
    add_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${DEF_FILE})
	
    if (CYGWIN)
        set_target_properties( ${TARGET_NAME} PROPERTIES LINK_FLAGS ${CMAKE_CURRENT_SOURCE_DIR}/${DEF_FILE} )
    endif (CYGWIN)

    target_link_libraries( ${TARGET_NAME} openvx openvx-fastcv-lib)

    install ( TARGETS ${TARGET_NAME} 
              RUNTIME DESTINATION bin
              ARCHIVE DESTINATION lib
              LIBRARY DESTINATION bin )
			  
	set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${SAMPLE_TARGETS_FOLDER} )
		  
endif ( ANDROID AND EXPERIMENTAL_USE_FASTCV )