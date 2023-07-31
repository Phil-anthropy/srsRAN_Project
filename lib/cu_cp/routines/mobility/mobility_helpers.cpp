/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "mobility_helpers.h"

using namespace srsran;
using namespace srsran::srs_cu_cp;

bool srsran::srs_cu_cp::handle_context_setup_response(
    cu_cp_inter_du_handover_response&         response_msg,
    e1ap_bearer_context_modification_request& bearer_context_modification_request,
    const f1ap_ue_context_setup_response&     target_ue_context_setup_response,
    up_context&                               up_context,
    const srslog::basic_logger&               logger)
{
  // Sanity checks.
  if (target_ue_context_setup_response.ue_index == ue_index_t::invalid) {
    logger.error("Failed to create UE at the target DU.");
    return false;
  }

  if (!target_ue_context_setup_response.srbs_failed_to_be_setup_list.empty()) {
    logger.error("Couldn't setup {} SRBs at target DU",
                 target_ue_context_setup_response.srbs_failed_to_be_setup_list.size());
    return false;
  }

  if (!target_ue_context_setup_response.drbs_failed_to_be_setup_list.empty()) {
    logger.error("Couldn't setup {} DRBs at target DU",
                 target_ue_context_setup_response.drbs_failed_to_be_setup_list.size());
    return false;
  }

  if (!target_ue_context_setup_response.c_rnti.has_value()) {
    logger.error("No C-RNTI present in UE context setup");
    return false;
  }

  // Create bearer context mod request.
  if (!target_ue_context_setup_response.drbs_setup_list.empty()) {
    bearer_context_modification_request.ng_ran_bearer_context_mod_request.emplace();
  }

  auto& context_mod_request = bearer_context_modification_request.ng_ran_bearer_context_mod_request.value();

  // Extract new DL tunnel information for CU-UP.
  for (const auto& pdu_session : up_context.pdu_sessions) {
    // The modifications are only for this PDU session.
    e1ap_pdu_session_res_to_modify_item e1ap_mod_item;
    e1ap_mod_item.pdu_session_id = pdu_session.first;

    for (const auto& drb_item : pdu_session.second.drbs) {
      srsran_assert(target_ue_context_setup_response.drbs_setup_list.contains(drb_item.first),
                    "Couldn't find {} in UE context setup response",
                    drb_item.first);

      const auto& context_setup_drb_item = target_ue_context_setup_response.drbs_setup_list[drb_item.first];

      e1ap_drb_to_modify_item_ng_ran e1ap_drb_item;
      e1ap_drb_item.drb_id = drb_item.first;

      for (const auto& dl_up_param : context_setup_drb_item.dl_up_tnl_info_to_be_setup_list) {
        e1ap_up_params_item e1ap_dl_up_param;
        e1ap_dl_up_param.up_tnl_info   = dl_up_param.dl_up_tnl_info;
        e1ap_dl_up_param.cell_group_id = 0; // TODO: Remove hardcoded value

        e1ap_drb_item.dl_up_params.push_back(e1ap_dl_up_param);
      }
      e1ap_mod_item.drb_to_modify_list_ng_ran.emplace(e1ap_drb_item.drb_id, e1ap_drb_item);
    }

    context_mod_request.pdu_session_res_to_modify_list.emplace(e1ap_mod_item.pdu_session_id, e1ap_mod_item);
  }

  return target_ue_context_setup_response.success;
}
