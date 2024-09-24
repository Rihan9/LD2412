#include "mode_select.h"

namespace esphome {
namespace LD2412 {

void ModeSelect::control(const std::string &value) {
  this->publish_state(value);
  this->parent_->set_mode(state);
}

}  // namespace LD2412
}  // namespace esphome
