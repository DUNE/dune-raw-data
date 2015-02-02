#ifndef lbne_artdaq_Overlays_PennMicroSlice_hh
#define lbne_artdaq_Overlays_PennMicroSlice_hh

#include <stdint.h>
#include <stddef.h>

namespace lbne {
  class PennMicroSlice;
}

class lbne::PennMicroSlice {

public:

  struct Header {

    typedef uint32_t data_t;
    
    typedef uint8_t  format_version_t;
    typedef uint8_t  sequence_id_t;
    typedef uint16_t block_size_t;

    format_version_t format_version : 8;
    sequence_id_t    sequence_id    : 8;
    block_size_t     block_size     : 16;

    static size_t const size_words = sizeof(data_t);

    //static constexpr size_t raw_header_words = 1;
    //data_t raw_header_data[raw_header_words];
  };

  struct Payload_Header {

    typedef uint32_t data_t;

    typedef uint8_t  data_packet_type_t;
    typedef uint32_t short_nova_timestamp_t;

    uint32_t data_packet_type     : 4;
    uint32_t short_nova_timestamp : 28;

    static size_t const size_words = sizeof(data_t);
  };

  typedef Header::block_size_t microslice_size_t;

  //the size of the payloads (neglecting the Payload_Header)
  microslice_size_t payload_size_counter   = 3 * sizeof(uint32_t); //96-bit payload
  microslice_size_t payload_size_trigger   = 1 * sizeof(uint32_t); //32-bit payload
  microslice_size_t payload_size_timestamp = 2 * sizeof(uint32_t); //64-bit payload


  // This constructor accepts a memory buffer that contains an existing
  // microSlice and allows the the data inside it to be accessed
  PennMicroSlice(uint8_t* address);


  // Returns the format version field from the header
  Header::format_version_t format_version() const;

  // Returns the sequence ID field from the header
  Header::format_version_t sequence_id() const;

  // Returns the block size field from the header
  Header::block_size_t block_size() const;

  /*
  // Returns the data packet type from the payload header
  Payload_Header::data_packet_type_t data_packet_type() const;

  // Returns the short nova timestamp from the payload header
  Payload_Header::short_nova_timestamp_t short_nova_timestamp() const;
  */

  // Returns the size of the PennMicroSlice
  lbne::PennMicroSlice::microslice_size_t size() const;

  // Returns the number of samples in the microslice
  typedef uint32_t sample_count_t;
  sample_count_t sampleCount(sample_count_t &n_counter_words, sample_count_t &n_trigger_words, sample_count_t &n_timestamp_words) const;

  // Fetches the value for the requested sample.  Returns true if
  // the requested value was found, false if not.
  bool sampleValue(uint32_t channel, uint16_t& value) const;

  static uint64_t getMask(int param){
	uint64_t mask=0;
	mask = (1 << param) - 1;//sets the mask to 0000...11111...11
	return mask;
  };

  uint32_t* raw() const;

protected:

  // returns a pointer to the header
  Header const* header_() const;

  // returns a pointer to the first sample word
  uint32_t const* data_() const;

  uint8_t* buffer_;
};

#endif /* lbne_artdaq_Overlays_PennMicroSlice_hh */
