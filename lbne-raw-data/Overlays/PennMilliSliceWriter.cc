#include "lbne-raw-data/Overlays/PennMilliSliceWriter.hh"

lbne::PennMilliSliceWriter::
PennMilliSliceWriter(uint8_t* address, uint32_t max_size_bytes) :
    PennMilliSlice(address), max_size_bytes_(max_size_bytes)
{
  header_()->version = 1;
  header_()->millislice_size = sizeof(Header);
  header_()->microslice_count = 0;
  header_()->payload_count           = 0;
  header_()->payload_count_counter   = 0;
  header_()->payload_count_trigger   = 0;
  header_()->payload_count_timestamp = 0;
}

std::shared_ptr<lbne::PennMicroSliceWriter>
lbne::PennMilliSliceWriter::reserveMicroSlice(uint32_t ms_max_bytes)
{
  // finalize the most recent PennMicroSlice, in case that hasn't
  // already been done
  finalizeLatestMicroSlice_();

  // test if this new PennMicroSlice could overflow our maximum size
  if ((size() + ms_max_bytes) > max_size_bytes_) {
    return false;
  }

  // create the next PennMicroSlice in our buffer, and update our
  // counters to include the new PennMicroSlice
  uint8_t* ms_ptr = data_(header_()->microslice_count);
  latest_microslice_ptr_.reset(new PennMicroSliceWriter(ms_ptr, ms_max_bytes));
  ++(header_()->microslice_count);
  header_()->millislice_size += ms_max_bytes;
  return latest_microslice_ptr_;
}

int32_t lbne::PennMilliSliceWriter::finalize(bool override, uint32_t data_size_bytes, uint32_t microslice_count,
					     uint16_t payload_count, uint16_t payload_count_counter,
					     uint16_t payload_count_trigger, uint16_t payload_count_timestamp)
{
  // first, we need to finalize the last MicroSlice, in case that
  // hasn't already been done
  finalizeLatestMicroSlice_();

  //Override the current size if requested
  if(override) {
    header_()->millislice_size = sizeof(Header) + data_size_bytes;
    header_()->microslice_count = microslice_count;
    header_()->payload_count = payload_count;
    header_()->payload_count_counter = payload_count_counter;
    header_()->payload_count_trigger = payload_count_trigger;
    header_()->payload_count_timestamp = payload_count_timestamp;
  }

  // next, we update our maximum size so that no more MicroSlices
  // can be added
  int32_t size_diff = max_size_bytes_ - header_()->millislice_size;
  max_size_bytes_ = header_()->millislice_size;
  return size_diff;
}

void lbne::PennMilliSliceWriter::finalizeLatestMicroSlice_()
{
  if (header_()->microslice_count > 0 && latest_microslice_ptr_.get() != 0) {
    int size_change = latest_microslice_ptr_->finalize();
    header_()->millislice_size -= size_change;
  }
}

lbne::PennMilliSliceWriter::Header* lbne::PennMilliSliceWriter::header_()
{
  return reinterpret_cast<Header *>(buffer_);
}

uint8_t* lbne::PennMilliSliceWriter::data_(int index)
{
  uint8_t* ms_ptr = buffer_ + sizeof(Header);
  for (int idx = 0; idx < index; ++idx) {
    PennMicroSlice tmp_ms(ms_ptr);
    ms_ptr += tmp_ms.size();
  }
  return ms_ptr;
}
