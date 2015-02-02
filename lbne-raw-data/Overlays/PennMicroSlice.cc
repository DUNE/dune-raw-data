#include "lbne-raw-data/Overlays/PennMicroSlice.hh"
#include <iostream>
#include <iomanip>
#include <stdio.h>

lbne::PennMicroSlice::PennMicroSlice(uint8_t* address) : buffer_(address) 
{
}

lbne::PennMicroSlice::microslice_size_t lbne::PennMicroSlice::size() const
{
//	microslice_size_t raw_payload_words = raw_payload_words_uncompressed;
//	return (lbne::PennMicroSlice::microslice_size_t)((Header::raw_header_words + raw_payload_words)* sizeof(raw_data_word_t));
	return lbne::PennMicroSlice::block_size();
}

lbne::PennMicroSlice::Header::format_version_t lbne::PennMicroSlice::format_version() const
{
  //uint8_t header_format_version = (header_()->raw_header_data[0] >> 56) & 0xF;
  //return static_cast<lbne::PennMicroSlice::Header::format_version_t>(header_format_version);
  return header_()->format_version;
}

lbne::PennMicroSlice::Header::sequence_id_t lbne::PennMicroSlice::sequence_id() const
{
  return header_()->sequence_id;
}

lbne::PennMicroSlice::Header::block_size_t lbne::PennMicroSlice::block_size() const
{
  return header_()->block_size;
}

/*
lbne::PennMicroSlice::Payload_Header::data_packet_type_t lbne::PennMicroSlice::data_packet_type() const
{
  return header_()->data_packet_type;
}

lbne::PennMicroSlice::Payload_Header::short_nova_timestamp_t lbne::PennMicroSlice::short_nova_timestamp() const
{
  return header_()->short_nova_timestamp;
}
*/

// Returns the sample count in the microslice
lbne::PennMicroSlice::sample_count_t lbne::PennMicroSlice::sampleCount(
								       lbne::PennMicroSlice::sample_count_t &n_counter_words,
								       lbne::PennMicroSlice::sample_count_t &n_trigger_words,
								       lbne::PennMicroSlice::sample_count_t &n_timestamp_words) const
{
  n_counter_words = n_trigger_words = n_timestamp_words = 0;
  uint64_t i = 0;
  while(true) {
    lbne::PennMicroSlice::Payload_Header::data_packet_type_t type = reinterpret_cast<Payload_Header const *>(data_()[i])->data_packet_type;
    if(type == 0x01) {
      n_counter_words++;
      i += lbne::PennMicroSlice::payload_size_counter / sizeof(uint32_t);
    }
    else if(type == 0x02) {
      n_trigger_words++;
      i += lbne::PennMicroSlice::payload_size_trigger / sizeof(uint32_t);
    }
    else if(type == 0x08) {
      n_timestamp_words++;
      i += lbne::PennMicroSlice::payload_size_timestamp / sizeof(uint32_t);
      break;
    }
    else {
      std::cerr << "Unknown data packet type found " << std::hex << type << std::endl;
      return -1;
    }
  }//while true
  return n_counter_words + n_trigger_words + n_timestamp_words;
}


// Returns a pointer to the raw data words in the microslice for diagnostics
uint32_t* lbne::PennMicroSlice::raw() const
{
	return reinterpret_cast<uint32_t*>(buffer_);
}

// Returns a pointer to the microslice header
lbne::PennMicroSlice::Header const* lbne::PennMicroSlice::header_() const
{
  return reinterpret_cast<Header const *>(buffer_);
}

// Returns a pointer to the first payload data word in the microslice
uint32_t const* lbne::PennMicroSlice::data_() const
{
  return reinterpret_cast<uint32_t const*>(buffer_ + sizeof(lbne::PennMicroSlice::Header));
}
