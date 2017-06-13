# project name
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

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
  )

install_headers()
install_source()
