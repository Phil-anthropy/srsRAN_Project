/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "pdsch_encoder_impl.h"
#include "srsran/srsvec/bit.h"

using namespace srsran;

void pdsch_encoder_impl::encode(span<uint8_t>        codeword,
                                span<const uint8_t>  transport_block,
                                const configuration& config)
{
  // Clear the buffer.
  d_segments.clear();

  segmenter_config segmenter_cfg;
  segmenter_cfg.base_graph     = config.base_graph;
  segmenter_cfg.rv             = config.rv;
  segmenter_cfg.mod            = config.mod;
  segmenter_cfg.Nref           = config.Nref;
  segmenter_cfg.nof_layers     = config.nof_layers;
  segmenter_cfg.nof_ch_symbols = config.nof_ch_symbols;

  // Segmentation (it includes CRC attachment for the entire transport block and each individual segment).
  segmenter->segment(d_segments, transport_block, segmenter_cfg);

  unsigned offset = 0;
  for (unsigned i_cb = 0, i_cb_end = d_segments.size(); i_cb != i_cb_end; ++i_cb) {
    // Select segment description.
    const described_segment& descr_seg = d_segments[i_cb];

    // Prepare rate matching buffer.
    rm_buffer.resize(descr_seg.get_metadata().cb_specific.full_length);

    // Encode the segment into a codeblock.
    encoder->encode(rm_buffer, descr_seg.get_data(), descr_seg.get_metadata().tb_common);

    // Select the correct chunk of the output codeword.
    unsigned rm_length = descr_seg.get_metadata().cb_specific.rm_length;
    srsran_assert(offset + rm_length <= codeword.size(), "Wrong codeword length.");
    span<uint8_t> codeblock = span<uint8_t>(codeword).subspan(offset, rm_length);

    // Rate match the codeblock.
    codeblock_packed.resize(rm_length);
    rate_matcher->rate_match(codeblock_packed, rm_buffer, descr_seg.get_metadata());

    // Unpack code block.
    srsvec::bit_unpack(codeblock, codeblock_packed);

    // Advance write code block.
    offset += rm_length;
  }
}
