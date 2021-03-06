# define ROOTLIB
set( ROOTLIB -L$ENV{ROOTSYS}/lib -lCore -lCint -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -pthread -lm -ldl)

set( SUPERA_LIB -L$ENV{LARCAFFE_LIBDIR} -lLArCaffe -lSuperaCore -L$ENV{PROTOBUF_LIBDIR} -lprotobuf -L$ENV{LMDB_LIBDIR} -llmdb)

set( UBOONECODE_LIB -L$ENV{UBOONECODE_LIB} )

include_directories ( $ENV{PROTOBUF_INCDIR} $ENV{LMDB_INCDIR} $ENV{LARCAFFE_INCDIR} )

link_libraries( ${LIB_NAME} -L$ENV{BOOST_LIB} -lboost_system ${ROOTLIB} ${SUPERA_LIB})

#add_subdirectory(job)

art_make( BASENAME_ONLY
	  LIBRARY_NAME     uboone_Supera
	  LIB_LIBRARIES
	  	   Geometry
		   uboone_Geometry
		   Geometry_service
	   	   Simulation
		   SimulationBase
		   Utilities
		   TimeService_service
		   RawData
		   MCBase
		   SummaryData
		   RecoBase
		   RecoObjects
		   LArProperties_service
		   DetectorProperties_service
		   ${UBOONECODE_LIB}
		   ${SIMULATIONBASE}
		   ${LARRECO_LIB}
		   ${LARDATA_LIB}
		   ${ART_FRAMEWORK_CORE}
		   ${ART_FRAMEWORK_PRINCIPAL}
                   ${ART_FRAMEWORK_BASIC}
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

          MODULE_LIBRARIES 	  		   
	           uboone_Supera
		   Geometry
		   uboone_Geometry
		   Geometry_service
		   TimeService_service
		   Simulation
		   SimulationBase
		   Utilities
		   MCBase
		   SummaryData
		   RawData
		   RecoBase
		   RecoObjects
		   LArProperties_service
		   DetectorProperties_service
		   ${UBOONECODE_LIB}
		   ${SIMULATIONBASE}
		   ${LARRECO_LIB}
		   ${LARDATA_LIB}
		   ${ART_FRAMEWORK_CORE}
		   ${ART_FRAMEWORK_PRINCIPAL}
                   ${ART_FRAMEWORK_BASIC}
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
		)

# install_headers()
install_fhicl()
install_source()
