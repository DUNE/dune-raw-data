#ifndef lbne_artdaq_Overlays_PennMilliSlice_hh
#define lbne_artdaq_Overlays_PennMilliSlice_hh

#include "lbne-raw-data/Overlays/PennMicroSlice.hh"
#include "artdaq-core/Data/Fragment.hh"

namespace lbne {
  class PennMilliSlice;
}

class lbne::PennMilliSlice {

public:

  struct Header {
    typedef uint32_t data_t;

    typedef uint16_t pattern_t;
    typedef uint16_t version_t;
    typedef uint32_t millislice_size_t;  
    typedef uint32_t microslice_count_t;  
    typedef uint16_t payload_count_t;

    // TODO finalise millislice header

    data_t fixed_pattern : 16;
    data_t version       : 16;

    data_t millislice_size : 32;   // total size, data & header

    data_t microslice_count : 32;

    data_t payload_count           : 16;
    data_t payload_count_counter   : 16;
    data_t payload_count_trigger   : 16;
    data_t payload_count_timestamp : 16;
  };

  // This constructor accepts a memory buffer that contains an existing
  // PennMilliSlice and allows the the data inside it to be accessed
  PennMilliSlice(uint8_t* address);

  // Returns the size of the MilliSlice
  Header::millislice_size_t size() const;

  // Returns the number of MicroSlices in this MilliSlice
  Header::microslice_count_t microSliceCount() const;

  // Returns the number of payloads in this MilliSlice
  Header::payload_count_t payloadCount() const;
  Header::payload_count_t payloadCount(Header::payload_count_t& counter, Header::payload_count_t& trigger, Header::payload_count_t& timestamp) const;

  // Returns the requested MicroSlice if the requested slice was found,
  // otherwise returns an empty pointer
  std::unique_ptr<PennMicroSlice> microSlice(uint32_t index) const;

  // Returns the requested Payload if found,
  // otherwise returns an empty pointer
  uint8_t* payload(uint32_t index, lbne::PennMicroSlice::Payload_Header::data_packet_type_t& data_packet_type,
		   lbne::PennMicroSlice::Payload_Header::short_nova_timestamp_t& short_nova_timestamp,
		   size_t& payload_size) const;

protected:

  // returns a pointer to the header
  Header const* header_() const;

  // returns a pointer to the requested MicroSlice
  uint8_t* data_(int index) const;

  uint8_t* buffer_;
};

#endif /* lbne_artdaq_Overlays_PennMilliSlice_hh */
