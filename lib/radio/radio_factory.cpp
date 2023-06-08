/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "srsran/radio/radio_factory.h"

#ifdef ENABLE_UHD
#include "uhd/radio_uhd_impl.h"
#endif // ENABLE_UHD

#ifdef ENABLE_ZMQ
#include "zmq/radio_factory_zmq_impl.h"
#endif // ENABLE_ZMQ

using namespace srsran;

namespace {

struct radio_factory_entry {
  std::string                                     name;
  std::function<std::unique_ptr<radio_factory>()> make;
};

} // namespace

static const std::vector<radio_factory_entry> radio_factory_available_factories = {
#ifdef ENABLE_UHD
    {"uhd", []() { return std::make_unique<radio_factory_uhd_impl>(); }},
#endif // ENABLE_UHD
#ifdef ENABLE_ZMQ
    {"zmq", []() { return std::make_unique<radio_factory_zmq_impl>(); }},
#endif // ENABLE_ZMQ
};

void srsran::print_available_radio_factories()
{
  if (radio_factory_available_factories.empty()) {
    return;
  }

  // Print available factories.
  fmt::print("Available radio types: ");
  for (unsigned i = 0, e = radio_factory_available_factories.size(); i != e; ++i) {
    if (i > 0) {
      if (i == radio_factory_available_factories.size() - 1) {
        fmt::print(" and ");
      } else {
        fmt::print(", ");
      }
    }
    fmt::print("{}", radio_factory_available_factories[i].name);
  }
  fmt::print(".\n");
}

std::unique_ptr<radio_factory> srsran::create_radio_factory(std::string driver_name)
{
  if (radio_factory_available_factories.empty()) {
    fmt::print("No radio devices available.\n");
    return nullptr;
  }

  // Convert driver name to lower case.
  for (char& c : driver_name) {
    c = std::tolower(c);
  }

  // Iterate all available driver names.
  for (const radio_factory_entry& entry : radio_factory_available_factories) {
    if (entry.name == driver_name) {
      return entry.make();
    }
  }

  // No match, print available factories.
  fmt::print("Factory for radio type {} not found. Make sure to select a valid type.\n", driver_name);

  return nullptr;
}
