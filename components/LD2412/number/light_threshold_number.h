#pragma once

#include "esphome/components/number/number.h"
#include "../LD2412.h"

namespace esphome {
namespace LD2412 {

class LightThresholdNumber : public number::Number, public Parented<LD2412Component> {
 public:
  LightThresholdNumber() = default;

 protected:
  void control(float value) override;
};

}  // namespace LD2412
}  // namespace esphome
