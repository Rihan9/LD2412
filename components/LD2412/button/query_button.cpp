#include "query_button.h"

namespace esphome {
namespace LD2412 {

void QueryButton::press_action() { this->parent_->read_all_info(); }

}  // namespace LD2412
}  // namespace esphome
