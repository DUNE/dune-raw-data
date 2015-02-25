# project name
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

project(lbne-raw-data_Overlays)

art_make(BASENAME_ONLY
               LIBRARY_NAME lbne-raw-data_Overlays
               LIB_LIBRARIES
  ${ARTDAQ_DAQDATA}
  ${CETLIB}
  ${Boost_SYSTEM_LIBRARY}
  )

install_headers()
install_source()
