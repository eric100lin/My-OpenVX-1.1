include_directories( BEFORE
                     ${CMAKE_CURRENT_SOURCE_DIR} 
					 ${CMAKE_SOURCE_DIR}/include
					 ${OPENCV_INCLUDE_PATH}
					 ${CMAKE_SOURCE_DIR}/..)
					 
# set target name
set( TARGET_NAME_SCHEDULING_CASE vx_scheduling )

FIND_SOURCES()

# add executable ${TARGET_NAME_SCHEDULING_CASE}
add_executable (${TARGET_NAME_SCHEDULING_CASE} ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries (${TARGET_NAME_SCHEDULING_CASE} ${TARGET_NAME_MYAPP_LIB})

install ( TARGETS ${TARGET_NAME_SCHEDULING_CASE} 
          RUNTIME DESTINATION bin
          ARCHIVE DESTINATION bin
          LIBRARY DESTINATION bin )

set_target_properties( ${TARGET_NAME_SCHEDULING_CASE} PROPERTIES FOLDER ${MYAPPS_FOLDER} )