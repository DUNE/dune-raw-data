#ifndef lbne_artdaq_Overlays_PennMillisliceWriter_hh
#define lbne_artdaq_Overlays_PennMilliSliceWriter_hh

#include "lbne-raw-data/Overlays/PennMilliSlice.hh"
#include "lbne-raw-data/Overlays/PennMicroSliceWriter.hh"

namespace lbne {
  class PennMilliSliceWriter;
}

class lbne::PennMilliSliceWriter : public lbne::PennMilliSlice {

public:

  // This constructor creates an empty PennMilliSlice which can be filled
  // with the appropriate data
  PennMilliSliceWriter(uint8_t* address, uint32_t max_size_bytes);

  // Reserves the next MicroSlice in memory within this PennMilliSlice.
  // The MicroSliceWriter that is returned is initialized to an empty
  // state and is ready to be populated with data.  This method returns
  // true if the MicroSlice was successfully added, false if not (for example,
  // if the additional MicroSlice would overflow the maximum size of the
  // PennMilliSlice).
  std::shared_ptr<PennMicroSliceWriter> reserveMicroSlice(uint32_t ms_max_bytes);

  // Finalizes the PennMilliSlice.  This takes care of updating the
  // PennMilliSlice so that all internal pointers and data are consistent
  // with the MicroSlices that have been added.  No more MicroSlices
  // can be added once the PennMilliSlice has been finalized.
  // This method returns the number of bytes that were reclaimed
  // when the maximum size for this PennMilliSlice was reduced to match
  // its actual current size.
  int32_t finalize(bool override = false, uint32_t data_size_bytes = 0, uint32_t microslice_count = 0,
		   uint16_t payload_count = 0, uint16_t payload_count_counter = 0,
		   uint16_t payload_count_trigger = 0, uint16_t payload_count_timestamp = 0);

protected:

  // finalizes the MicroSlice that was most recently added and
  // updates our size to reflect any memory that was reclaimed
  // by finalizing the MicroSlice
  void finalizeLatestMicroSlice_();

  // returns a pointer to the header
  Header* header_();

  // returns a pointer to the requested MicroSlice
  uint8_t* data_(int index);

  uint32_t max_size_bytes_;
  std::shared_ptr<PennMicroSliceWriter> latest_microslice_ptr_;
};

#endif /* lbne_artdaq_Overlays_PennMilliSliceWriter_hh */
