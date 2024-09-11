#include "reset_button.h"

namespace esphome {
namespace LD2412 {

void ResetButton::press_action() { this->parent_->factory_reset(); }

}  // namespace LD2412
}  // namespace esphome
