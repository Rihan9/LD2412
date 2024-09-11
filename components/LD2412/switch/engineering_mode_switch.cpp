#include "engineering_mode_switch.h"

namespace esphome {
namespace LD2412 {

void EngineeringModeSwitch::write_state(bool state) {
  this->publish_state(state);
  this->parent_->set_engineering_mode(state);
}

}  // namespace LD2412
}  // namespace esphome
