// FelixCompress.hh
// Milo Vermeulen 2018
// Compression studies in software, primarily focussing on speed.

#ifndef artdaq_dune_Overlays_FelixComp_hh
#define artdaq_dune_Overlays_FelixComp_hh

// Option for previous value subtraction.
#define PREV

#include <bitset>  // testing
#include <cmath>   // log
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "artdaq-core/Data/Fragment.hh"
#include "dune-raw-data/Overlays/FelixFormat.hh"
#include "dune-raw-data/Overlays/FelixFragment.hh"

namespace dune {

// A Huffman tree is literally a bunch of nodes that are connected in a certain
// way and may or may not have values assigned to them.
struct HuffTree {
  struct Node {
    Node* left = NULL;
    Node* right = NULL;
    uint16_t value = 0;
    size_t huffcode = 0;
    uint8_t hufflength = 0;
    unsigned frequency = 0;
    bool hasParent = false;

    void operator=(const Node& other) {
      if (this != &other) {
        left = other.left;
        right = other.right;
        value = other.value;
        huffcode = other.huffcode;
        huffcode = other.hufflength;
        frequency = other.frequency;
        hasParent = other.hasParent;
      }
    }

    // Operator< overloading for ordering by frequency. If the frequency is the
    // same, the values in the nodes are taken into account.
    bool operator<(const Node& other) const {
      return (frequency < other.frequency) ||
             ((frequency == other.frequency) && (value < other.value));
    }
  };

  Node* root;
  Node* nodelist;
  std::unordered_map<uint16_t, Node*> nodes;

  // Function to get a node from a value.
  Node* operator()(const uint16_t value) { return nodes[value]; }

  // Function to print all leaves from bottom to top.
  void print(Node* loc) {
    if (loc->left == NULL) {
      std::cout << "Node with value " << loc->value << ", frequency "
                << loc->frequency << " and huffcode "
                << std::bitset<32>(loc->huffcode) << " length "
                << (unsigned)loc->hufflength << '\n';
    } else {
      print(loc->left);
      print(loc->right);
    }
    return;
  }
  void print() { return print(root); }

  // Function to generate codes for a Huffman tree.
  void generate_codes(Node* loc, size_t buff = 0, uint8_t len = 0) {
    // Assign a string when a leaf is reached. Otherwise go one layer deeper.
    if (loc->left == NULL) {
      loc->huffcode = buff;
      loc->hufflength = len;
      nodes[loc->value] = loc;
    } else {
      // Move one node down.
      generate_codes(loc->left, buff, len + 1);
      generate_codes(loc->right, buff | (1 << len), len + 1);
    }

    // Move one node up.
    return;
  }
  void generate_codes() { return generate_codes(root); }

  // Function to create a tree from a buffer of nodes.
  std::unordered_map<uint16_t, Node*> make_tree(std::vector<Node> nodevec) {
    // Order vector to get the same results every time.
    std::sort(nodevec.begin(), nodevec.end());
    // Create a new buffer to hold the leaves and N-1 new nodes.
    const unsigned totlen = 2 * nodevec.size() - 1;
    Node* begin = &nodevec[0];
    nodelist = new Node[totlen];
    std::copy(begin, begin + nodevec.size(), nodelist);

    // Continue until the buffer is filled.
    for (unsigned i = 0; i < totlen - nodevec.size(); ++i) {
      // Find the lowest-frequency non-parented nodes.
      Node* lowest = nodelist;
      while (lowest->hasParent) {
        ++lowest;
      }
      Node* sec_lowest = lowest + 1;
      while (sec_lowest->hasParent) {
        ++sec_lowest;
      }
      for (Node* n = nodelist; n != nodelist + nodevec.size() + i; ++n) {
        // Disregard parented nodes.
        if (n->hasParent) {
          continue;
        }
        if (n->frequency < lowest->frequency) {
          sec_lowest = lowest;
          lowest = n;
        } else if (n->frequency < sec_lowest->frequency && n != lowest) {
          sec_lowest = n;
        }
      }

      // Make lowest value the left node in case of equal frequency.
      if (lowest->frequency == sec_lowest->frequency &&
          lowest->value > sec_lowest->value) {
        Node* tmp = lowest;
        lowest = sec_lowest;
        sec_lowest = tmp;
      }

      // Link the lowest frequency nodes to a new one in the buffer.
      Node* newNode = &nodelist[nodevec.size() + i];
      newNode->left = lowest;
      newNode->right = sec_lowest;
      newNode->frequency = lowest->frequency + sec_lowest->frequency;
      lowest->hasParent = true;
      sec_lowest->hasParent = true;
    }

    // Assign root to the last unparented node and generate codes.
    root = &nodelist[totlen - 1];
    generate_codes();

    // Output the generated map.
    return nodes;
  }

  ~HuffTree() { delete[] nodelist; }
};

struct MetaData {
  uint32_t comp_method : 2, unique_values : 14, num_frames : 16;
};

//========================
// FELIX compressor class
//========================
class FelixCompressor {
 private:
  // Input and output buffers.
  const void* input;
  const size_t input_length;
  const size_t num_frames = input_length / sizeof(FelixFrame);

  // Frame access to data.
  FelixFrame const* frame_(const size_t& frame_num = 0) const {
    return static_cast<FelixFrame const*>(input) + frame_num;
  }

  // Huffman tree storage for easy access.
  HuffTree hufftree;

 public:
  FelixCompressor(const uint8_t* data, const size_t num_frames = 10000)
      : input(data), input_length(num_frames * sizeof(dune::FelixFrame)) {
    std::cout << "Compressing FELIX frames from binary of " << input_length
              << " bytes.\n";
  }
  FelixCompressor(const dune::FelixFragment& frag)
      : input(frag.dataBeginBytes()), input_length(frag.dataSizeBytes()) {
    std::cout << "Compressing FELIX fragment from overlay of " << input_length
              << " bytes.\n";
  }

  // Function to store metadata in a recognisable format.
  void store_metadata(std::vector<char>& out) {
    // Clear the vector as a first action to make sure there is a clean slate to
    // work with.
    out.clear();

    MetaData meta = {1, 0, (uint32_t)num_frames};
    out.resize(sizeof(meta));
    memcpy(&out[0], &meta, sizeof(meta));
  }

  // Function to reduce headers and produce fatal errors upon error encounter.
  void header_reduce(std::vector<char>& out) {
    // Check all error fields and increments of timestamp and CCC.
    for (unsigned i = 1; i < num_frames; ++i) {
      bool check_failed = false;

      // WIB header checks.
      check_failed |= frame_()->sof() ^ frame_(i)->sof();
      check_failed |= frame_()->version() ^ frame_(i)->version();
      check_failed |= frame_()->fiber_no() ^ frame_(i)->fiber_no();
      check_failed |= frame_()->crate_no() ^ frame_(i)->crate_no();
      check_failed |= frame_()->slot_no() ^ frame_(i)->slot_no();
      check_failed |= frame_()->mm() ^ frame_(i)->mm();
      check_failed |= frame_()->oos() ^ frame_(i)->oos();
      check_failed |= frame_()->wib_errors() ^ frame_(i)->wib_errors();
      check_failed |= frame_()->z() ^ frame_(i)->z();

      check_failed |=
          (uint64_t)(frame_()->timestamp() + 25 * i) ^ frame_(i)->timestamp();

      // COLDATA header checks.
      for (unsigned j = 0; j < 4; ++j) {
        check_failed |= frame_()->s1_error(j) ^ frame_(i)->s1_error(j);
        check_failed |= frame_()->s2_error(j) ^ frame_(i)->s2_error(j);
        check_failed |= frame_()->checksum_a(j) ^ frame_(i)->checksum_a(j);
        check_failed |= frame_()->checksum_b(j) ^ frame_(i)->checksum_b(j);
        check_failed |=
            frame_()->error_register(j) ^ frame_(i)->error_register(j);
        for (unsigned h = 0; h < 8; ++h) {
          check_failed |= frame_()->hdr(j, h) ^ frame_(i)->hdr(j, h);
        }

        check_failed |=
            (uint16_t)(frame_()->coldata_convert_count(j) + 25 * i) ^
            frame_(i)->coldata_convert_count(j);
      }

      // Fatal error if anything failed.
      if (check_failed) {
        std::cout << "Check failed at the " << i << "th frame.\n";

        std::cout << "First frame:\n";
        frame_(i - 1)->print();
        std::cout << "Second frame:\n";
        frame_(i)->print();

        exit(1);
      }
    }  // Loop over all frames.

    // Record first header.
    const size_t tail = out.size();
    std::cout << "Out size: " << tail << '\n';
    out.resize(tail + sizeof(WIBHeader) + sizeof(ColdataHeader));

    memcpy(&out[tail], frame_()->wib_header(), sizeof(WIBHeader));
    memcpy(&out[tail + sizeof(WIBHeader)], frame_()->coldata_header(),
           sizeof(ColdataHeader));
  }

  // Function to generate a Huffman table and tree.
  void generate_Huff_tree(std::vector<char>& out) {
    // Build a frequency table.
    std::unordered_map<uint16_t, uint32_t> freq_table;
    for (unsigned vi = 0; vi < frame_()->num_ch_per_frame; ++vi) {
      freq_table[frame_(0)->channel(vi)]++;
      for (unsigned fri = 1; fri < num_frames; ++fri) {
        adc_t curr_val = frame_(fri)->channel(vi);
#ifdef PREV
        curr_val -= frame_(fri - 1)->channel(vi);
#endif
        freq_table[curr_val]++;
      }
    }

    // Save the number of unique values to the metadata.
    MetaData* meta = reinterpret_cast<MetaData*>(&out[0]);
    meta->unique_values = freq_table.size();

    // Save the frequency table to the outgoing buffer so that the decompressor
    // can make a Huffman tree of its own.
    size_t tail = out.size();
    out.resize(tail + freq_table.size() * (sizeof(adc_t) + 4));
    for (auto p : freq_table) {
      memcpy(&out[tail], &p.first, sizeof(p.first));
      memcpy(&out[tail + sizeof(p.first)], &p.second, sizeof(p.second));
      tail += sizeof(p.first) + sizeof(p.second);
    }

    // Insert the frequency table into Huffman tree nodes and calculate the
    // information entropy according to Shannon.
    std::vector<HuffTree::Node> nodes;
    double entropy = 0;
    const unsigned num_vals = num_frames * frame_()->num_ch_per_frame;
    for (auto p : freq_table) {
      HuffTree::Node curr_node;
      curr_node.value = p.first;
      curr_node.frequency = p.second;
      nodes.push_back(curr_node);

      entropy += (double)p.second / num_vals * log((double)num_vals / p.second);
    }
    std::cout << "ADC value entropy: " << entropy << " bits.\n";

    // Connect the nodes in the tree according to Huffman's method.
    hufftree.make_tree(nodes);
  }

  // Function to write ADC values using the Huffman table.
  void ADC_compress(std::vector<char>& out) {
    // Resize the output buffer appropriately for the worst case.
    const size_t tail = out.size();
    out.resize(tail + sizeof(adc_t) * num_frames * 256);
    // Record the encoded ADC values into the buffer.
    unsigned rec_bits = 0;
    char* dest = &out[0] + tail;
    for (unsigned i = 0; i < num_frames * 256; ++i) {
      adc_t curr_val = frame_(i % num_frames)->channel(i / num_frames);
// Possibly use previous value subtraction.
#ifdef PREV
      if (i % num_frames != 0) {
        curr_val -= frame_(i % num_frames - 1)->channel(i / num_frames);
      }
#endif
      char* curr_dest = dest + rec_bits / 8;
      const size_t curr_code = hufftree(curr_val)->huffcode;
      // Keep track of how many bits are left to record.
      int bits_left = hufftree(curr_val)->hufflength;
      // Fill current byte if it is partly filled already.
      if (rec_bits % 8 != 0) {
        *curr_dest |= curr_code << rec_bits % 8;
        bits_left -= 8 - rec_bits % 8;
        ++curr_dest;
      }
      // Go through full bytes and fill them.
      while (bits_left > 0) {
        *curr_dest |= curr_code >> (hufftree(curr_val)->hufflength - bits_left);
        bits_left -= 8;
        ++curr_dest;
      }

      rec_bits += hufftree(curr_val)->hufflength;
    }
    // Resize output buffer to actual data length.
    out.resize(tail + rec_bits / 8 + 1);
  }

  // Function that calls all others relevant for compression.
  void compress_copy(std::vector<char>& out) {
    std::cout << "Initiated the compress_copy function.\n";

    store_metadata(out);
    header_reduce(out);
    generate_Huff_tree(out);
    ADC_compress(out);

    std::cout << "Compressed buffer with factor "
              << (double)input_length / out.size() << ".\n";
  }
};  // FelixCompressor

// Single function for compressing data from a fragment.
std::vector<char> FelixCompress(const dune::FelixFragment& frag) {
  FelixCompressor compressor(frag);
  std::vector<char> result;
  compressor.compress_copy(result);

  return result;
}

// Similarly, a function for decompressing data and returning an
// artdaq::Fragment.
artdaq::Fragment FelixDecompress(const std::vector<char>& buff) {
  artdaq::Fragment result;
  const char* src = buff.data();

  // Read metadata and apply to fragment.
  const MetaData* meta = reinterpret_cast<MetaData const*>(src);
  size_t num_frames = meta->num_frames;
  uint16_t unique_values = meta->unique_values;
  result.resizeBytes(num_frames * sizeof(FelixFrame));
  src += sizeof(MetaData);

  // Handle for filling fragment data.
  FelixFrame* frame = reinterpret_cast<FelixFrame*>(result.dataBeginBytes());

  // Access saved WIB header and generate headers.
  const WIBHeader* whead = reinterpret_cast<WIBHeader const*>(src);
  for (unsigned i = 0; i < num_frames; ++i) {
    (frame + i)->set_sof(whead->sof);
    (frame + i)->set_version(whead->version);
    (frame + i)->set_fiber_no(whead->fiber_no);
    (frame + i)->set_crate_no(whead->crate_no);
    (frame + i)->set_slot_no(whead->slot_no);
    (frame + i)->set_mm(whead->mm);
    (frame + i)->set_oos(whead->oos);
    (frame + i)->set_wib_errors(whead->wib_errors);
    (frame + i)->set_timestamp(whead->timestamp() + i * 25);
    (frame + i)->set_wib_counter(whead->wib_counter());
    (frame + i)->set_z(whead->z);
  }
  src += sizeof(WIBHeader) + sizeof(ColdataHeader);

  // Read frequency table and generate a Huffman tree.
  std::unordered_map<uint16_t, unsigned> freq_table;
  for (unsigned i = 0; i < unique_values; ++i) {
    const uint16_t* v = reinterpret_cast<uint16_t const*>(src);
    const uint32_t* f =
        reinterpret_cast<uint32_t const*>(src + sizeof(uint16_t));
    freq_table[*v] = *f;
    src += sizeof(uint16_t) + sizeof(uint32_t);
  }
  std::vector<HuffTree::Node> nodes;
  for (auto p : freq_table) {
    HuffTree::Node curr_node;
    curr_node.value = p.first;
    curr_node.frequency = p.second;
    nodes.push_back(curr_node);
  }
  HuffTree hufftree;
  hufftree.make_tree(nodes);

  // Read ADC values from the buffer.
  size_t bits_read = 0;
  for (unsigned i = 0; i < num_frames * 256; ++i) {
    // Pointer to walk through the tree.
    HuffTree::Node* pnode = hufftree.root;
    while (pnode->left != NULL) {
      if (*src >> (bits_read % 8) & 1) {  // Next bit is 1.
        pnode = pnode->right;
      } else {  // Next bit is 0.
        pnode = pnode->left;
      }
      // Increment bits read and possibly the source pointer.
      ++bits_read;
      if (bits_read % 8 == 0) {
        ++src;  // Next byte reached.
      }
    }
    // Value reached.
    adc_t found_val = pnode->value;
#ifdef PREV
    if (i % num_frames != 0) {
      found_val += (frame + i % num_frames - 1)->channel(i / num_frames);
    }
#endif
    (frame + i % num_frames)->set_channel(i / num_frames, found_val);
  }

  return result;
}

}  // namespace dune

#endif /* artdaq_dune_Overlays_FelixComp_hh */
