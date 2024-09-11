#pragma once

#include "esphome/components/select/select.h"
#include "../LD2412.h"

namespace esphome {
namespace LD2412 {

class BaudRateSelect : public select::Select, public Parented<LD2412Component> {
 public:
  BaudRateSelect() = default;

 protected:
  void control(const std::string &value) override;
};

}  // namespace LD2412
}  // namespace esphome
