

cet_test(NanoSlice_t USE_BOOST_UNIT
  LIBRARIES lbne-raw-data_Overlays 
  ${ARTDAQ-CORE_DATA}
)

cet_test(MicroSlice_t USE_BOOST_UNIT
  LIBRARIES lbne-raw-data_Overlays
  ${ARTDAQ-CORE_DATA}
)

cet_test(MilliSlice_t USE_BOOST_UNIT
  LIBRARIES lbne-raw-data_Overlays
  ${ARTDAQ-CORE_DATA}
)

cet_test(MilliSliceFragment_t USE_BOOST_UNIT
  LIBRARIES lbne-raw-data_Overlays
  ${ARTDAQ-CORE_DATA} 
)
