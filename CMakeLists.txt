cmake_minimum_required (VERSION 2.6)

string(ASCII 27 Esc)
set(ColorReset "${Esc}[m")
set(Yellow      "${Esc}[33m")

#
# Some useful variables
#
execute_process(COMMAND root-config --incdir OUTPUT_VARIABLE ROOT_INCDIR OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND root-config --libs OUTPUT_VARIABLE ROOT_LIB OUTPUT_STRIP_TRAILING_WHITESPACE)

#
# Set external dependency flags
#
set(LMDB_INCDIR $ENV{LMDB_INCDIR})
set(LMDB_LDFLAGS "-L$ENV{LMDB_LIBDIR} -llmdb")

set(PROTOBUF_INCDIR $ENV{PROTOBUF_INCDIR})
set(PROTOBUF_LDFLAGS "-L$ENV{PROTOBUF_LIBDIR} -lprotobuf")

set(USE_ROOT6 $ENV{LARCAFFE_ROOT6})

#
# Set directories
#
set( LARCAFFE_INCDIR ${CMAKE_SOURCE_DIR}               )
set( LARCAFFE_LIBDIR $ENV{LARCAFFE_LIBDIR}             )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LARCAFFE_LIBDIR} )
execute_process(COMMAND mkdir -p ${LARCAFFE_LIBDIR}    )

Project (FakeSupera)
add_subdirectory(LArCaffe)
add_subdirectory(SuperaCore)
add_subdirectory(LLSupera)


