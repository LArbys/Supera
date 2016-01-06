# find_ups_product is meant to be called from the top CMakeLists.txt file

set( PROTOBUF_INC $ENV{PROTOBUF_HOME}/include )
set( PROTOBUF_LIB $ENV{PROTOBUF_HOME}/lib/libprotobuf.so )
set( LMDB_INC $ENV{LMDB_HOME} )
set( LMDB_LIB $ENV{LMDB_HOME}/liblmdb.so )

include_directories( ${PROTOBUF_INC} ${LMDB_INC} )

art_make( BASENAME_ONLY
	  LIBRARY_NAME  uboone_Supera
	  EXCLUDE Supera_module.cc
	  LIB_LIBRARIES OpticalDetector
	  	        Geometry_service
                        RawData
			RecoBase
			Utilities			
                        ${ART_FRAMEWORK_CORE}
			${ART_FRAMEWORK_IO_SOURCES}
                        ${ART_FRAMEWORK_PRINCIPAL}
                        ${ART_FRAMEWORK_SERVICES_REGISTRY}
                        ${ART_FRAMEWORK_SERVICES_OPTIONAL}
                        ${ART_FRAMEWORK_SERVICES_OPTIONAL_TFILESERVICE_SERVICE}
                        ${ART_PERSISTENCY_COMMON}
                        ${ART_PERSISTENCY_PROVENANCE}
                        ${ART_UTILITIES}
                        ${MF_MESSAGELOGGER}
                        ${MF_UTILITIES}
                        ${FHICLCPP}
                        ${CETLIB}
                        ${ROOT_GEOM}
                        ${ROOT_XMLIO}
                        ${ROOT_GDML}
                        ${ROOT_BASIC_LIB_LIST}
			${PROTOBUF_LIB}
			${LMDB_LIB}
        )							   

simple_plugin( Supera "module"
	       uboone_Supera
	       RawData
	       RecoBase
	       Geometry
	       Geometry_service
	       Utilities
               ${ART_FRAMEWORK_CORE}
	       ${ART_FRAMEWORK_IO_SOURCES}
               ${ART_FRAMEWORK_PRINCIPAL}
               ${ART_FRAMEWORK_SERVICES_REGISTRY}
               ${ART_FRAMEWORK_SERVICES_OPTIONAL}
               ${ART_FRAMEWORK_SERVICES_OPTIONAL_TFILESERVICE_SERVICE}
               ${ART_PERSISTENCY_COMMON}
               ${ART_PERSISTENCY_PROVENANCE}
               ${ART_UTILITIES}
	       ${ROOT_BASIC_LIB_LIST}
	       ${PROTOBUF_LIB}
	       ${LMDB_LIB}
               BASENAME_ONLY
             )


install_headers()
install_source()

install_fhicl()
FILE(GLOB fcl_files *.fcl)
install_source( EXTRAS ${fcl_files} )
