/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#ifndef SRSGNB_MAC_CELL_CONFIGURATION_HELPERS_H
#define SRSGNB_MAC_CELL_CONFIGURATION_HELPERS_H

#include "srsgnb/mac/cell_configuration.h"
#include "srsgnb/ran/tdd_ul_dl_config.h"

// TODO: This file is temporary. Eventually we will receive cell configurations from the DU config file.

namespace srsgnb {

namespace test_helpers {

inline carrier_configuration make_default_carrier_configuration()
{
  carrier_configuration cfg;
  cfg.carrier_bw_mhz = 10;
  cfg.arfcn          = 365000;
  cfg.nof_ant        = 1;
  return cfg;
}

inline tdd_ul_dl_config_common make_default_tdd_ul_dl_config_common()
{
  tdd_ul_dl_config_common cfg{};
  cfg.ref_scs                            = subcarrier_spacing::kHz15;
  cfg.pattern1.dl_ul_tx_period_nof_slots = 10;
  cfg.pattern1.nof_dl_slots              = 6;
  cfg.pattern1.nof_dl_symbols            = 0;
  cfg.pattern1.nof_ul_slots              = 3;
  cfg.pattern1.nof_ul_symbols            = 0;
  return cfg;
}

inline mac_cell_creation_request make_default_mac_cell_creation_request()
{
  mac_cell_creation_request msg{};

  msg.cell_index = to_du_cell_index(0);
  msg.pci        = 1;

  msg.scs_common = subcarrier_spacing::kHz15;
  msg.dl_carrier = make_default_carrier_configuration();
  msg.ul_carrier = make_default_carrier_configuration();

  // Configure FrequencyInfoDL.
  msg.dl_cfg_common.freq_info_dl.offset_to_point_a = 0;
  msg.dl_cfg_common.freq_info_dl.scs_carrier_list.emplace_back();
  msg.dl_cfg_common.freq_info_dl.scs_carrier_list.back().scs               = subcarrier_spacing::kHz15;
  msg.dl_cfg_common.freq_info_dl.scs_carrier_list.back().offset_to_carrier = 0;
  msg.dl_cfg_common.freq_info_dl.scs_carrier_list.back().carrier_bandwidth = 52;

  // Configure initial DL BWP.
  msg.dl_cfg_common.init_dl_bwp.generic_params.scs  = subcarrier_spacing::kHz15;
  msg.dl_cfg_common.init_dl_bwp.generic_params.crbs = {0, 52};
  msg.dl_cfg_common.init_dl_bwp.pdcch_common.coresets.emplace(0);
  msg.dl_cfg_common.init_dl_bwp.pdcch_common.coresets[0].id = to_coreset_id(0);
  msg.dl_cfg_common.init_dl_bwp.pdcch_common.search_spaces.emplace(0);
  msg.dl_cfg_common.init_dl_bwp.pdcch_common.search_spaces[0].id    = to_search_space_id(0);
  msg.dl_cfg_common.init_dl_bwp.pdcch_common.search_spaces[0].cs_id = to_coreset_id(0);
  msg.dl_cfg_common.init_dl_bwp.pdcch_common.search_spaces.emplace(1);
  msg.dl_cfg_common.init_dl_bwp.pdcch_common.search_spaces[1].id    = to_search_space_id(1);
  msg.dl_cfg_common.init_dl_bwp.pdcch_common.search_spaces[1].cs_id = to_coreset_id(0);
  msg.dl_cfg_common.init_dl_bwp.pdcch_common.ra_search_space_id     = to_search_space_id(1);

  // Configure initial UL BWP.
  msg.ul_cfg_common.init_ul_bwp.generic_params = msg.dl_cfg_common.init_dl_bwp.generic_params;
  msg.ul_cfg_common.init_ul_bwp.rach_cfg_common.emplace();
  msg.ul_cfg_common.init_ul_bwp.rach_cfg_common->total_nof_ra_preambles            = 64;
  msg.ul_cfg_common.init_ul_bwp.rach_cfg_common->prach_root_seq_index_l839_present = true;
  msg.ul_cfg_common.init_ul_bwp.rach_cfg_common->prach_root_seq_index              = 1;
  msg.ul_cfg_common.init_ul_bwp.rach_cfg_common->rach_cfg_generic.ra_resp_window   = 10;
  msg.ul_cfg_common.init_ul_bwp.pusch_cfg_common.emplace();
  msg.ul_cfg_common.init_ul_bwp.pusch_cfg_common->pusch_td_alloc_list.resize(1);
  msg.ul_cfg_common.init_ul_bwp.pusch_cfg_common->pusch_td_alloc_list[0].k2 = 2;

  // TODO: Remaining fields.

  return msg;
}

} // namespace test_helpers

} // namespace srsgnb

#endif // SRSGNB_MAC_CELL_CONFIGURATION_HELPERS_H
