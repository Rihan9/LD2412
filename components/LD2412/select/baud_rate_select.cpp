#include "baud_rate_select.h"

namespace esphome {
namespace LD2412 {

void BaudRateSelect::control(const std::string &value) {
  this->publish_state(value);
  this->parent_->set_baud_rate(state);
}

}  // namespace LD2412
}  // namespace esphome
