#include "distance_resolution_select.h"

namespace esphome {
namespace LD2412 {

void DistanceResolutionSelect::control(const std::string &value) {
  this->publish_state(value);
  this->parent_->set_distance_resolution(state);
}

}  // namespace LD2412
}  // namespace esphome
