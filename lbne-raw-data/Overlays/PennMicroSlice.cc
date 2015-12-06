#include "lbne-raw-data/Overlays/PennMicroSlice.hh"
#include "lbne-raw-data/Overlays/Utilities.hh"

#include "messagefacility/MessageLogger/MessageLogger.h"
#include "cetlib/exception.h"

#include <bitset>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <boost/asio.hpp>

//#define __DEBUG_sampleCount__
//#define __DEBUG_sampleTimeSplit__
//#define __DEBUG_sampleTimeSplitAndCount__
#define __DEBUG_sampleTimeSplitAndCountTwice__

lbne::PennMicroSlice::PennMicroSlice(uint8_t* address) : buffer_(address) , current_payload_(address), current_word_id_(0)
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

uint8_t* lbne::PennMicroSlice::get_next_payload(uint32_t& word_id,
						lbne::PennMicroSlice::Payload_Header::data_packet_type_t &data_packet_type,	
						lbne::PennMicroSlice::Payload_Header::short_nova_timestamp_t& short_nova_timestamp,
						size_t& payload_size,
						bool swap_payload_header_bytes,
						size_t override_uslice_size){
  size_t pl_size = 0;
  if(current_word_id_ == 0){
    if(override_uslice_size) {
      current_payload_ = buffer_;
      pl_size = override_uslice_size - sizeof(Header);
    }
    else {
      current_payload_ = buffer_ + sizeof(Header);
      pl_size = size();
    }
  }//if(current_word_id_ == 0)

  //Switch from network to host byte ordering //Copied from get_payload
  if(swap_payload_header_bytes)
    *((uint32_t*)current_payload_) = ntohl(*((uint32_t*)current_payload_));

  if(current_word_id_ != 0 ){
    //current_payload_ points to the last payload served up
    //We need to skip over this one
    lbne::PennMicroSlice::Payload_Header* payload_header = reinterpret_cast_checked<lbne::PennMicroSlice::Payload_Header*>(current_payload_);
    lbne::PennMicroSlice::Payload_Header::data_packet_type_t type = payload_header->data_packet_type;
    switch(type)
      {
      case lbne::PennMicroSlice::DataTypeCounter:
	current_payload_ += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_counter;
	break;
      case lbne::PennMicroSlice::DataTypeTrigger:
	current_payload_ += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_trigger;
	break;
      case lbne::PennMicroSlice::DataTypeTimestamp:
	current_payload_ += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_timestamp;
	break;
      case lbne::PennMicroSlice::DataTypeWarning:
	current_payload_ += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_selftest;
	break;
      case lbne::PennMicroSlice::DataTypeChecksum:
	current_payload_ += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_checksum;
	break;
      default:
	std::cerr << "Unknown data packet type found 0x" << std::hex << (unsigned int)type << std::endl;
	return nullptr;
	break;
      }//switch(type)
  }//(current_word_id_ != 0)

  //Now current_payload_ will point to the next payload. Process the information that we want

  //Need to make sure we have not gone off the end of the buffer
  if(current_payload_ >= (buffer_ + pl_size)) 
    return nullptr;
  

  lbne::PennMicroSlice::Payload_Header* payload_header = reinterpret_cast_checked<lbne::PennMicroSlice::Payload_Header*>(current_payload_);
  lbne::PennMicroSlice::Payload_Header::data_packet_type_t type = payload_header->data_packet_type;

  //Set the variables passed as references
  word_id = current_word_id_;
  type = payload_header->data_packet_type;
  data_packet_type = type;
  short_nova_timestamp = payload_header->short_nova_timestamp;

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
    case lbne::PennMicroSlice::DataTypeWarning:
      payload_size = lbne::PennMicroSlice::payload_size_selftest;
      break;
    case lbne::PennMicroSlice::DataTypeChecksum:
      payload_size = lbne::PennMicroSlice::payload_size_checksum;
      break;
    default:
      std::cerr << "Unknown data packet type found 0x" << std::hex << (unsigned int)type << std::endl;
      payload_size = 0;
      break;
    }//switch(type)

  current_word_id_++;
  //current_payload_ points at the Payload_Header of the next payload
  //this function assumes that state is true the next time it is called (so it knows how to shift to the next payload)
  //but the user is expecting just the payload data. Therefore return the buffer offset by the payload_header

  return (current_payload_ + lbne::PennMicroSlice::Payload_Header::size_words);
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
    lbne::PennMicroSlice::Payload_Header* payload_header = reinterpret_cast_checked<lbne::PennMicroSlice::Payload_Header*>(pl_ptr);
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
	case lbne::PennMicroSlice::DataTypeWarning:
	  payload_size = lbne::PennMicroSlice::payload_size_selftest;
	  break;
	case lbne::PennMicroSlice::DataTypeChecksum:
	  payload_size = lbne::PennMicroSlice::payload_size_checksum;
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
      case lbne::PennMicroSlice::DataTypeWarning:
	pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_selftest;
	break;
      case lbne::PennMicroSlice::DataTypeChecksum:
	pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_checksum;
	break;
      default:
	std::cerr << "Unknown data packet type found 0x" << std::hex << (unsigned int)type << std::endl;
	return nullptr;
	break;
      }//switch(type)
    i++;
  }
  std::cerr << "Could not find payload with ID " << word_id << " (the data buffer has overrun)" << std::endl;
  return nullptr;
}


// Returns the sample count in the microslice
lbne::PennMicroSlice::sample_count_t lbne::PennMicroSlice::sampleCount(
								       lbne::PennMicroSlice::sample_count_t &n_counter_words,
								       lbne::PennMicroSlice::sample_count_t &n_trigger_words,
								       lbne::PennMicroSlice::sample_count_t &n_timestamp_words,
								       lbne::PennMicroSlice::sample_count_t &n_selftest_words,
								       lbne::PennMicroSlice::sample_count_t &n_checksum_words,
								       bool swap_payload_header_bytes,
								       size_t override_uslice_size) const
{
  throw cet::exception("PennMicroSlice") << "As of Jul-28-2015, lbne::PennMicroSlice::sampleCount is deprecated";

  n_counter_words = n_trigger_words = n_timestamp_words = n_selftest_words = n_checksum_words = 0;

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
    lbne::PennMicroSlice::Payload_Header* payload_header = reinterpret_cast_checked<lbne::PennMicroSlice::Payload_Header*>(pl_ptr);
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
      case lbne::PennMicroSlice::DataTypeWarning:
	n_selftest_words++;
        pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_selftest;
        break;
      case lbne::PennMicroSlice::DataTypeChecksum:
	n_checksum_words++;
        pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_checksum;
        break;
      default:
	std::cerr << "Unknown data packet type found 0x" << std::hex << (unsigned int)type << std::endl;
	return 0;
	break;
      }//switch(type)
  }
  return n_counter_words + n_trigger_words + n_timestamp_words + n_selftest_words + n_checksum_words;
}

//Returns a point to the first payload header AFTER boundary_time
uint8_t* lbne::PennMicroSlice::sampleTimeSplit(uint64_t boundary_time, size_t& remaining_size,
					       bool swap_payload_header_bytes, size_t override_uslice_size) const
{

  throw cet::exception("PennMicroSlice") << "As of Jul-28-2015, lbne::PennMicroSlice::sampleTimeSplit is deprecated";

  //need to mask to get the lowest 28 bits of the nova timestamp
  //in order to compare with the 'short_nova_timestamp' in the Payload_Header
  boundary_time = boundary_time & 0xFFFFFFF;

  //if we're overriding, we don't have a Header to offset by
  uint8_t* pl_ptr;
  size_t   pl_size;
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
    lbne::PennMicroSlice::Payload_Header* payload_header = reinterpret_cast_checked<lbne::PennMicroSlice::Payload_Header*>(pl_ptr);
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
      case lbne::PennMicroSlice::DataTypeWarning:
        pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_selftest;
        break;
      case lbne::PennMicroSlice::DataTypeChecksum:
        pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_checksum;
        break;
      default:
	std::cerr << "Unknown data packet type found 0x" << std::hex << (unsigned int)type << std::endl;
	return nullptr;
	break;
      }//switch(type)
  }
  return nullptr;
}

//Returns a pointer to the first payload header AFTER boundary_time, and also counts payload types before/after the boundary
uint8_t* lbne::PennMicroSlice::sampleTimeSplitAndCount(uint64_t boundary_time, size_t& remaining_size,
						       lbne::PennMicroSlice::sample_count_t &n_words_b,
						       lbne::PennMicroSlice::sample_count_t &n_counter_words_b,
						       lbne::PennMicroSlice::sample_count_t &n_trigger_words_b,
						       lbne::PennMicroSlice::sample_count_t &n_timestamp_words_b,
						       lbne::PennMicroSlice::sample_count_t &n_selftest_words_b,
						       lbne::PennMicroSlice::sample_count_t &n_checksum_words_b,
						       lbne::PennMicroSlice::sample_count_t &n_words_a,
						       lbne::PennMicroSlice::sample_count_t &n_counter_words_a,
						       lbne::PennMicroSlice::sample_count_t &n_trigger_words_a,
						       lbne::PennMicroSlice::sample_count_t &n_timestamp_words_a,
						       lbne::PennMicroSlice::sample_count_t &n_selftest_words_a,
						       lbne::PennMicroSlice::sample_count_t &n_checksum_words_a,
						       bool swap_payload_header_bytes, size_t override_uslice_size) const
{
  throw cet::exception("PennMicroSlice") << "As of Jul-28-2015, lbne::PennMicroSlice::sampleTimeSplitAndCount is deprecated";

  n_words_b = n_counter_words_b = n_trigger_words_b = n_timestamp_words_b = n_selftest_words_b = n_checksum_words_b = 0;
  n_words_a = n_counter_words_a = n_trigger_words_a = n_timestamp_words_a = n_selftest_words_a = n_checksum_words_a = 0;
  remaining_size = 0;
  uint8_t* remaining_data_ptr = nullptr;
  bool is_before = true;

  //if we're overriding, we don't have a Header to offset by
  uint8_t* pl_ptr;
  size_t   pl_size;
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
    lbne::PennMicroSlice::Payload_Header* payload_header = reinterpret_cast_checked<lbne::PennMicroSlice::Payload_Header*>(pl_ptr);
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
      case lbne::PennMicroSlice::DataTypeWarning:
	if(is_before)
	  n_selftest_words_b++;
	else
	  n_selftest_words_a++;
	pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_selftest;
	break;
      case lbne::PennMicroSlice::DataTypeChecksum:
	if(is_before)
	  n_checksum_words_b++;
	else
	  n_checksum_words_a++;
	pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_checksum;
	break;
      default:
	std::cerr << "Unknown data packet type found 0x" << std::hex << (unsigned int)type 
		  << std::dec << std::endl;
	return nullptr;
	break;
      }//switch(type)
  }
  n_words_b = n_counter_words_b + n_trigger_words_b + n_timestamp_words_b + n_selftest_words_b + n_checksum_words_b;
  n_words_a = n_counter_words_a + n_trigger_words_a + n_timestamp_words_a + n_selftest_words_a + n_checksum_words_a;
#ifdef __DEBUG_sampleTimeSplitAndCount__
  std::cout << "PennMicroSlice::sampleTimeSplitAndCount DEBUG returning with remaining size " << remaining_size << " for boundary_time " << boundary_time
	    << std::endl
	    << "PennMicroSlice::sampleTimeSplitAndCount DEBUG returning with: "
	    << " Payloads before " << n_words_b << " = " << n_counter_words_b << " + " << n_trigger_words_b
	    << " + " << n_timestamp_words_b << " + " << n_selftest_words_b << " + " << n_checksum_words_b
	    << " Payloads after  " << n_words_a << " = " << n_counter_words_a << " + " << n_trigger_words_a
	    << " + " << n_timestamp_words_a << " + " << n_selftest_words_a << " + " << n_checksum_words_a
	    << std::endl;
#endif
  return remaining_data_ptr;
}

//Returns a pointer to the first payload header AFTER boundary_time, and also counts payload types before/after the boundary
//Also checks the overlap_time, and does the same for that

// JCF, Jul-28-2015

// Sections in the code where, e.g., there's a timestamp rollover will now result in an exception throw

uint8_t* lbne::PennMicroSlice::sampleTimeSplitAndCountTwice(uint64_t boundary_time, size_t& remaining_size,
							    uint64_t overlap_time,  size_t& overlap_size, uint8_t*& overlap_data_ptr,
							    lbne::PennMicroSlice::sample_count_t &n_words_b,
							    lbne::PennMicroSlice::sample_count_t &n_counter_words_b,
							    lbne::PennMicroSlice::sample_count_t &n_trigger_words_b,
							    lbne::PennMicroSlice::sample_count_t &n_timestamp_words_b,
							    lbne::PennMicroSlice::sample_count_t &n_selftest_words_b,
							    lbne::PennMicroSlice::sample_count_t &n_checksum_words_b,
							    lbne::PennMicroSlice::sample_count_t &n_words_a,
							    lbne::PennMicroSlice::sample_count_t &n_counter_words_a,
							    lbne::PennMicroSlice::sample_count_t &n_trigger_words_a,
							    lbne::PennMicroSlice::sample_count_t &n_timestamp_words_a,
							    lbne::PennMicroSlice::sample_count_t &n_selftest_words_a,
							    lbne::PennMicroSlice::sample_count_t &n_checksum_words_a,
							    lbne::PennMicroSlice::sample_count_t &n_words_o,
							    lbne::PennMicroSlice::sample_count_t &n_counter_words_o,
							    lbne::PennMicroSlice::sample_count_t &n_trigger_words_o,
							    lbne::PennMicroSlice::sample_count_t &n_timestamp_words_o,
							    lbne::PennMicroSlice::sample_count_t &n_selftest_words_o,
							    lbne::PennMicroSlice::sample_count_t &n_checksum_words_o,
							    uint32_t &checksum,
							    bool swap_payload_header_bytes, size_t override_uslice_size) const
{
  n_words_b = n_counter_words_b = n_trigger_words_b = n_timestamp_words_b = n_selftest_words_b = n_checksum_words_b = 0;
  n_words_a = n_counter_words_a = n_trigger_words_a = n_timestamp_words_a = n_selftest_words_a = n_checksum_words_a = 0;
  //n_words_o = n_counter_words_o = n_trigger_words_o = n_timestamp_words_o = n_selftest_words_o = n_checksum_words_o = 0; //don't reset these, as there is likely multiple uslices contained in the overlap

  overlap_size = remaining_size = 0;
  checksum = 0;
  uint8_t* remaining_data_ptr = nullptr;
  overlap_data_ptr = nullptr;
  bool is_before_boundary = true, is_before_overlap = true, is_in_overlap = false;

  //if we're overriding, we don't have a Header to offset by
  uint8_t* pl_ptr;
  size_t   pl_size;
  if(override_uslice_size) {
    pl_ptr  = buffer_;
    pl_size = override_uslice_size - sizeof(Header);
  }
  else {
    pl_ptr  = buffer_ + sizeof(Header);
    pl_size = size();
  }
  
  //  display_bits(pl_ptr, pl_size, "PennMicroSlice");

  //loop over the microslice
  while(pl_ptr < (buffer_ + pl_size)) {
#ifdef __DEBUG_sampleTimeSplitAndCountTwice__
    mf::LogInfo("PennMicroSlice") << "PennMicroSlice::sampleTimeSplitAndCountTwice DEBUG pointers." 
	      << " Payload "    << (unsigned int*)pl_ptr
	      << "\tOverlap "   << (unsigned int*)overlap_data_ptr
	      << "\tRemaining " << (unsigned int*)remaining_data_ptr
	      << std::endl;
#endif

    // JCF, Jul-29-2015: is this "if" block necessary any longer?
    if(swap_payload_header_bytes) {
      *((uint32_t*)pl_ptr) = ntohl(*((uint32_t*)pl_ptr));
    }

    lbne::PennMicroSlice::Payload_Header* payload_header = reinterpret_cast_checked<lbne::PennMicroSlice::Payload_Header*>(pl_ptr);
    lbne::PennMicroSlice::Payload_Header::data_packet_type_t     type      = payload_header->data_packet_type;
    lbne::PennMicroSlice::Payload_Header::short_nova_timestamp_t timestamp = payload_header->short_nova_timestamp;

#ifdef __DEBUG_sampleTimeSplitAndCountTwice__
    mf::LogInfo("PennMicroSlice") << "PennMicroSlice::sampleTimeSplitAndCountTwice DEBUG type " << std::bitset<3>(type) << " timestamp " << static_cast<uint32_t>(timestamp) << " ["<< std::hex << timestamp << std::dec << "]" << std::endl;
    switch (type) {
    case 0x1: // counter word
      mf::LogInfo("PennMicroSlice") << "Contents : [" << std::bitset<lbne::PennMicroSlice::payload_
    	break;
    case 0x2: // trigger word
      mf::LogInfo("PennMicroSlice") << "Contents : [" << std::bitset<lbne::PennMicroSlice::payload_
								     break;
    case 0x7: // timestamp word
      mf::LogInfo("PennMicroSlice") << "Contents : [" << std::bitset<lbne::PennMicroSlice::payload_
								     break;
    default:
      mf::LogInfo("PennMicroSlice") << "Unexpected size...taking as if it was a self test";
    }

    //    mf::LogInfo("PennMicroSlice") << "PennMicroSlice::sampleTimeSplitAndCountTwice DEBUG: boundary_time == " << 
    //      boundary_time << ", overlap_time == " << overlap_time << ", is_before_boundary == " << is_before_boundary << ", overlap_size ==" << overlap_size << 
    //      ", is_before_overlap == " << is_before_overlap << ", is_in_overlap == " << is_in_overlap;

#endif
    //check the timestamp
    if(is_before_boundary && (timestamp > boundary_time)) {
      //need to be careful and make sure that boundary_time hasn't overflowed the 28 bits
      //do this by checking whether timestamp isn't very large AND boundary_time isn't very small
      //TODO a better way to catch rollovers?
      if(!((boundary_time < lbne::PennMicroSlice::ROLLOVER_LOW_VALUE) && (timestamp > lbne::PennMicroSlice::ROLLOVER_HIGH_VALUE))) {
	remaining_size     = (buffer_ + pl_size) - pl_ptr;
	remaining_data_ptr = pl_ptr;
	is_before_boundary = false;
	is_before_overlap  = false;
	is_in_overlap      = false;

	if(overlap_size) {
	  overlap_size -= remaining_size; //take off the bytes after the overlap started, and after the end of the current millislice
	}
      } else {
	throw cet::exception("PennMicroSlice") << "Error: one or both of the following is true: boundary time of " << 
	  boundary_time << " is greater than ROLLOVER_LOW_VALUE time of " << lbne::PennMicroSlice::ROLLOVER_LOW_VALUE << 
	  ", timestamp value of " << timestamp << " is less than ROLLOVER_HIGH_VALUE time of " << 
	  lbne::PennMicroSlice::ROLLOVER_HIGH_VALUE; 
      }
    }
    //'else' is to make sure we don't have data in both the 'overlap' and 'remaining' buffers
    else if(is_before_overlap && (timestamp > overlap_time)) {

      //need to be careful and make sure that boundary_time hasn't overflowed the 28 bits
      //do this by checking whether timestamp isn't very large AND boundary_time isn't very small
      //TODO a better way to catch rollovers?
      if(!((overlap_time < lbne::PennMicroSlice::ROLLOVER_LOW_VALUE) && (timestamp > lbne::PennMicroSlice::ROLLOVER_HIGH_VALUE))) {
	overlap_size      = (buffer_ + pl_size) - pl_ptr;
	overlap_data_ptr  = pl_ptr;
	is_in_overlap     = true;
	is_before_overlap = false;
      } else {
	throw cet::exception("PennMicroSlice") << "Error: one or both of the following is true: overlap time of " << 
	  overlap_time << " is greater than ROLLOVER_LOW_VALUE time of " << lbne::PennMicroSlice::ROLLOVER_LOW_VALUE << 
	  ", timestamp value of " << timestamp << " is less than ROLLOVER_HIGH_VALUE time of " << 
	  lbne::PennMicroSlice::ROLLOVER_HIGH_VALUE; 
      }
    }

    //check the type to increment counters & the ptr
    switch(type)
      {
      case lbne::PennMicroSlice::DataTypeCounter:
	if(is_before_boundary)
	  n_counter_words_b++;
	else
	  n_counter_words_a++;
	if(is_in_overlap)
	  n_counter_words_o++;
	pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_counter;
	break;
      case lbne::PennMicroSlice::DataTypeTrigger:
	if(is_before_boundary)
	  n_trigger_words_b++;
	else
	  n_trigger_words_a++;
	if(is_in_overlap)
	  n_trigger_words_o++;
	pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_trigger;
	break;
      case lbne::PennMicroSlice::DataTypeTimestamp:
	if(is_before_boundary)
	  n_timestamp_words_b++;
	else
	  n_timestamp_words_a++;
	if(is_in_overlap)
	  n_timestamp_words_o++;
	pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_timestamp;
	//	mf::LogInfo("PennMicroSlice") << "JCF, sampleTimeSplitAndCountTwice: now have n_timestamp_words_b == " <<
	//	  n_timestamp_words_b << ", n_timestamp_words_a == " << n_timestamp_words_a <<
	//	  ", n_timestamp_words_o == " << n_timestamp_words_o;
	break;
      case lbne::PennMicroSlice::DataTypeWarning:
        if(is_before_boundary)
          n_selftest_words_b++;
        else
          n_selftest_words_a++;
        if(is_in_overlap)
          n_selftest_words_o++;
        pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_selftest;
        break;
      case lbne::PennMicroSlice::DataTypeChecksum:
        if(is_before_boundary)
          n_checksum_words_b++;
        else
          n_checksum_words_a++;
        if(is_in_overlap)
          n_checksum_words_o++;

	//	mf::LogInfo("PennMicroSlice") << "JCF, sampleTimeSplitAndCountTwice: now have n_checksum_words_b == " <<
	//	  n_checksum_words_b << ", n_checksum_words_a == " << n_checksum_words_a << 
	//	  ", n_checksum_words_o == " << n_checksum_words_o;

	checksum = *( reinterpret_cast<uint16_t*>(pl_ptr) );
	
        pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_checksum;
        break;
      default:
	std::cerr << "Unknown data packet type found 0x" << std::hex << (unsigned int)type 
		  << std::dec << std::endl;
	return nullptr;
	break;
      }//switch(type)
  }
  n_words_b = n_counter_words_b + n_trigger_words_b + n_timestamp_words_b + n_selftest_words_b + n_checksum_words_b;
  n_words_a = n_counter_words_a + n_trigger_words_a + n_timestamp_words_a + n_selftest_words_a + n_checksum_words_a;
  n_words_o = n_counter_words_o + n_trigger_words_o + n_timestamp_words_o + n_selftest_words_o + n_checksum_words_o;
#ifdef __DEBUG_sampleTimeSplitAndCountTwice__
  mf::LogInfo("PennMicroSlice") << "PennMicroSlice::sampleTimeSplitAndCountTwice DEBUG returning with:"
				<< " remaining size " << remaining_size << " for boundary_time " << boundary_time
				<< " overlap   size " << overlap_size   << " for overlap_time "  << overlap_time;

  mf::LogInfo("PennMicroSlice") << "PennMicroSlice::sampleTimeSplitAndCountTwice DEBUG returning with: "
				<< " Payloads before  " << n_words_b << " = " << n_counter_words_b << " + " << n_trigger_words_b 
				<< " + " << n_timestamp_words_b << " + " << n_selftest_words_b << " + " << n_checksum_words_b
				<< " Payloads after   " << n_words_a << " = " << n_counter_words_a << " + " << n_trigger_words_a
				<< " + " << n_timestamp_words_a << " + " << n_selftest_words_a << " + " << n_checksum_words_a
				<< " Overlap payloads " << n_words_o << " = " << n_counter_words_o << " + " << n_trigger_words_o
				<< " + " << n_timestamp_words_o << " + " << n_selftest_words_o << " + " << n_checksum_words_o ;
#endif
  return remaining_data_ptr;
}

// Returns a pointer to the raw data words in the microslice for diagnostics
uint32_t* lbne::PennMicroSlice::raw() const
{
	return reinterpret_cast_checked<uint32_t*>(buffer_);
}

// Returns a pointer to the microslice header
lbne::PennMicroSlice::Header const* lbne::PennMicroSlice::header_() const
{
  return reinterpret_cast_checked<Header const *>(buffer_);
}

// Returns a pointer to the first payload data word in the microslice
uint32_t const* lbne::PennMicroSlice::data_() const
{
  return reinterpret_cast_checked<uint32_t const*>(buffer_ + sizeof(lbne::PennMicroSlice::Header));
}
