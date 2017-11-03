# project name
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories(  "/nfs/sw/rce/proto-dune-dam-lib/install/x86_64-avx2-linux/include" )

find_library(RCE_DAM_LIB protodune-dam 
    HINTS "/nfs/sw/rce/proto-dune-dam-lib/install/x86_64-avx2-linux/lib")

project(dune-raw-data_Overlays)

art_make(BASENAME_ONLY
               LIBRARY_NAME dune-raw-data_Overlays
               LIB_LIBRARIES
  ${ARTDAQ_DAQDATA}
  ${CETLIB_EXCEPT}
  ${Boost_SYSTEM_LIBRARY}
  ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${MF_MESSAGEUTILITIES}
  ${RCE_DAM_LIB}
  )

install_headers()
install_source()
