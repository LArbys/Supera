string(ASCII 27 Esc)
set(ColorReset "${Esc}[m")
set(Yellow      "${Esc}[33m")

#cmake_minimum_required (VERSION 2.8)
project (LArCaffe)

# Now set CMAKE variables for my use
set(SHLIB "libLArCaffe.so")
set(CMAKE_MACOSX_RPATH TRUE)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "./lib")
set(DICT_NAME "LArCaffeDict")
if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/${DICT_NAME}.cxx)
  MESSAGE("Removing a dictionary file: ${Yellow}${DICT_NAME}.cxx${ColorReset}")
  file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/${DICT_NAME}.cxx)
endif()
if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/${DICT_NAME}.h)
  MESSAGE("Removing a dictionary file: ${Yellow}${DICT_NAME}.h${ColorReset}")
  file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/${DICT_NAME}.h)
endif()

# Get list of course
file( GLOB HEADERS *.h  )
file( GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cxx  )

file( GLOB LINKDEF LinkDef.h )
list( GET LINKDEF 0 LINKDEF )
list( REMOVE_ITEM HEADERS ${LINKDEF} )

# Pre process to generate dictionary source code + getting compiler/linker flags
IF(${USE_ROOT6})
execute_process(COMMAND rootcling
		-f ${CMAKE_CURRENT_BINARY_DIR}/${DICT_NAME}.cxx
		-s   ${LARCAFFE_LIBDIR}/${SHLIB}
		-rml ${SHLIB}
		-rmf ${LARCAFFE_LIBDIR}/libLArCaffe.rootmap
		-c ${CINTFLAGS} -I${ROOT_INCDIR} -I${LARCAFFE_INCDIR} -I${CMAKE_CURRENT_SOURCE_DIR}
		${HEADERS} ${LINKDEF})
ELSE()
execute_process(COMMAND rootcint
		-f ${CMAKE_CURRENT_BINARY_DIR}/${DICT_NAME}.cxx
		-c -I${ROOT_INCDIR} -I${LARCAFFE_INCDIR} -I${CMAKE_CURRENT_SOURCE_DIR}
		-p  ${HEADERS} ${LINKDEF})
ENDIF()

# Set include dirs
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${LARCAFFE_INCDIR})
include_directories(${ROOT_INCDIR})
set(CMAKE_CXX_FLAGS "-std=c++11")

# Generate a package library ... make sure dictionary comes the end
add_library(LArCaffe SHARED ${SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/${DICT_NAME}.cxx)
target_link_libraries(LArCaffe ${ROOT_LIB} )

set_target_properties( LArCaffe
		       PROPERTIES
	               LIBRARY_OUTPUT_DIRECTORY ${LARCAFFE_LIBDIR}
)
if (APPLE)
   set_property(TARGET LArCaffe PROPERTY PREFIX "lib")
   set_property(TARGET LArCaffe PROPERTY OUTPUT_NAME "LArCaffe.so")
   set_property(TARGET LArCaffe PROPERTY SUFFIX "")
endif()


# Generate a rootmap file
IF(NOT ${USE_ROOT6}) 
add_custom_command( TARGET LArCaffe
		    POST_BUILD
		    COMMAND rlibmap ARGS -f
		    -o ${LARCAFFE_LIBDIR}/libLArCaffe.rootmap
		    -l ${LARCAFFE_LIBDIR}/libLArCaffe.so
		    -c ${CMAKE_CURRENT_SOURCE_DIR}/LinkDef.h
)
ENDIF()
set_directory_properties( PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_CURRENT_SOURCE_DIR}/lib/libLArCaffe.rootmap" )

