#ifndef lbne_artdaq_Overlays_PennMilliSlice_hh
#define lbne_artdaq_Overlays_PennMilliSlice_hh

#include "lbne-raw-data/Overlays/PennMicroSlice.hh"
#include "artdaq-core/Data/Fragment.hh"

#include <boost/crc.hpp>

//#define PENN_DONT_REBLOCK_USLICES

namespace lbne {
  class PennMilliSlice;
}

class lbne::PennMilliSlice {

public:

  struct Header {
    //typedef uint32_t data_t;
    typedef uint64_t data_t;

    typedef uint16_t sequence_id_t;
    typedef uint16_t version_t;
    typedef uint32_t millislice_size_t;  
    typedef uint16_t payload_count_t;
    typedef uint64_t timestamp_t;
    typedef uint32_t ticks_t;
#ifdef PENN_DONT_REBLOCK_USLICES
    typedef uint32_t microslice_count_t;
#endif

    // TODO finalise millislice header

    data_t sequence_id     : 16;
    data_t version         : 16;
    data_t millislice_size : 32;   // total size, data & header
#ifdef PENN_DONT_REBLOCK_USLICES
    data_t microslice_count : 32;
#endif

    data_t payload_count           : 16;
    data_t payload_count_counter   : 16;
    data_t payload_count_trigger   : 16;
    data_t payload_count_timestamp : 16;

    data_t end_timestamp    : 64;
    data_t width_in_ticks   : 32;  // neglecting overlap
    data_t overlap_in_ticks : 32;
  };

  // This constructor accepts a memory buffer that contains an existing
  // PennMilliSlice and allows the the data inside it to be accessed
  PennMilliSlice(uint8_t* address);

  // Returns the size of the MilliSlice
  Header::millislice_size_t size() const;

  // Returns the sequence ID of the MilliSlice
  Header::sequence_id_t sequenceID() const;

  // Returns the version of the MilliSlice
  Header::version_t version() const;

  // Returns the timestamp marking the end of the MilliSlice
  Header::timestamp_t endTimestamp() const;

  // Returns the number of ticks (neglecting overlaps) in the MilliSlice
  Header::ticks_t widthTicks() const;

  // Returns the number of ticks in the overlap region of the MilliSlice
  Header::ticks_t overlapTicks() const;

  // Returns the number of payloads in this MilliSlice
  Header::payload_count_t payloadCount() const;
  Header::payload_count_t payloadCount(Header::payload_count_t& counter, Header::payload_count_t& trigger, Header::payload_count_t& timestamp
				       //, Header::payload_count_t& selftest, Header::payload_count_t& checksum
				       ) const;

#ifdef PENN_DONT_REBLOCK_USLICES
  // Returns the number of MicroSlices in this MilliSlice
  Header::microslice_count_t microSliceCount() const;

  // Returns the requested MicroSlice if the requested slice was found,
  // otherwise returns an empty pointer
  std::unique_ptr<PennMicroSlice> microSlice(uint32_t index) const;
#endif

  // Returns the requested Payload if found,
  // otherwise returns an empty pointer
  uint8_t* payload(uint32_t index, lbne::PennMicroSlice::Payload_Header::data_packet_type_t& data_packet_type,
		   lbne::PennMicroSlice::Payload_Header::short_nova_timestamp_t& short_nova_timestamp,
		   size_t& payload_size) const;

  // Calculate a checksum for this millslice
  uint32_t calculateChecksum() const;

protected:

  // returns a pointer to the header
  Header const* header_() const;

  // returns a pointer to the requested MicroSlice
  uint8_t* data_(int index) const;

  uint8_t* buffer_;
};

#endif /* lbne_artdaq_Overlays_PennMilliSlice_hh */
