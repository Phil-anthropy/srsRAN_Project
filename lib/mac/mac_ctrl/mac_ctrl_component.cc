
#include "mac_ctrl_component.h"
#include "ue_creation_procedure.h"
#include "ue_delete_procedure.h"

using namespace srsgnb;

mac_ctrl_component::mac_ctrl_component(mac_common_config_t& cfg_,
                                       mac_ul_component&    ul_unit_,
                                       mac_dl_component&    dl_unit_) :
  cfg(cfg_), logger(cfg.logger), ul_unit(ul_unit_), dl_unit(dl_unit_)
{
  std::fill(rnti_to_ue_index_map.begin(), rnti_to_ue_index_map.end(), MAX_NOF_UES);
}

void mac_ctrl_component::ue_create_request(const mac_ue_create_request_message& msg)
{
  ue_element* u = add_ue(msg.ue_index, msg.crnti, msg.cell_index);
  if (u == nullptr) {
    mac_ue_create_request_response_message resp{};
    resp.ue_index   = msg.ue_index;
    resp.cell_index = msg.cell_index;
    resp.result     = false;
    cfg.cfg_notifier.on_ue_create_request_complete(resp);
    return;
  }
  // UE object added to ue_db successfully

  // Enqueue UE create request procedure
  u->ctrl_loop.schedule<mac_ue_create_request_procedure>(msg, cfg, ul_unit, dl_unit);
}

void mac_ctrl_component::ue_delete_request(const mac_ue_delete_request_message& msg)
{
  if (not ue_db.contains(msg.ue_index)) {
    logger.warning("Failed to find ueId={}", msg.ue_index);
    mac_ue_delete_response_message resp{};
    resp.ue_index = msg.ue_index;
    resp.result   = false;
    cfg.cfg_notifier.on_ue_delete_complete(resp);
    return;
  }
  ue_element& u = ue_db[msg.ue_index];

  // Enqueue UE delete procedure
  u.ctrl_loop.schedule<mac_ue_delete_procedure>(msg, cfg, ul_unit, dl_unit);
}

mac_ctrl_component::ue_element*
mac_ctrl_component::add_ue(du_ue_index_t ue_index, rnti_t crnti, du_cell_index_t cell_index)
{
  srsran_assert(crnti != INVALID_RNTI, "Invalid RNTI");
  srsran_assert(ue_index < MAX_NOF_UES, "Invalid ue_index=%d", ue_index);

  if (rnti_to_ue_index_map[crnti % MAX_NOF_UES] < MAX_NOF_UES) {
    // rnti already exists
    return nullptr;
  }

  if (ue_db.contains(ue_index)) {
    // UE already existed with same ue_index
    return nullptr;
  }

  // Create UE object
  ue_db.emplace(ue_index);
  ue_element& u        = ue_db[ue_index];
  u.ue_ctx.du_ue_index = ue_index;
  u.ue_ctx.rnti        = crnti;
  u.ue_ctx.pcell_idx   = cell_index;

  // Update RNTI -> UE index map
  rnti_to_ue_index_map[crnti % MAX_NOF_UES] = ue_index;
  return &u;
}

bool mac_ctrl_component::remove_ue(du_ue_index_t ue_index)
{
  srsran_assert(ue_index < MAX_NOF_UES, "Invalid ue_index=%d", ue_index);

  if (not ue_db.contains(ue_index)) {
    // no UE existed with provided ue_index
    return false;
  }
  srsran_sanity_check(ue_db[ue_index].ue_ctx.rnti != INVALID_RNTI, "ue_index=%d has invalid RNTI", ue_index);
  ue_db.erase(ue_index);

  return true;
}

mac_ue_context* mac_ctrl_component::find_ue(du_ue_index_t ue_index)
{
  srsran_assert(ue_index < MAX_NOF_UES, "Invalid ue_index=%d", ue_index);
  return ue_db.contains(ue_index) ? &ue_db[ue_index].ue_ctx : nullptr;
}

mac_ue_context* mac_ctrl_component::find_by_rnti(rnti_t rnti)
{
  srsran_assert(rnti != INVALID_RNTI, "Invalid rnti=0x%x", rnti);
  du_ue_index_t ue_index = rnti_to_ue_index_map[rnti % MAX_NOF_UES];
  if (ue_index == MAX_NOF_UES) {
    return nullptr;
  }
  return find_ue(ue_index);
}
