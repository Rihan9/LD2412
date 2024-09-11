#include "bluetooth_switch.h"

namespace esphome {
namespace LD2412 {

void BluetoothSwitch::write_state(bool state) {
  this->publish_state(state);
  this->parent_->set_bluetooth(state);
}

}  // namespace LD2412
}  // namespace esphome
