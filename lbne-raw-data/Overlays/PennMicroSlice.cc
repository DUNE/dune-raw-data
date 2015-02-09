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
  return block_size();
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
  //return ((header_()->block_size & 0xFF00) >> 8) | ((header_()->block_size & 0x00FF) << 8);
  return header_()->block_size;
}

uint8_t* lbne::PennMicroSlice::get_payload(uint32_t word_id, lbne::PennMicroSlice::Payload_Header::data_packet_type_t& data_packet_type,
				       lbne::PennMicroSlice::Payload_Header::short_nova_timestamp_t& short_nova_timestamp,
				       size_t& payload_size) const {
  uint8_t* pl_ptr = buffer_ + sizeof(Header);
  uint32_t i = 0;
  while(pl_ptr < (buffer_ + size())) {
    lbne::PennMicroSlice::Payload_Header* payload_header = reinterpret_cast<lbne::PennMicroSlice::Payload_Header*>(pl_ptr);
    lbne::PennMicroSlice::Payload_Header::data_packet_type_t type = payload_header->data_packet_type;
    //uint8_t type = ((*pl_ptr) & 0xF0) >> 4;
    if(i == word_id) {
      data_packet_type = type;
      short_nova_timestamp = payload_header->short_nova_timestamp;
      pl_ptr += 4;
      /*
      short_nova_timestamp  = ((*pl_ptr) & 0x0F) << 24;
      pl_ptr++;
      short_nova_timestamp |= ((*pl_ptr) & 0xFF) << 16;
      pl_ptr++;
      short_nova_timestamp |= ((*pl_ptr) & 0xFF) << 8;
      pl_ptr++;
      short_nova_timestamp |= ((*pl_ptr) & 0xFF);
      pl_ptr++;
      */
      if(type == 0x01)
	payload_size = lbne::PennMicroSlice::payload_size_counter;
      else if(type == 0x02)
	payload_size = lbne::PennMicroSlice::payload_size_trigger;
      else if(type == 0x08)
	payload_size = lbne::PennMicroSlice::payload_size_timestamp;
      else
	payload_size = 0;
      return pl_ptr;
    }
    else if(type == 0x01) {
      pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_counter;
    }
    else if(type == 0x02) {
      pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_trigger;
    }
    else if(type == 0x08) {
      pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_timestamp;
      break;
    }
    else {
      std::cerr << "Unknown data packet type found " << std::hex << type << std::endl;
      return 0;
    }
    i++;
  }
  std::cerr << "Could not find payload with ID " << word_id << " (the data buffer has overrun)" << std::endl;
  return 0;
}


// Returns the sample count in the microslice
lbne::PennMicroSlice::sample_count_t lbne::PennMicroSlice::sampleCount(
								       lbne::PennMicroSlice::sample_count_t &n_counter_words,
								       lbne::PennMicroSlice::sample_count_t &n_trigger_words,
								       lbne::PennMicroSlice::sample_count_t &n_timestamp_words) const
{
  n_counter_words = n_trigger_words = n_timestamp_words = 0;
  uint8_t* pl_ptr = buffer_ + sizeof(Header);
  while(pl_ptr < (buffer_ + size())) {
    lbne::PennMicroSlice::Payload_Header* payload_header = reinterpret_cast<lbne::PennMicroSlice::Payload_Header*>(pl_ptr);
    lbne::PennMicroSlice::Payload_Header::data_packet_type_t type = payload_header->data_packet_type;
    //uint8_t type = ((*pl_ptr) & 0xF0) >> 4;
    //lbne::PennMicroSlice::Payload_Header::data_packet_type_t type = reinterpret_cast<Payload_Header const *>(data_()[i])->data_packet_type;
    //lbne::PennMicroSlice::Payload_Header::data_packet_type_t type = (reinterpret_cast<Payload_Header const *>(data_()[i])->short_nova_timestamp) & 0xF;
    if(type == 0x01) {
      n_counter_words++;
      pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_counter;
    }
    else if(type == 0x02) {
      n_trigger_words++;
      pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_trigger;
    }
    else if(type == 0x08) {
      n_timestamp_words++;
      pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_timestamp;
      break;
    }
    else {
      std::cerr << "Unknown data packet type found " << std::hex << type << std::endl;
      return -1;
    }
  }
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
