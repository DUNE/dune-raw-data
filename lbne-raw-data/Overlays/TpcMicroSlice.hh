#ifndef lbne_artdaq_Overlays_TpcMicroSlice_hh
#define lbne_artdaq_Overlays_TpcMicroSlice_hh

#include "lbne-raw-data/Overlays/TpcNanoSlice.hh"
#include <memory>

namespace lbne {
  class TpcMicroSlice;
}

class lbne::TpcMicroSlice {

public:

  struct Header {

	// Generic raw header word type
    typedef uint32_t data_t;

    // Raw header word 0 fields
    typedef uint32_t microslice_size_t;

    // Raw header word 1 fields
    typedef uint32_t sequence_id_t;

    // Parameters derived from header field translation
    typedef uint32_t nanoslice_count_t;

    static constexpr size_t raw_header_words = 2;
    static constexpr uint8_t groups_per_microslice = 4;

    // Raw header words
    data_t raw_header_data[raw_header_words];

  };

  // This constructor accepts a memory buffer that contains an existing
  // TpcMicroSlice and allows the the data inside it to be accessed
  TpcMicroSlice(uint8_t* address);

  // Returns the size of the TpcMicroSlice
  Header::microslice_size_t size() const;

  // Returns the sequence ID of the TpcMicroSlice
  Header::sequence_id_t sequence_id() const;

  // Returns the number of nanoslices in the microslice
  Header::nanoslice_count_t nanoSliceCount() const;

  // Returns the requested NanoSlice if the requested slice was found,
  // otherwise returns an empty pointer
  std::unique_ptr<TpcNanoSlice> nanoSlice(uint32_t index) const;

protected:

  // returns a pointer to the header
  Header const* header_() const;

  // returns a pointer to the requested NanoSlice
  uint8_t* data_(uint32_t index) const;

  uint8_t* buffer_;
};

#endif /* lbne_artdaq_Overlays_TpcMicroSlice_hh */
