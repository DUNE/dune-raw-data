#include "lbne-raw-data/Overlays/PennMicroSlice.hh"
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <boost/asio.hpp>

//#define __DEBUG_sampleCount__
//#define __DEBUG_sampleTimeSplit__
//#define __DEBUG_sampleTimeSplitAndCount__

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
					   size_t& payload_size,
					   bool swap_payload_header_bytes,
					   size_t override_uslice_size) const
{
  uint32_t i = 0;

  //if we're overriding, we don't have a Header to offset by
  uint8_t* pl_ptr = 0;
  size_t   pl_size = 0;
  if(override_uslice_size) {
    pl_ptr  = buffer_;
    pl_size = override_uslice_size - sizeof(Header);
  }
  else {
    pl_ptr  = buffer_ + sizeof(Header);
    pl_size = size();
  }

  while(pl_ptr < (buffer_ + pl_size)) {
    if(swap_payload_header_bytes)
      *((uint32_t*)pl_ptr) = ntohl(*((uint32_t*)pl_ptr));
    lbne::PennMicroSlice::Payload_Header* payload_header = reinterpret_cast<lbne::PennMicroSlice::Payload_Header*>(pl_ptr);
    lbne::PennMicroSlice::Payload_Header::data_packet_type_t type = payload_header->data_packet_type;
    if(i == word_id) {
      data_packet_type = type;
      short_nova_timestamp = payload_header->short_nova_timestamp;
      pl_ptr += 4;
      switch(type)
	{
	case lbne::PennMicroSlice::DataTypeCounter:
	  payload_size = lbne::PennMicroSlice::payload_size_counter;
	  break;
	case lbne::PennMicroSlice::DataTypeTrigger:
	  payload_size = lbne::PennMicroSlice::payload_size_trigger;
	  break;
	case lbne::PennMicroSlice::DataTypeTimestamp:
	  payload_size = lbne::PennMicroSlice::payload_size_timestamp;
	  break;
	default:
	  std::cerr << "Unknown data packet type found 0x" << std::hex << (unsigned int)type << std::endl;
	  payload_size = 0;
	  break;
	}//switch(type)
      return pl_ptr;
    }
    switch(type)
      {
      case lbne::PennMicroSlice::DataTypeCounter:
	pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_counter;
	break;
      case lbne::PennMicroSlice::DataTypeTrigger:
	pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_trigger;
	break;
      case lbne::PennMicroSlice::DataTypeTimestamp:
	pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_timestamp;
	break;
      default:
	std::cerr << "Unknown data packet type found 0x" << std::hex << (unsigned int)type << std::endl;
	return 0;
	break;
      }//switch(type)
    i++;
  }
  std::cerr << "Could not find payload with ID " << word_id << " (the data buffer has overrun)" << std::endl;
  return 0;
}


// Returns the sample count in the microslice
lbne::PennMicroSlice::sample_count_t lbne::PennMicroSlice::sampleCount(
								       lbne::PennMicroSlice::sample_count_t &n_counter_words,
								       lbne::PennMicroSlice::sample_count_t &n_trigger_words,
								       lbne::PennMicroSlice::sample_count_t &n_timestamp_words,
								       bool swap_payload_header_bytes,
								       size_t override_uslice_size) const
{
  n_counter_words = n_trigger_words = n_timestamp_words = 0;

  //if we're overriding, we don't have a Header to offset by
  uint8_t* pl_ptr = 0;
  size_t   pl_size = 0;
  if(override_uslice_size) {
    pl_ptr  = buffer_;
    pl_size = override_uslice_size - sizeof(Header);
  }
  else {
    pl_ptr  = buffer_ + sizeof(Header);
    pl_size = size();
  }

  while(pl_ptr < (buffer_ + pl_size)) {
    if(swap_payload_header_bytes)
      *((uint32_t*)pl_ptr) = ntohl(*((uint32_t*)pl_ptr));
    lbne::PennMicroSlice::Payload_Header* payload_header = reinterpret_cast<lbne::PennMicroSlice::Payload_Header*>(pl_ptr);
    lbne::PennMicroSlice::Payload_Header::data_packet_type_t type = payload_header->data_packet_type;
#ifdef __DEBUG_sampleCount__
    std::cout << "PennMicroSlice::sampleCount DEBUG type 0x" << std::hex << (unsigned int)type << " timestamp " << std::dec << payload_header->short_nova_timestamp << std::endl;
#endif
    switch(type)
      {
      case lbne::PennMicroSlice::DataTypeCounter:
	n_counter_words++;
	pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_counter;
	break;
      case lbne::PennMicroSlice::DataTypeTrigger:
	n_trigger_words++;
	pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_trigger;
	break;
      case lbne::PennMicroSlice::DataTypeTimestamp:
	n_timestamp_words++;
	pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_timestamp;
	break;
      default:
	std::cerr << "Unknown data packet type found 0x" << std::hex << (unsigned int)type << std::endl;
	return 0;
	break;
      }//switch(type)
  }
  return n_counter_words + n_trigger_words + n_timestamp_words;
}

//Returns a point to the first payload header AFTER boundary_time
uint8_t* lbne::PennMicroSlice::sampleTimeSplit(uint64_t boundary_time, size_t& remaining_size,
					       bool swap_payload_header_bytes, size_t override_uslice_size) const
{
  //need to mask to get the lowest 28 bits of the nova timestamp
  //in order to compare with the 'short_nova_timestamp' in the Payload_Header
  boundary_time = boundary_time & 0xFFFFFFF;

  //if we're overriding, we don't have a Header to offset by
  uint8_t* pl_ptr = 0;
  size_t   pl_size = 0;
  if(override_uslice_size) {
    pl_ptr  = buffer_;
    pl_size = override_uslice_size - sizeof(Header);
  }
  else {
    pl_ptr  = buffer_ + sizeof(Header);
    pl_size = size();
  }

  //loop over the microslice
  while(pl_ptr < (buffer_ + pl_size)) {
    if(swap_payload_header_bytes)
      *((uint32_t*)pl_ptr) = ntohl(*((uint32_t*)pl_ptr));
    lbne::PennMicroSlice::Payload_Header* payload_header = reinterpret_cast<lbne::PennMicroSlice::Payload_Header*>(pl_ptr);
    lbne::PennMicroSlice::Payload_Header::data_packet_type_t     type      = payload_header->data_packet_type;
    lbne::PennMicroSlice::Payload_Header::short_nova_timestamp_t timestamp = payload_header->short_nova_timestamp;
#ifdef __DEBUG_sampleTimeSplit__
    std::cout << "PennMicroSlice::sampleTimeSplit DEBUG type 0x" << std::hex << (unsigned int)type << " timestamp " << std::dec << timestamp << std::endl;
#endif
    //check the timestamp
    if(timestamp > boundary_time) {
      //need to be careful and make sure that boundary_time hasn't overflowed the 28 bits
      //do this by checking whether timestamp isn't very large AND boundary_time isn't very small
      //TODO a better way to catch rollovers?
      if(!((boundary_time < lbne::PennMicroSlice::ROLLOVER_LOW_VALUE) && (timestamp > lbne::PennMicroSlice::ROLLOVER_HIGH_VALUE))) {
	remaining_size = (buffer_ + pl_size) - pl_ptr;
	return pl_ptr;
      }
    }
    //check the type, to increment
    switch(type)
      {
      case lbne::PennMicroSlice::DataTypeCounter:
	pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_counter;
	break;
      case lbne::PennMicroSlice::DataTypeTrigger:
	pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_trigger;
	break;
      case lbne::PennMicroSlice::DataTypeTimestamp:
	pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_timestamp;
	break;
      default:
	std::cerr << "Unknown data packet type found 0x" << std::hex << (unsigned int)type << std::endl;
	return 0;
	break;
      }//switch(type)
  }
  return 0;
}

//Returns a point to the first payload header AFTER boundary_time, and also counts payload types before/after the boundary
uint8_t* lbne::PennMicroSlice::sampleTimeSplitAndCount(uint64_t boundary_time, size_t& remaining_size,
						       lbne::PennMicroSlice::sample_count_t &n_words_b,
						       lbne::PennMicroSlice::sample_count_t &n_counter_words_b,
						       lbne::PennMicroSlice::sample_count_t &n_trigger_words_b,
						       lbne::PennMicroSlice::sample_count_t &n_timestamp_words_b,
						       lbne::PennMicroSlice::sample_count_t &n_words_a,
						       lbne::PennMicroSlice::sample_count_t &n_counter_words_a,
						       lbne::PennMicroSlice::sample_count_t &n_trigger_words_a,
						       lbne::PennMicroSlice::sample_count_t &n_timestamp_words_a,
						       bool swap_payload_header_bytes, size_t override_uslice_size) const
{
  n_words_b = n_counter_words_b = n_trigger_words_b = n_timestamp_words_b = 0;
  n_words_a = n_counter_words_a = n_trigger_words_a = n_timestamp_words_a = 0;
  remaining_size = 0;
  uint8_t* remaining_data_ptr = 0;
  bool is_before = true;

  //if we're overriding, we don't have a Header to offset by
  uint8_t* pl_ptr = 0;
  size_t   pl_size = 0;
  if(override_uslice_size) {
    pl_ptr  = buffer_;
    pl_size = override_uslice_size - sizeof(Header);
  }
  else {
    pl_ptr  = buffer_ + sizeof(Header);
    pl_size = size();
  }

  //loop over the microslice
  while(pl_ptr < (buffer_ + pl_size)) {
    if(swap_payload_header_bytes)
      *((uint32_t*)pl_ptr) = ntohl(*((uint32_t*)pl_ptr));
    lbne::PennMicroSlice::Payload_Header* payload_header = reinterpret_cast<lbne::PennMicroSlice::Payload_Header*>(pl_ptr);
    lbne::PennMicroSlice::Payload_Header::data_packet_type_t     type      = payload_header->data_packet_type;
    lbne::PennMicroSlice::Payload_Header::short_nova_timestamp_t timestamp = payload_header->short_nova_timestamp;
#ifdef __DEBUG_sampleTimeSplitAndCount__
    std::cout << "PennMicroSlice::sampleTimeSplitAndCount DEBUG type 0x" << std::hex << (unsigned int)type << " timestamp " << std::dec << timestamp << std::endl;
#endif
    //check the timestamp
    if(is_before && (timestamp > boundary_time)) {
      //need to be careful and make sure that boundary_time hasn't overflowed the 28 bits
      //do this by checking whether timestamp isn't very large AND boundary_time isn't very small
      //TODO a better way to catch rollovers?
      if(!((boundary_time < lbne::PennMicroSlice::ROLLOVER_LOW_VALUE) && (timestamp > lbne::PennMicroSlice::ROLLOVER_HIGH_VALUE))) {
	remaining_size     = (buffer_ + pl_size) - pl_ptr;
	remaining_data_ptr = pl_ptr;
	is_before = false;
      }
    }
    //check the type to increment counters & the ptr
    switch(type)
      {
      case lbne::PennMicroSlice::DataTypeCounter:
	if(is_before)
	  n_counter_words_b++;
	else
	  n_counter_words_a++;
	pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_counter;
	break;
      case lbne::PennMicroSlice::DataTypeTrigger:
	if(is_before)
	  n_trigger_words_b++;
	else
	  n_trigger_words_a++;
	pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_trigger;
	break;
      case lbne::PennMicroSlice::DataTypeTimestamp:
	if(is_before)
	  n_timestamp_words_b++;
	else
	  n_timestamp_words_a++;
	pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_timestamp;
	break;
      default:
	std::cerr << "Unknown data packet type found 0x" << std::hex << (unsigned int)type 
		  << std::dec << std::endl;
	return 0;
	break;
      }//switch(type)
  }
  n_words_b = n_counter_words_b + n_trigger_words_b + n_timestamp_words_b;
  n_words_a = n_counter_words_a + n_trigger_words_a + n_timestamp_words_a;
#ifdef __DEBUG_sampleTimeSplitAndCount__
  std::cout << "PennMicroSlice::sampleTimeSplitAndCount DEBUG returning with remaining size " << remaining_size << " for boundary_time " << boundary_time << std::endl;
#endif
  return remaining_data_ptr;
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
