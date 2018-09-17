// FelixDecode.hh serves to decode root files containing artdaq Fragments with
// FELIX data.

#ifndef artdaq_dune_Overlays_FelixDecode_hh
#define artdaq_dune_Overlays_FelixDecode_hh

#include <math.h>
#include <stdint.h>
#include <complex>
#include <iomanip>
#include <iostream>
#include <string>
#include <valarray>
#include <vector>
#include <fstream>
#include "art/Framework/Principal/Handle.h"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq-core/Data/ContainerFragment.hh"
#include "canvas/Utilities/InputTag.h"
#include "dune-raw-data/Overlays/FelixFragment.hh"
#include "dune-raw-data/Overlays/FragmentType.hh"
#include "gallery/Event.h"

namespace dune {

class FelixDecoder {
 private:
  art::InputTag tag = "daq:ContainerFELIX:DAQ";
  std::vector<std::string> filenames;
  std::vector<artdaq::Fragment> frags_;

  std::vector<unsigned> Uch = {
      0,  1,  2,   3,   4,   16,  17,  18,  19,  20,  43,  44, 45, 46,
      47, 59, 60,  61,  62,  63,  64,  65,  66,  67,  68,  80, 81, 82,
      83, 84, 107, 108, 109, 110, 111, 123, 124, 125, 126, 127};
  std::vector<unsigned> Vch = {
      5,  6,  7,   8,   9,   21,  22,  23,  24,  25,  38,  39, 40, 41,
      42, 54, 55,  56,  57,  58,  69,  70,  71,  72,  73,  85, 86, 87,
      88, 89, 102, 103, 104, 105, 106, 118, 119, 120, 121, 122};
  std::vector<unsigned> Wch = {
      10, 11, 12, 13, 14, 15, 26, 27, 28,  29,  30,  31,  32,  33,  34,  35,
      36, 37, 48, 49, 50, 51, 52, 53, 74,  75,  76,  77,  78,  79,  90,  91,
      92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 112, 113, 114, 115, 116, 117};

 public:
  // General information accessors.
  size_t total_fragments() const { return frags_.size(); }

  size_t total_frames(const size_t i = 0) const {
    dune::FelixFragment flxfrag(frags_[i]);
    return flxfrag.total_frames();
  }

  size_t total_events() const {
    gallery::Event evt(filenames);
    return evt.numberOfEventsInFile();
  }

  // Check functions.
  bool check_timestamps(unsigned frag_num) const {
    bool failed = false;

    dune::FelixFragment flxfrag(frags_[frag_num]);
    // Loop over frames in the fragment and check the timestamps.
    uint64_t prevtimestamp = flxfrag.timestamp(0);
    for (unsigned fr = 1; fr < flxfrag.total_frames(); ++fr) {
      // Check timestamp.
      if (flxfrag.timestamp(fr) - prevtimestamp != 25) {
        std::cout << "Timestamp does not increase by 25 at frame " << fr
                  << " in fragment " << frag_num << "!\n";
        std::cout << "It instead increases by "
                  << flxfrag.timestamp(fr) - prevtimestamp << "\n";
        std::cout << "Fiber " << (unsigned)flxfrag.fiber_no(fr) << ", slot "
                  << (unsigned)flxfrag.slot_no(fr) << ", crate "
                  << (unsigned)flxfrag.crate_no(fr) << '\n';
        failed = true;
        break;
      }
      // std::cout << flxfrag.timestamp(fr) - prevtimestamp << "\t";
      prevtimestamp = flxfrag.timestamp(fr);
    }  // Loop over frames.

    return !failed;
  }

  bool check_all_timestamps() const {
    bool failed = false;
    // Loop over fragments within an event.
    std::cout << "Going through " << frags_.size() << " fragments.\n";
    for (unsigned i = 0; i < frags_.size(); ++i) {
      std::cout << i << '\r';
      failed |= !(check_timestamps(i));
    }

    if (failed) {
      std::cout << "Timestamp check failed.\n";
    } else {
      std::cout << "Timestamp check succeeded.\n";
    }

    return !failed;
  }

  bool check_CCCs(unsigned frag_num) const {
    bool failed = false;

    dune::FelixFragment flxfrag(frags_[frag_num]);
    // Loop over frames in the fragment and check the COLDATA convert
    // counts.
    uint16_t prevCCC1 = flxfrag.coldata_convert_count(0, 0);
    uint16_t prevCCC2 = flxfrag.coldata_convert_count(0, 2);
    for (unsigned fr = 1; fr < flxfrag.total_frames(); ++fr) {
      // Check CCC1.
      if (flxfrag.coldata_convert_count(fr, 0) - prevCCC1 != 1 &&
          flxfrag.coldata_convert_count(fr, 0) - prevCCC1 != -33919 &&
          flxfrag.coldata_convert_count(fr, 0) - prevCCC1 != -65535) {
        std::cout << "CCC1 does not increase by 1 at frame " << fr
                  << " in fragment " << frag_num << "!\n";
        std::cout << "It instead increases by "
                  << flxfrag.coldata_convert_count(fr, 0) - prevCCC1 << '\n';
        failed = true;
        break;

        // Check CCC2.
        if (flxfrag.coldata_convert_count(fr, 2) - prevCCC2 != 1 &&
            flxfrag.coldata_convert_count(fr, 2) - prevCCC2 != -33919 &&
            flxfrag.coldata_convert_count(fr, 0) - prevCCC2 != -65535) {
          std::cout << "CCC2 does not increase by 1 at frame " << fr
                    << " in fragment " << frag_num << "!\n";
          std::cout << "It instead increases by "
                    << flxfrag.coldata_convert_count(fr, 2) - prevCCC2 << '\n';
          failed = true;
          break;
        }
        prevCCC2 = flxfrag.coldata_convert_count(fr, 2);
      }
      prevCCC1 = flxfrag.coldata_convert_count(fr, 0);

      // Check whether other CCCs correspond.
      if (flxfrag.coldata_convert_count(fr, 0) !=
              flxfrag.coldata_convert_count(fr, 1) ||
          flxfrag.coldata_convert_count(fr, 2) !=
              flxfrag.coldata_convert_count(fr, 3)) {
        std::cout << "CCC in frame " << fr << " do not correspond!\n";
        failed = true;
        break;
      }
    }  // Loop over frames.

    return !failed;
  }

  bool check_all_CCCs() const {
    bool failed = false;
    for (unsigned i = 0; i < frags_.size(); ++i) {
      failed |= !(check_CCCs(i));
    }

    if (failed) {
      std::cout << "CCC check failed.\n";
    } else {
      std::cout << "CCC check succeeded.\n";
    }

    return !failed;
  }

  bool check_IDs(unsigned frag_num) const {
    bool failed = false;

    dune::FelixFragment flxfrag(frags_[frag_num]);
    // Output a fragment's fiber, crate and slot number.
    unsigned fiber_no = flxfrag.fiber_no();
    unsigned crate_no = flxfrag.crate_no();
    unsigned slot_no = flxfrag.slot_no();
    // std::cout << "Fragment " << i << " has:\nfiber_no: " << fiber_no <<
    // '\n'; std::cout << "crate_no: " << crate_no << '\n'; std::cout <<
    // "slot_no: " << slot_no << '\n';

    // Check whether fragments have logical
    // fiber, crate and slot numbers between frames.
    for (unsigned fi = 1; fi < flxfrag.total_frames(); ++fi) {
      // The fiber_no has to stay constant
      if (flxfrag.fiber_no(fi) != fiber_no) {
        std::cout << "Non-constant fiber_no in fragment " << frag_num
                  << " frame " << fi << ": " << (unsigned)(flxfrag.fiber_no(fi))
                  << " instead of " << fiber_no << '\n';
        fiber_no = flxfrag.fiber_no(fi);
        failed = true;
        break;
      }
      // The crate_no has to stay constant.
      if (flxfrag.crate_no(fi) != crate_no) {
        std::cout << "Non-constant crate_no in fragment " << frag_num
                  << " frame " << fi << ": " << (unsigned)(flxfrag.crate_no(fi))
                  << " instead of " << crate_no << '\n';
        crate_no = flxfrag.crate_no(fi);
        failed = true;
        break;
      }
      // The slot_no has to stay constant.
      if (flxfrag.slot_no(fi) != slot_no) {
        std::cout << "Non-constant slot_no in fragment " << frag_num
                  << " frame " << fi << ": " << (unsigned)(flxfrag.slot_no(fi))
                  << " instead of " << slot_no << '\n';
        slot_no = flxfrag.slot_no(fi);
        failed = true;
        break;
      }
    }  // Loop over frames.

    return !failed;
  }

  bool check_all_IDs() const {
    bool failed = false;
    for (unsigned i = 0; i < frags_.size(); ++i) {
      failed |= !(check_IDs(i));
    }

    if (failed) {
      std::cout << "ID check failed.\n";
    } else {
      std::cout << "ID check succeeded.\n";
    }

    return !failed;
  }

  // Accessor for artdaq::Fragments.
  artdaq::Fragment Fragment(const size_t& frag_num) const {
    if (frag_num > total_fragments()) {
      std::cout << "Fragment index out of range.\n";
      return frags_[0];
    }
    return frags_[frag_num];
  }

  // Cooley-Tukey FFT (in-place, breadth-first, decimation-in-frequency)
  // Copied from rosettacode.org and modified here. Perform FFT over the first
  // fragment and print to file for plotting.
  void FFT(std::vector<std::complex<double>>& x) {
    typedef std::complex<double> Complex;

    // Determine the length of the vector to be processed.
    unsigned N = 1;
    for(unsigned i = 0; i < 16; ++i) {
      if(N<<1 < x.size()) {
        N <<= 1;
      }
    }
    // Resize original vector.
    x.resize(N);

    // DFT
    unsigned int k = N, n;
    double thetaT = 3.14159265358979323846264338328L / N;
    Complex phiT = Complex(cos(thetaT), -sin(thetaT)), T;
    while (k > 1) {
      n = k;
      k /= 2;
      phiT = phiT * phiT;
      T = 1.0L;
      for (unsigned int l = 0; l < k; l++) {
        for (unsigned int a = l; a < N; a += n) {
          unsigned int b = a + k;
          Complex t = x[a] - x[b];
          x[a] += x[b];
          x[b] = t * T;
        }
        T *= phiT;
      }
    }
    // Decimate
    unsigned int m = (unsigned int)log2(N);
    for (unsigned int a = 0; a < N; a++) {
      unsigned int b = a;
      // Reverse bits
      b = (((b & 0xaaaaaaaa) >> 1) | ((b & 0x55555555) << 1));
      b = (((b & 0xcccccccc) >> 2) | ((b & 0x33333333) << 2));
      b = (((b & 0xf0f0f0f0) >> 4) | ((b & 0x0f0f0f0f) << 4));
      b = (((b & 0xff00ff00) >> 8) | ((b & 0x00ff00ff) << 8));
      b = ((b >> 16) | (b << 16)) >> (32 - m);
      if (b > a) {
        Complex t = x[a];
        x[a] = x[b];
        x[b] = t;
      }
    }
    // // Normalize
    // Complex f = 1.0 / sqrt(N);
    // for (unsigned int i = 0; i < N; i++) {
    //   x[i] *= f;
    // }
  }

  // Function to determine channel RMS and print to file.
  void calculateNoiseRMS(std::string destination) {
    // Number of fragments to run this code over.
    unsigned num_frags = total_fragments() / total_events();

    // Ugly way to get all the channels in the planes.
    unsigned Usize = Uch.size();
    unsigned Vsize = Vch.size();
    unsigned Wsize = Wch.size();
    for (unsigned i = 0; i < Usize; ++i) {
      Uch.push_back(Uch[i] + 128);
    }
    for (unsigned i = 0; i < Vsize; ++i) {
      Vch.push_back(Vch[i] + 128);
    }
    for (unsigned i = 0; i < Wsize; ++i) {
      Wch.push_back(Wch[i] + 128);
    }

    // Vectors to contain channel averages, RMSes and test results.
    std::vector<double> ch_avgsU(2 * 5 * 256, 0);
    std::vector<double> ch_avgsV(2 * 5 * 256, 0);
    std::vector<double> ch_avgsW(2 * 5 * 256, 0);
    std::vector<double> ch_rmsU(2 * 5 * 256, 0);
    std::vector<double> ch_rmsV(2 * 5 * 256, 0);
    std::vector<double> ch_rmsW(2 * 5 * 256, 0);
    std::vector<unsigned> ch_freqU(2 * 5 * 256, 0);
    std::vector<unsigned> ch_freqV(2 * 5 * 256, 0);
    std::vector<unsigned> ch_freqW(2 * 5 * 256, 0);
    // Vector to contain fragment test results.
    std::vector<bool> frag_good(num_frags, false);
    // Vector to contain FFT results per FEMB.
    std::vector<std::vector<std::complex<double>>> FEMB_FFT(
        20, std::vector<std::complex<double>>(total_frames(), 0));

    // Test fragments for data integrity.
    std::cout << "Testing fragments for integrity.\n";
    for (unsigned frag_num = 0; frag_num < num_frags; ++frag_num) {
      bool data_good = true;
      data_good |= check_timestamps(frag_num);  // Checks increments by 25.
      data_good |= check_CCCs(
          frag_num);  // Checks increments by 1 and correspondence between CCCs.
      data_good |= check_IDs(frag_num);  // Check the identifier fields of
                                         // all the frames in fragments.
      frag_good[frag_num] = data_good;
    }

    // Record the average of all channels of good fragments.
    std::cout << "Calculating channel averages.\n";
    for (unsigned frag_num = 0; frag_num < num_frags; ++frag_num) {
      if (!frag_good[frag_num]) {
        continue;
      }

      // Load the fragment and put it in the overlay.
      artdaq::Fragment frag = Fragment(frag_num);
      dune::FelixFragment flxfrag(frag);
      unsigned ch_num_base =
          flxfrag.slot_no() * 512 + (flxfrag.fiber_no() - 1) * 256;
      // Loop through frames and channels.
      for (unsigned fr_num = 0; fr_num < flxfrag.total_frames(); ++fr_num) {
        for (unsigned u = 0; u < Uch.size(); ++u) {
          ch_avgsU[ch_num_base + Uch[u]] += flxfrag.get_ADC(fr_num, Uch[u]);
          ch_freqU[ch_num_base + Uch[u]]++;
        }
        for (unsigned v = 0; v < Vch.size(); ++v) {
          ch_avgsV[ch_num_base + Vch[v]] += flxfrag.get_ADC(fr_num, Vch[v]);
          ch_freqV[ch_num_base + Vch[v]]++;
        }
        for (unsigned w = 0; w < Wch.size(); ++w) {
          ch_avgsW[ch_num_base + Wch[w]] += flxfrag.get_ADC(fr_num, Wch[w]);
          ch_freqW[ch_num_base + Wch[w]]++;
        }
      }
    }
    for (unsigned ch_num = 0; ch_num < ch_avgsU.size(); ++ch_num) {
      if (ch_freqU[ch_num] != 0) {
        ch_avgsU[ch_num] /= ch_freqU[ch_num];
      }
    }
    for (unsigned ch_num = 0; ch_num < ch_avgsV.size(); ++ch_num) {
      if (ch_freqV[ch_num] != 0) {
        ch_avgsV[ch_num] /= ch_freqV[ch_num];
      }
    }
    for (unsigned ch_num = 0; ch_num < ch_avgsW.size(); ++ch_num) {
      if (ch_freqW[ch_num] != 0) {
        ch_avgsW[ch_num] /= ch_freqW[ch_num];
      }
    }

    // Record the RMS of each channel.
    std::cout << "Calculating RMS values per channel.\n";
    for (unsigned frag_num = 0; frag_num < num_frags; ++frag_num) {
      if (!frag_good[frag_num]) {
        continue;
      }

      // Load the fragment and put it in the overlay.
      artdaq::Fragment frag = Fragment(frag_num);
      dune::FelixFragment flxfrag(frag);
      // std::cout << "Fragment " << frag_num << " has crate " << (unsigned)flxfrag.crate_no()
      //           << " slot " << (unsigned)flxfrag.slot_no() << " and fiber " << (unsigned)flxfrag.fiber_no() << '\n';
      unsigned ch_num_base =
          flxfrag.slot_no() * 512 + (flxfrag.fiber_no() - 1) * 256;
      // Loop through channels and frames.
      // Add to noise RMS vectors.
      for (unsigned u = 0; u < Uch.size(); ++u) {
        for(unsigned fr_num = 0; fr_num < flxfrag.total_frames(); ++fr_num) {
          ch_rmsU[ch_num_base + Uch[u]] += pow(
              ch_avgsU[ch_num_base + Uch[u]] - flxfrag.get_ADC(fr_num, Uch[u]),
              2);
        }
      }
      for (unsigned v = 0; v < Vch.size(); ++v) {
        for(unsigned fr_num = 0; fr_num < flxfrag.total_frames(); ++fr_num) {
          ch_rmsV[ch_num_base + Vch[v]] += pow(
              ch_avgsV[ch_num_base + Vch[v]] - flxfrag.get_ADC(fr_num, Vch[v]),
              2);
        }
      }
      for (unsigned w = 0; w < Wch.size(); ++w) {
        for(unsigned fr_num = 0; fr_num < flxfrag.total_frames(); ++fr_num) {
          ch_rmsW[ch_num_base + Wch[w]] += pow(
              ch_avgsW[ch_num_base + Wch[w]] - flxfrag.get_ADC(fr_num, Wch[w]),
              2);
        }
      }
      for (unsigned ch_num = 0; ch_num < 256; ++ch_num) {
        // FFT vector.
        std::vector<std::complex<double>> ch_fft_waveform(
            flxfrag.total_frames(), 0);
        for (unsigned fr_num = 0; fr_num < flxfrag.total_frames(); ++fr_num) {
          // Add to FFT vector.
          ch_fft_waveform[fr_num] = flxfrag.get_ADC(fr_num, ch_num);
        }
        FFT(ch_fft_waveform);
        // Add to master FFT waveform.
        unsigned FEMB_num = (flxfrag.slot_no() - 0) * 4 +
                            (flxfrag.fiber_no() - 1) * 2 + ch_num / 128;
        for (unsigned i = 0; i < ch_fft_waveform.size(); ++i) {
          FEMB_FFT[FEMB_num][i] += ch_fft_waveform[i];
        }
      }
    }
    // Write RMS to file.
    std::cout << "Writing to file " << destination + "/RMSU.dat"
              << ".\n";
    std::ofstream rmsfileU(destination + "/RMSU.dat");
    rmsfileU << "#WIB   Fibre    Channel   Global   Noise RMS U\n";
    for (unsigned ch_num = 0; ch_num < ch_rmsU.size(); ++ch_num) {
      if (ch_freqU[ch_num] == 0) {
        continue;
      }
      rmsfileU << ch_num / 512 + 1 << "   " << (ch_num % 512) / 256 + 1
               << "    " << ch_num % 256 << "    " << ch_num << "    "
               << sqrt(ch_rmsU[ch_num] / ch_freqU[ch_num]) << '\n';
    }
    rmsfileU.close();

    std::cout << "Writing to file " << destination + "/RMSV.dat"
              << ".\n";
    std::ofstream rmsfileV(destination + "/RMSV.dat");
    rmsfileV << "#WIB   Fibre    Channel   Global   Noise RMS V\n";
    for (unsigned ch_num = 0; ch_num < ch_rmsV.size(); ++ch_num) {
      if (ch_freqV[ch_num] == 0) {
        continue;
      }
      rmsfileV << ch_num / 512 + 1 << "   " << (ch_num % 512) / 256 + 1
               << "    " << ch_num % 256 << "    " << ch_num << "    "
               << sqrt(ch_rmsV[ch_num] / ch_freqV[ch_num]) << '\n';
    }
    rmsfileV.close();

    std::cout << "Writing to file " << destination + "/RMSW.dat"
              << ".\n";
    std::ofstream rmsfileW(destination + "/RMSW.dat");
    rmsfileW << "#WIB   Fibre    Channel   Global   Noise RMS W\n";
    for (unsigned ch_num = 0; ch_num < ch_rmsW.size(); ++ch_num) {
      if (ch_freqW[ch_num] == 0) {
        continue;
      }
      rmsfileW << ch_num / 512 + 1 << "   " << (ch_num % 512) / 256 + 1
               << "    " << ch_num % 256 << "    " << ch_num << "    "
               << sqrt(ch_rmsW[ch_num] / ch_freqW[ch_num]) << '\n';
    }
    rmsfileW.close();

    // Normalise and write FFTs to file.
    std::string fftfilename = destination + "/FFT.dat";
    std::cout << "Writing to file " << fftfilename << ".\n";
    std::ofstream fftfile(fftfilename);
    // Write headers.
    fftfile << "#Slot ->  ";
    for (unsigned f = 0; f < 20; ++f) {
      fftfile << std::setw(15) << f / 4 + 1;
    }
    fftfile << '\n' << "#Fiber -> ";
    for (unsigned f = 0; f < 20; ++f) {
      fftfile << std::setw(15) << (f % 4) / 2 + 1;
    }
    fftfile << '\n' << "#FEMB ->  ";
    for (unsigned f = 0; f < 20; ++f) {
      fftfile << std::setw(15) << f % 2 + 1;
    }
    fftfile << '\n' << "#Global ->";
    for (unsigned f = 0; f < 20; ++f) {
      fftfile << std::setw(15) << f + 1;
    }
    fftfile << '\n';

    // Write the first row a little wider.
    fftfile << std::left << std::setw(5) << 0 << std::right;
    for (unsigned femb = 0; femb < 20; ++femb) {
      fftfile << std::setw(15)
              << log10(pow(std::abs(FEMB_FFT[femb][0]) / num_frags, 2));
    }
    fftfile << '\n';

    for (unsigned tick = 1; tick < total_frames(); ++tick) {
      // std::cout << "Tick" << tick << " written.\n";
      fftfile << std::left << std::setw(10) << tick << std::right;
      // Write FFT for each FEMB.
      for (unsigned femb = 0; femb < 20; ++femb) {
        fftfile << std::setw(15)
                << log10(pow(std::abs(FEMB_FFT[femb][tick]) / num_frags, 2));
      }
      fftfile << '\n';
    }

    fftfile.close();
  }

  // Print function.
  void printFrames(const size_t& begin_frame, const size_t& end_frame,
                   const size_t& frag_num = 0) const {
    artdaq::Fragment frag(Fragment(frag_num));
    dune::FelixFragment flxfrag(frag);
    for (unsigned i = begin_frame; i < end_frame; ++i) {
      std::cout << "Frame " << i << '\n';
      flxfrag.print(i);
    }
  }

  // Constructors that load a file.
  FelixDecoder() = delete;
  FelixDecoder(const std::string& filename) {
    filenames.push_back(filename);
    for (gallery::Event evt(filenames); !evt.atEnd(); evt.next()) {
      gallery::ValidHandle<std::vector<artdaq::Fragment>> const& frags =
          evt.getValidHandle<std::vector<artdaq::Fragment>>(tag);
      for (const auto& frag : *frags) {
        artdaq::ContainerFragment cont_frag(frag);
        for(unsigned b = 0; b < cont_frag.block_count(); ++b) {
          frags_.push_back(*cont_frag[b]);
        }
      }
    }
    std::cout << "Loaded " << total_fragments() << " fragments in "
              << total_events() << " events.\n";
  }
};  // class FelixDecoder

}  // namespace dune

#endif /* artdaq_dune_Overlays_FelixDecode_hh */
