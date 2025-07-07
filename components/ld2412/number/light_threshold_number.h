#pragma once

#include "esphome/components/number/number.h"
#include "../ld2412.h"

namespace esphome {
namespace ld2412 {

class LightThresholdNumber : public number::Number, public Parented<LD2412Component> {
 public:
  LightThresholdNumber() = default;

 protected:
  void control(float value) override;
};

}  // namespace ld2412
}  // namespace esphome
