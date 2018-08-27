add_executable(FELIXdecode FELIXdecode.cc)
target_link_libraries(FELIXdecode
  dune-raw-data_Overlays
  ${ARTDAQ-CORE_DATA}
  pthread)