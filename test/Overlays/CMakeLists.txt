

cet_test(DUNE_NanoSlice_t USE_BOOST_UNIT
  LIBRARIES dune-raw-data_Overlays 
  artdaq-core_Data
)

cet_test(DUNE_MicroSlice_t USE_BOOST_UNIT
  LIBRARIES dune-raw-data_Overlays
  artdaq-core_Data
)

cet_test(DUNE_MilliSlice_t USE_BOOST_UNIT
  LIBRARIES dune-raw-data_Overlays
  artdaq-core_Data
)

cet_test(DUNE_MilliSliceFragment_t USE_BOOST_UNIT
  LIBRARIES dune-raw-data_Overlays
  artdaq-core_Data 
)

cet_test(DUNE_FelixFragment_t USE_BOOST_UNIT
  LIBRARIES dune-raw-data_Overlays
  artdaq-core_Data 
)

