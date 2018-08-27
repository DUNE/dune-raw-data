

cet_test(DUNE_NanoSlice_t USE_BOOST_UNIT
  LIBRARIES dune-raw-data_Overlays 
  ${ARTDAQ-CORE_DATA}
)

cet_test(DUNE_MicroSlice_t USE_BOOST_UNIT
  LIBRARIES dune-raw-data_Overlays
  ${ARTDAQ-CORE_DATA}
)

cet_test(DUNE_MilliSlice_t USE_BOOST_UNIT
  LIBRARIES dune-raw-data_Overlays
  ${ARTDAQ-CORE_DATA}
)

cet_test(DUNE_MilliSliceFragment_t USE_BOOST_UNIT
  LIBRARIES dune-raw-data_Overlays
  ${ARTDAQ-CORE_DATA} 
)

cet_test(DUNE_FelixFragment_t USE_BOOST_UNIT
  LIBRARIES dune-raw-data_Overlays
  ${ARTDAQ-CORE_DATA}
  pthread
)

cet_test(DUNE_FelixReorder_t USE_BOOST_UNIT
  LIBRARIES dune-raw-data_Overlays
  ${ARTDAQ-CORE_DATA}
  pthread
)

cet_test(DUNE_FelixAvxReorder_t USE_BOOST_UNIT
  LIBRARIES dune-raw-data_Overlays
  ${ARTDAQ-CORE_DATA}
  pthread
)

cet_test(DUNE_FelixDecode_t USE_BOOST_UNIT
  LIBRARIES dune-raw-data_Overlays
  ${ARTDAQ-CORE_DATA}
  ${GALLERY_LIB}
  pthread
)
