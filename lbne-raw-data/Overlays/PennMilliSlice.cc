#include "lbne-raw-data/Overlays/PennMilliSlice.hh"

lbne::PennMilliSlice::PennMilliSlice(uint8_t* address) : buffer_(address)
{
}

lbne::PennMilliSlice::Header::millislice_size_t lbne::PennMilliSlice::size() const
{
  return header_()->millislice_size;
}

lbne::PennMilliSlice::Header::microslice_count_t lbne::PennMilliSlice::microSliceCount() const
{
  return header_()->microslice_count;
}

// Returns the requested MicroSlice if the requested slice was found,
// otherwise returns an empty pointer
std::unique_ptr<lbne::PennMicroSlice> lbne::PennMilliSlice::microSlice(uint32_t index) const
{
  std::unique_ptr<PennMicroSlice> mslice_ptr;
  if (index < microSliceCount()) {
    uint8_t* ms_ptr = data_(index);
    mslice_ptr.reset(new PennMicroSlice(ms_ptr));
  }
  return mslice_ptr;
}

lbne::PennMilliSlice::Header const* lbne::PennMilliSlice::header_() const
{
  return reinterpret_cast<Header const*>(buffer_);
}

// returns a pointer to the requested MicroSlice
uint8_t* lbne::PennMilliSlice::data_(int index) const
{
  uint8_t* ms_ptr = buffer_ + sizeof(Header);
  for (int idx = 0; idx < index; ++idx) {
    PennMicroSlice tmp_ms(ms_ptr);
    ms_ptr += tmp_ms.size();
  }
  return ms_ptr;
}
