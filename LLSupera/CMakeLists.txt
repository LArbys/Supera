string(ASCII 27 Esc)
set(ColorReset "${Esc}[m")
set(Yellow      "${Esc}[33m")

#cmake_minimum_required (VERSION 2.8)
project (LLSupera)

# Now set CMAKE variables for my use
set(SHLIB "libLLSupera.so")
set(CMAKE_MACOSX_RPATH TRUE)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "./lib")
set(DICT_NAME "LLSuperaDict")
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
		-rmf ${LARCAFFE_LIBDIR}/libLLSupera.rootmap
		-c ${CINTFLAGS} -I${ROOT_INCDIR} -I${CMAKE_CURRENT_SOURCE_DIR} ${HEADERS} ${LINKDEF})
ELSE()
execute_process(COMMAND rootcint
                -f ${CMAKE_CURRENT_BINARY_DIR}/${DICT_NAME}.cxx
                -c -I${ROOT_INCDIR} -I${CMAKE_CURRENT_SOURCE_DIR}
                -p  ${HEADERS} ${LINKDEF})
ENDIF()

# Set include dirs
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${ROOT_INCDIR})
include_directories(${LMDB_INCDIR})
include_directories(${PROTOBUF_INCDIR})
include_directories(${LARCAFFE_INCDIR})
set(CMAKE_CXX_FLAGS "-std=c++11")

# Generate a package library ... make sure dictionary comes the end
add_library(LLSupera SHARED ${SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/${DICT_NAME}.cxx)
target_link_libraries(LLSupera ${ROOT_LIB} ${LMDB_LDFLAGS} ${PROTOBUF_LDFLAGS})

set_target_properties( LLSupera
		       PROPERTIES
	               LIBRARY_OUTPUT_DIRECTORY ${LARCAFFE_LIBDIR}
)
if (APPLE)
   set_property(TARGET LLSupera PROPERTY PREFIX "lib")
   set_property(TARGET LLSupera PROPERTY OUTPUT_NAME "LLSupera.so")
   set_property(TARGET LLSupera PROPERTY SUFFIX "")
endif()

# Generate a rootmap file
if(NOT ${USE_ROOT6})
add_custom_command( TARGET LLSupera
                    POST_BUILD
                    COMMAND rlibmap ARGS -f
                    -o ${LARCAFFE_LIBDIR}/libLLSupera.rootmap
                    -l ${LARCAFFE_LIBDIR}/lib/libLLSupera.so
                    -c ${CMAKE_CURRENT_SOURCE_DIR}/LinkDef.h)
endif()

set_directory_properties( PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${LARCAFFE_LIBDIR}/libLLSupera.rootmap" )

