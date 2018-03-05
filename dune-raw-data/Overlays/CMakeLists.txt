# project name
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

include_directories( "$ENV{DUNEPDSPRCE_INC}" ) 
cet_find_library( RCEDAMLIB NAMES protodune-dam PATHS ENV DUNEPDSPRCE_LIB NO_DEFAULT_PATH )

# a cmake version of a test in JJ Russell's define_tools.mk.  Unfortunately this is needed
# whenever anyone includes protodune dam lib includes.
# haven't tested this with the clang compiler as we distribute gcc on mac

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCLASS_MEMBER_UNUSED=unused")
else(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCLASS_MEMBER_UNUSED=")
endif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")

project(dune-raw-data_Overlays)

art_make(BASENAME_ONLY
               LIBRARY_NAME dune-raw-data_Overlays
               LIB_LIBRARIES
               artdaq-core_Data
  ${ARTDAQ_DAQDATA}
  ${CETLIB_EXCEPT}
  ${Boost_SYSTEM_LIBRARY}
  ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${MF_MESSAGEUTILITIES}
  ${RCEDAMLIB}
  pthread
  )

install_headers()
install_source()
