#include <stdint.h>
#include <bitset>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <vector>
#include "artdaq-core/Data/Fragment.hh"
#include "dune-raw-data/Overlays/FelixFragment.hh"
#include "dune-raw-data/Overlays/FelixFragmentReordered.hh"
#include "dune-raw-data/Overlays/FelixReorder.hh"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"

#define BOOST_TEST_MODULE(MilliSlice_t)
#ifdef HAVE_CANVAS
#include "cetlib/quiet_unit_test.hpp"
#else
#include "boost/test/auto_unit_test.hpp"
#endif

BOOST_AUTO_TEST_SUITE(FelixFragment_test)

BOOST_AUTO_TEST_CASE(BaselineTest) {
  std::cout << "### WOOF -> Testing fromFile creation...\n";
  artdaq::FragmentPtr frag = dune::FelixFragment::fromFile(
      "/home/felixdev/milo/uBframes/Plane_0-Link_1.dat");
  dune::FelixFragment flxfrg(*frag);
  std::cout << "\n\n";

  std::cout << "### WOOF -> Test for the presence of a single frame...\n";
  const size_t words = 9600*117;
  const size_t frames = 9600;
  const size_t adc_values = 9600*256;

  std::cout << "  -> Total words: " << flxfrg.total_words() << '\n';
  std::cout << "  -> Total frames: " << flxfrg.total_frames() << '\n';
  std::cout << "  -> Total adc values: " << flxfrg.total_adc_values() << '\n';

  BOOST_REQUIRE_EQUAL(flxfrg.total_words(), words);
  BOOST_REQUIRE_EQUAL(flxfrg.total_frames(), frames);
  BOOST_REQUIRE_EQUAL(flxfrg.total_adc_values(), adc_values);
  std::cout << "\n\n";

  std::cout << "### WOOF -> WIB frame test...\n";
  const size_t frame_size = 468;
  BOOST_REQUIRE_EQUAL(sizeof(dune::FelixFragment::WIBFrame), frame_size);
  std::cout << " -> SOF: " << unsigned(flxfrg.sof(0)) << "\n";
  std::cout << " -> Version: " << unsigned(flxfrg.version(0)) << "\n";
  std::cout << " -> FiberNo: " << unsigned(flxfrg.fiber_no(0)) << "\n";
  std::cout << " -> SlotNo: " << unsigned(flxfrg.slot_no(0)) << "\n";
  std::cout << " -> CrateNo: " << unsigned(flxfrg.crate_no(0)) << "\n";
  std::cout << " -> Timestamp: " << std::hex << flxfrg.timestamp(0) << std::dec
            << "\n";
  std::cout << "\n\n";

  // std::cout << "### WOOF -> Dumping frames...\n";
  // flxfrg.print_frames();
  // std::cout << "\n\n";

  // Test whether FelixFragment and FelixFragmentReordered correspond.
  std::cout << "### MEOW -> Reordered FELIX Fragment test.\n";
  std::ifstream ifile("/home/felixdev/milo/uBframes/Plane_0-Link_1.dat",
                      std::ios_base::ate);
  const size_t ifile_size = ifile.tellg();
  ifile.seekg(0, std::ios_base::beg);
  uint8_t* buffer = new uint8_t[ifile_size];
  ifile.read(reinterpret_cast<char*>(buffer), ifile_size);

  artdaq::Fragment reordfrg(dune::FelixReorder(buffer, ifile_size / 468));
  dune::FelixFragmentReordered reordflxfrg(reordfrg);

  std::cout << "  -> Total words: " << reordflxfrg.total_words() << '\n';
  std::cout << "  -> Total frames: " << reordflxfrg.total_frames() << '\n';
  std::cout << "  -> Total adc values: " << reordflxfrg.total_adc_values() << '\n';

  std::cout << "### MEOW -> Comparing " << ifile_size / 468 << " frames.\n";
  for (unsigned i = 0; i < ifile_size / 468; ++i) {
    BOOST_REQUIRE_EQUAL(flxfrg.sof(i), reordflxfrg.sof(i));
    BOOST_REQUIRE_EQUAL(flxfrg.version(i), reordflxfrg.version(i));
    BOOST_REQUIRE_EQUAL(flxfrg.fiber_no(i), reordflxfrg.fiber_no(i));
    BOOST_REQUIRE_EQUAL(flxfrg.slot_no(i), reordflxfrg.slot_no(i));
    BOOST_REQUIRE_EQUAL(flxfrg.crate_no(i), reordflxfrg.crate_no(i));
    BOOST_REQUIRE_EQUAL(flxfrg.timestamp(i), reordflxfrg.timestamp(i));
    BOOST_REQUIRE_EQUAL(flxfrg.CRC32(i), reordflxfrg.CRC32(i));
    for (unsigned j = 0; j < 4; ++j) {
      BOOST_REQUIRE_EQUAL(flxfrg.s1_error(i, j), reordflxfrg.s1_error(i, j));
      BOOST_REQUIRE_EQUAL(flxfrg.s2_error(i, j), reordflxfrg.s2_error(i, j));
      BOOST_REQUIRE_EQUAL(flxfrg.checksum_a(i, j),
                          reordflxfrg.checksum_a(i, j));
      BOOST_REQUIRE_EQUAL(flxfrg.checksum_b(i, j),
                          reordflxfrg.checksum_b(i, j));
      BOOST_REQUIRE_EQUAL(flxfrg.coldata_convert_count(i, j),
                          reordflxfrg.coldata_convert_count(i, j));
      BOOST_REQUIRE_EQUAL(flxfrg.error_register(i, j),
                          reordflxfrg.error_register(i, j));
      for (unsigned h = 0; h < 8; ++h) {
        BOOST_REQUIRE_EQUAL(flxfrg.HDR(i, j, h), reordflxfrg.HDR(i, j, h));
      }
    }
    for (unsigned ch = 0; ch < 256; ++ch) {
      BOOST_REQUIRE_EQUAL(flxfrg.get_ADC(i, ch), reordflxfrg.get_ADC(i, ch));
    }
  }
  std::cout << "### MEOW -> Tests successful.\n";
  delete[] buffer;

  std::cout << "### WOOF WOOF -> Done...\n";
}

#if 0
BOOST_AUTO_TEST_CASE(TinyBufferTest)
{

  // *** Test a buffer that is too small for even the header

}

BOOST_AUTO_TEST_CASE(SmallBufferTest)
{

  // *** Test a buffer that is too small for two microslices

}

BOOST_AUTO_TEST_CASE(CopyTest)
{
}

BOOST_AUTO_TEST_CASE(BufferReuseTest)
{
}
#endif

BOOST_AUTO_TEST_SUITE_END()

#pragma GCC diagnostic pop
