#pragma once

#include "esphome/components/number/number.h"
#include "../LD2412.h"

namespace esphome {
namespace LD2412 {

class GateThresholdNumber : public number::Number, public Parented<LD2412Component> {
 public:
  GateThresholdNumber(uint8_t gate);

 protected:
  uint8_t gate_;
  void control(float value) override;
};

}  // namespace LD2412
}  // namespace esphome
