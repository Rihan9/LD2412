#include "restart_button.h"

namespace esphome {
namespace LD2412 {

void RestartButton::press_action() { this->parent_->restart_and_read_all_info(); }

}  // namespace LD2412
}  // namespace esphome
