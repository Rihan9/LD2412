#pragma once

#include "esphome/components/switch/switch.h"
#include "../LD2412.h"

namespace esphome {
namespace LD2412 {

class EngineeringModeSwitch : public switch_::Switch, public Parented<LD2412Component> {
 public:
  EngineeringModeSwitch() = default;

 protected:
  void write_state(bool state) override;
};

}  // namespace LD2412
}  // namespace esphome
