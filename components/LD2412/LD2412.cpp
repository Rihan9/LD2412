#include "LD2412.h"

#include <utility>
#if  defined(USE_SELECT) || defined(USE_NUMBER)
#include "esphome/components/number/number.h"
#endif
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif

#define highbyte(val) (uint8_t)((val) >> 8)
#define lowbyte(val) (uint8_t)((val) &0xff)

namespace esphome {
namespace LD2412 {

static const char *const TAG = "LD2412";

LD2412Component::LD2412Component() {}

void LD2412Component::dump_config() {
  ESP_LOGCONFIG(TAG, "LD2412:");
#ifdef USE_BINARY_SENSOR
  LOG_BINARY_SENSOR("  ", "TargetBinarySensor", this->target_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "MovingTargetBinarySensor", this->moving_target_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "StillTargetBinarySensor", this->still_target_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "OutPinPresenceStatusBinarySensor", this->out_pin_presence_status_binary_sensor_);
#endif
#ifdef USE_SWITCH
//  LOG_SWITCH("  ", "EngineeringModeSwitch", this->engineering_mode_switch_);
//  LOG_SWITCH("  ", "BluetoothSwitch", this->bluetooth_switch_);
#endif
#ifdef USE_BUTTON
  LOG_BUTTON("  ", "ResetButton", this->reset_button_);
  LOG_BUTTON("  ", "RestartButton", this->restart_button_);
  LOG_BUTTON("  ", "QueryButton", this->query_button_);
#endif
#ifdef USE_SENSOR
  //LOG_SENSOR("  ", "LightSensor", this->light_sensor_);
  LOG_SENSOR("  ", "MovingTargetDistanceSensor", this->moving_target_distance_sensor_);
  LOG_SENSOR("  ", "StillTargetDistanceSensor", this->still_target_distance_sensor_);
  LOG_SENSOR("  ", "MovingTargetEnergySensor", this->moving_target_energy_sensor_);
  LOG_SENSOR("  ", "StillTargetEnergySensor", this->still_target_energy_sensor_);
  LOG_SENSOR("  ", "DetectionDistanceSensor", this->detection_distance_sensor_);
  //for (sensor::Sensor *s : this->gate_still_sensors_) {
  //  LOG_SENSOR("  ", "NthGateStillSesnsor", s);
  //}
  //for (sensor::Sensor *s : this->gate_move_sensors_) {
  //  LOG_SENSOR("  ", "NthGateMoveSesnsor", s);
  //}
#endif
#ifdef USE_TEXT_SENSOR
  LOG_TEXT_SENSOR("  ", "VersionTextSensor", this->version_text_sensor_);
  LOG_TEXT_SENSOR("  ", "MacTextSensor", this->mac_text_sensor_);
#endif
#if  defined(USE_SELECT) || defined(USE_NUMBER)
//  LOG_SELECT("  ", "LightFunctionSelect", this->light_function_select_);
  LOG_SELECT("  ", "OutPinLevelSelect", this->out_pin_level_select_);
  //LOG_SELECT("  ", "DistanceResolutionSelect", this->distance_resolution_select_);
  LOG_SELECT("  ", "BaudRateSelect", this->baud_rate_select_);
  LOG_SELECT("  ", "ModeSelect", this->mode_select_);
#endif
#if  defined(USE_SELECT) || defined(USE_NUMBER)
//  LOG_NUMBER("  ", "LightThresholdNumber", this->light_threshold_number_);
  LOG_NUMBER("  ", "MaxDistanceGateNumber", this->max_distance_gate_number_);
  LOG_NUMBER("  ", "MinDistanceGateNumber", this->min_distance_gate_number_);
  LOG_NUMBER("  ", "TimeoutNumber", this->timeout_number_);
  // for (number::Number *n : this->gate_still_threshold_numbers_) {
  //   LOG_NUMBER("  ", "Still Thresholds Number", n);
  // }
  // for (number::Number *n : this->gate_move_threshold_numbers_) {
  //   LOG_NUMBER("  ", "Move Thresholds Number", n);
  // }
#endif
  this->read_all_info();
  ESP_LOGCONFIG(TAG, "  Throttle_ : %ums", this->throttle_);
  ESP_LOGCONFIG(TAG, "  MAC Address : %s", const_cast<char *>(this->mac_.c_str()));
  ESP_LOGCONFIG(TAG, "  Firmware Version : %s", const_cast<char *>(this->version_.c_str()));
}

void LD2412Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up LD2412...");
  this->read_all_info();
  ESP_LOGCONFIG(TAG, "Mac Address : %s", const_cast<char *>(this->mac_.c_str()));
  ESP_LOGCONFIG(TAG, "Firmware Version : %s", const_cast<char *>(this->version_.c_str()));
  ESP_LOGCONFIG(TAG, "LD2412 setup complete.");
}

void LD2412Component::read_all_info() {
  this->set_config_mode_(true);
  this->get_version_();
  delay(10);  // NOLINT
  this->get_mac_();
  delay(10);  // NOLINT
  this->get_distance_resolution_();
  delay(10);  // NOLINT
  //this->get_light_control_();
  this->query_parameters_();
  delay(10);  // NOLINT
  this->query_dymanic_background_correction_();
  delay(10);  // NOLINT
#if  defined(USE_SELECT) || defined(USE_NUMBER)
  this->get_gate_threshold();
  delay(10);  // NOLINT
#endif
  this->set_config_mode_(false);
#if  defined(USE_SELECT) || defined(USE_NUMBER)
  const auto baud_rate = std::to_string(this->parent_->get_baud_rate());
  if (this->baud_rate_select_ != nullptr && this->baud_rate_select_->state != baud_rate) {
    this->baud_rate_select_->publish_state(baud_rate);
  }
#endif
}

void LD2412Component::restart_and_read_all_info() {
  this->set_config_mode_(true);
  this->restart_();
  this->set_timeout(1000, [this]() { this->read_all_info(); });
}

void LD2412Component::loop() {
  const int max_line_length = 80;
  static uint8_t buffer[max_line_length];

  while (available()) {
    this->readline_(read(), buffer, max_line_length);
  }
}

void LD2412Component::send_command_(uint8_t command, const uint8_t *command_value, int command_value_len) {
  ESP_LOGV(TAG, "Sending COMMAND %02X", command);
  // frame start bytes
  this->write_array(CMD_FRAME_HEADER, 4);
  // length bytes
  int len = 2;
  if (command_value != nullptr)
    len += command_value_len;
  this->write_byte(lowbyte(len));
  this->write_byte(highbyte(len));

  // command
  this->write_byte(lowbyte(command));
  this->write_byte(highbyte(command));

  // command value bytes
  if (command_value != nullptr) {
    for (int i = 0; i < command_value_len; i++) {
      this->write_byte(command_value[i]);
    }
  }
  // frame end bytes
  this->write_array(CMD_FRAME_END, 4);
  // FIXME to remove
  delay(50);  // NOLINT
}

void LD2412Component::handle_periodic_data_(uint8_t *buffer, int len) {
  if (len < 12)
    return;  // 4 frame start bytes + 2 length bytes + 1 data end byte + 1 crc byte + 4 frame end bytes
  if (buffer[0] != 0xF4 || buffer[1] != 0xF3 || buffer[2] != 0xF2 || buffer[3] != 0xF1)  // check 4 frame start bytes
    return;
  if (buffer[7] != HEAD || buffer[len - 6] != END)  // Check constant values
    return;  // data head=0xAA, data end=0x55, crc=0x00

  /*
    Reduce data update rate to prevent home assistant database size grow fast
  */
  int32_t current_millis = millis();
  if (current_millis - last_periodic_millis_ < this->throttle_)
    return;
  last_periodic_millis_ = current_millis;

  /*
    Data Type: 7th
    0x01: Engineering mode
    0x02: Normal mode
  */
  bool engineering_mode = buffer[DATA_TYPES] == 0x01;
#if  defined(USE_SELECT) || defined(USE_NUMBER)
  if (this->mode_select_ != nullptr) {
    if(this->mode_select_->state == "Engineering" && !engineering_mode){
      this->mode_select_->publish_state("Normal");
    }
    if(this->mode_select_->state == "Normal" && engineering_mode){
      this->mode_select_->publish_state("Engineering");
    }
  }
#endif

//#ifdef USE_SWITCH
//  if (this->engineering_mode_switch_ != nullptr &&
//      current_millis - last_engineering_mode_change_millis_ > this->throttle_) {
//    this->engineering_mode_switch_->publish_state(engineering_mode);
//  }
//#endif
  char target_state = buffer[TARGET_STATES];
#ifdef USE_BINARY_SENSOR
  /*
    Target states: 9th
    0x00 = No target
    0x01 = Moving targets
    0x02 = Still targets
    0x03 = Moving+Still targets
  */
  if (this->target_binary_sensor_ != nullptr) {
    this->target_binary_sensor_->publish_state(target_state != 0x00);
  }
  if (this->moving_target_binary_sensor_ != nullptr) {
    this->moving_target_binary_sensor_->publish_state(CHECK_BIT(target_state, 0));
  }
  if (this->still_target_binary_sensor_ != nullptr) {
    this->still_target_binary_sensor_->publish_state(CHECK_BIT(target_state, 1));
  }
#endif
  /*
    Moving target distance: 10~11th bytes
    Moving target energy: 12th byte
    Still target distance: 13~14th bytes
    Still target energy: 15th byte
    Detect distance: 16~17th bytes
  */
#ifdef USE_SENSOR
  if (this->moving_target_distance_sensor_ != nullptr) {
    int new_moving_target_distance = target_state != 0x00 ? 
      this->two_byte_to_int_(buffer[MOVING_TARGET_LOW], buffer[MOVING_TARGET_HIGH]) :
      0;
    if (this->moving_target_distance_sensor_->get_state() != new_moving_target_distance)
      this->moving_target_distance_sensor_->publish_state(new_moving_target_distance);
  }
  if (this->moving_target_energy_sensor_ != nullptr) {
    int new_moving_target_energy = target_state != 0x00 ? 
      buffer[MOVING_ENERGY] :
      0;
    if (this->moving_target_energy_sensor_->get_state() != new_moving_target_energy)
      this->moving_target_energy_sensor_->publish_state(new_moving_target_energy);
  }
  if (this->still_target_distance_sensor_ != nullptr) {
    int new_still_target_distance = target_state != 0x00 ? 
      this->two_byte_to_int_(buffer[STILL_TARGET_LOW], buffer[STILL_TARGET_HIGH]) : 
      0;
    if (this->still_target_distance_sensor_->get_state() != new_still_target_distance)
      this->still_target_distance_sensor_->publish_state(new_still_target_distance);
  }
  if (this->still_target_energy_sensor_ != nullptr) {
    int new_still_target_energy = target_state != 0x00 ? 
      buffer[STILL_ENERGY] : 
      0;
    if (this->still_target_energy_sensor_->get_state() != new_still_target_energy)
      this->still_target_energy_sensor_->publish_state(new_still_target_energy);
  }
  if (this->detection_distance_sensor_ != nullptr) {
    int new_detect_distance = 0;
    if(target_state != 0x00 && CHECK_BIT(target_state, 0)){
      new_detect_distance = this->two_byte_to_int_(buffer[MOVING_TARGET_LOW], buffer[MOVING_TARGET_HIGH]);
    }else if(target_state != 0x00){
      new_detect_distance = this->two_byte_to_int_(buffer[STILL_TARGET_LOW], buffer[STILL_TARGET_HIGH]);
    }
    if (this->detection_distance_sensor_->get_state() != new_detect_distance)
      this->detection_distance_sensor_->publish_state(new_detect_distance);
  }
  if (engineering_mode) {
    /*
      Moving distance range: 18th byte
      Still distance range: 19th byte
      Moving enery: 20~28th bytes
    */
    for (std::vector<sensor::Sensor *>::size_type i = 0; i != this->gate_move_sensors_.size(); i++) {
      sensor::Sensor *s = this->gate_move_sensors_[i];
      if (s != nullptr) {
        s->publish_state(buffer[MOVING_SENSOR_START + i]);
      }
    }
    /*
      Still energy: 29~37th bytes
    */
    for (std::vector<sensor::Sensor *>::size_type i = 0; i != this->gate_still_sensors_.size(); i++) {
      sensor::Sensor *s = this->gate_still_sensors_[i];
      if (s != nullptr) {
        s->publish_state(buffer[STILL_SENSOR_START + i]);
      }
    }
    /*
      Light sensor: 38th bytes
    */
    if (this->light_sensor_ != nullptr) {
      int new_light_sensor = (buffer[LIGHT_SENSOR]*100)/255;
      if (this->light_sensor_->get_state() != new_light_sensor)
        this->light_sensor_->publish_state(new_light_sensor);
    }
  } 
  if(!engineering_mode) {
    for (auto *s : this->gate_move_sensors_) {
      if (s != nullptr && !std::isnan(s->get_state())) {
        s->publish_state(NAN);
      }
    }
    for (auto *s : this->gate_still_sensors_) {
      if (s != nullptr && !std::isnan(s->get_state())) {
        s->publish_state(NAN);
      }
    }
    if (this->light_sensor_ != nullptr && !std::isnan(this->light_sensor_->get_state())) {
      this->light_sensor_->publish_state(NAN);
    }
  }
  //}
#endif
#ifdef USE_BINARY_SENSOR
  if (engineering_mode) {
    if (this->out_pin_presence_status_binary_sensor_ != nullptr) {
      this->out_pin_presence_status_binary_sensor_->publish_state(buffer[OUT_PIN_SENSOR] == 0x01);
    }
  } else {
    if (this->out_pin_presence_status_binary_sensor_ != nullptr) {
      this->out_pin_presence_status_binary_sensor_->publish_state(false);
    }
  }
#endif
}

const char VERSION_FMT[] = "%u.%02X.%02X%02X%02X%02X";

std::string format_version(uint8_t *buffer) {
  std::string::size_type version_size = 256;
  std::string version;
  do {
    version.resize(version_size + 1);
    version_size = std::snprintf(&version[0], version.size(), VERSION_FMT, buffer[13], buffer[12], buffer[17],
                                 buffer[16], buffer[15], buffer[14]);
  } while (version_size + 1 > version.size());
  version.resize(version_size);
  return version;
}

const char MAC_FMT[] = "%02X:%02X:%02X:%02X:%02X:%02X";

const std::string UNKNOWN_MAC("unknown");
const std::string NO_MAC("08:05:04:03:02:01");

std::string format_mac(uint8_t *buffer) {
  std::string::size_type mac_size = 256;
  std::string mac;
  do {
    mac.resize(mac_size + 1);
    mac_size = std::snprintf(&mac[0], mac.size(), MAC_FMT, buffer[10], buffer[11], buffer[12], buffer[13], buffer[14],
                             buffer[15]);
  } while (mac_size + 1 > mac.size());
  mac.resize(mac_size);
  if (mac == NO_MAC) {
    return UNKNOWN_MAC;
  }
  return mac;
}

#if  defined(USE_SELECT) || defined(USE_NUMBER)
std::function<void(void)> set_number_value(number::Number *n, float value) {
  float normalized_value = value * 1.0;
  if (n != nullptr && (!n->has_state() || n->state != normalized_value)) {
    n->state = normalized_value;
    return [n, normalized_value]() { n->publish_state(normalized_value); };
  }
  return []() {};
}
#endif

bool LD2412Component::handle_ack_data_(uint8_t *buffer, int len) {
  ESP_LOGV(TAG, "Handling ACK DATA for COMMAND %02X", buffer[COMMAND]);
  if (len < 10) {
    ESP_LOGE(TAG, "Error with last command : incorrect length");
    return true;
  }
  if (buffer[0] != 0xFD || buffer[1] != 0xFC || buffer[2] != 0xFB || buffer[3] != 0xFA) {  // check 4 frame start bytes
    ESP_LOGE(TAG, "Error with last command : incorrect Header %02X, %02X, %02X, %02X", buffer[0], buffer[1], buffer[2], buffer[3]);
    //just a patch to handle a strange behavior. better this than have a costant wrong mode
    if(this->dynamic_bakground_correction_active_){
      this->query_dymanic_background_correction_();
    }
    return true;
  }
  if (buffer[COMMAND_STATUS] != 0x01) {
    ESP_LOGE(TAG, "Error with last command : status != 0x01");
    return true;
  }
  if (this->two_byte_to_int_(buffer[8], buffer[9]) != 0x00) {
    ESP_LOGE(TAG, "Error with last command , last buffer was: %u , %u", buffer[8], buffer[9]);
    return true;
  }
  bool dynamic_background_correction_active;
  switch (buffer[COMMAND]) {
    case lowbyte(CMD_ENABLE_CONF):
      ESP_LOGV(TAG, "Handled Enable conf command");
      break;
    case lowbyte(CMD_DISABLE_CONF):
      ESP_LOGV(TAG, "Handled Disabled conf command");
      break;
    case lowbyte(CMD_SET_BAUD_RATE):
      ESP_LOGV(TAG, "Handled baud rate change command");
#if  defined(USE_SELECT) || defined(USE_NUMBER)
      if (this->baud_rate_select_ != nullptr) {
        ESP_LOGE(TAG, "Change baud rate component config to %s and reinstall", this->baud_rate_select_->state.c_str());
      }
#endif
      break;
    case lowbyte(CMD_VERSION):
      this->version_ = format_version(buffer);
      ESP_LOGV(TAG, "FW Version is: %s", const_cast<char *>(this->version_.c_str()));
#ifdef USE_TEXT_SENSOR
      if (this->version_text_sensor_ != nullptr) {
        this->version_text_sensor_->publish_state(this->version_);
      }
#endif
      break;
    case lowbyte(CMD_QUERY_DISTANCE_RESOLUTION): {
      std::string distance_resolution =
          DISTANCE_RESOLUTION_INT_TO_ENUM.at(this->two_byte_to_int_(buffer[10], buffer[11]));
      ESP_LOGV(TAG, "Distance resolution is: %s", const_cast<char *>(distance_resolution.c_str()));
#if  defined(USE_SELECT) || defined(USE_NUMBER)
      if (this->distance_resolution_select_ != nullptr &&
          this->distance_resolution_select_->state != distance_resolution) {
        this->distance_resolution_select_->publish_state(distance_resolution);
      }
#endif
    } break;
    //case lowbyte(CMD_QUERY_LIGHT_CONTROL): {
    //  this->light_function_ = LIGHT_FUNCTION_INT_TO_ENUM.at(buffer[10]);
    //  this->light_threshold_ = buffer[11] * 1.0;
    //  this->out_pin_level_ = OUT_PIN_LEVEL_INT_TO_ENUM.at(buffer[12]);
    //  ESP_LOGV(TAG, "Light function is: %s", const_cast<char *>(this->light_function_.c_str()));
    //  ESP_LOGV(TAG, "Light threshold is: %f", this->light_threshold_);
    //  ESP_LOGV(TAG, "Out pin level is: %s", const_cast<char *>(this->out_pin_level_.c_str()));
    // #if  defined(USE_SELECT) || defined(USE_NUMBER)
    //       if (this->light_function_select_ != nullptr && this->light_function_select_->state != this->light_function_) {
    //         this->light_function_select_->publish_state(this->light_function_);
    //       }
    //       if (this->out_pin_level_select_ != nullptr && this->out_pin_level_select_->state != this->out_pin_level_) {
    //         this->out_pin_level_select_->publish_state(this->out_pin_level_);
    //       }
    // #endif
    // #if  defined(USE_SELECT) || defined(USE_NUMBER)
    //       if (this->light_threshold_number_ != nullptr &&
    //           (!this->light_threshold_number_->has_state() ||
    //            this->light_threshold_number_->state != this->light_threshold_)) {
    //         this->light_threshold_number_->publish_state(this->light_threshold_);
    //       }
    // #endif
    //     } break;
    case lowbyte(CMD_MAC):
      if (len < 20) {
        return false;
      }
      this->mac_ = format_mac(buffer);
      ESP_LOGV(TAG, "MAC Address is: %s", const_cast<char *>(this->mac_.c_str()));
#ifdef USE_TEXT_SENSOR
      if (this->mac_text_sensor_ != nullptr) {
        this->mac_text_sensor_->publish_state(this->mac_);
      }
#endif
#ifdef USE_SWITCH
      if (this->bluetooth_switch_ != nullptr) {
        this->bluetooth_switch_->publish_state(this->mac_ != UNKNOWN_MAC);
      }
#endif
      break;
    case lowbyte(CMD_SET_DISTANCE_RESOLUTION):
      ESP_LOGV(TAG, "Handled set distance resolution command");
      break;
    case lowbyte(CMD_QUEY_DYNAMIC_BACKGROUND_CORRECTION):
      ESP_LOGV(TAG, "Handled query dynamic background correction");
      dynamic_background_correction_active = (buffer[10] == 0x01);
      if(this->dynamic_bakground_correction_active_ != dynamic_background_correction_active && dynamic_background_correction_active){
        this->mode_select_->publish_state("Dynamic background correction");
      }else if(this->dynamic_bakground_correction_active_ != dynamic_background_correction_active && !dynamic_background_correction_active){
        this->mode_select_->publish_state("Normal");
      }
      this->dynamic_bakground_correction_active_ = dynamic_background_correction_active;
      if(this->dynamic_bakground_correction_active_){
        this->set_timeout(1000, [this]() { this->query_dymanic_background_correction_(); });
      }
      break;
//    case lowbyte(CMD_GATE_SENS):
//      ESP_LOGV(TAG, "Handled sensitivity command");
//      break;
//    case lowbyte(CMD_BLUETOOTH):
//      ESP_LOGV(TAG, "Handled bluetooth command");
//      break;
//    case lowbyte(CMD_SET_LIGHT_CONTROL):
//      ESP_LOGV(TAG, "Handled set light control command");
//      break;
//    case lowbyte(CMD_BT_PASSWORD):
//      ESP_LOGV(TAG, "Handled set bluetooth password command");
//      break;
    case lowbyte(CMD_QUERY_MOTION_GATE_SENS):{
      std::vector<std::function<void(void)>> updates;
      for(int i = 0; i < this->gate_still_threshold_numbers_.size(); i++){
        updates.push_back(set_number_value(this->gate_move_threshold_numbers_[i], buffer[10+i]));
      }
      for (auto &update : updates) {
        update();
      }
      break;
    }
    case lowbyte(CMD_QUERY_STATIC_GATE_SENS):{
      std::vector<std::function<void(void)>> updates;
      for(int i = 0; i < this->gate_still_threshold_numbers_.size(); i++){
        updates.push_back(set_number_value(this->gate_still_threshold_numbers_[i], buffer[10+i]));
      }
      for (auto &update : updates) {
        update();
      }
      break;
    }
    case lowbyte(CMD_QUERY):  // Query parameters response
    {
#if  defined(USE_SELECT) || defined(USE_NUMBER)
      /*
        Moving distance range: 9th byte
        Still distance range: 10th byte
      */
      std::vector<std::function<void(void)>> updates;
      updates.push_back(set_number_value(this->min_distance_gate_number_, buffer[10]));
      updates.push_back(set_number_value(this->max_distance_gate_number_, buffer[11]-1));
      ESP_LOGV(TAG, "min_distance_gate_number_: %u, max_distance_gate_number_ %u", buffer[10], buffer[11]);
      /*
        None Duration: 11~12th bytes
      */
      updates.push_back(set_number_value(this->timeout_number_, this->two_byte_to_int_(buffer[12], buffer[13])));
      ESP_LOGV(TAG, "timeout_number_: %u", this->two_byte_to_int_(buffer[12], buffer[13]));
      /*
        Output pin configuration: 13th bytes
      */
      this->out_pin_level_ = OUT_PIN_LEVEL_INT_TO_ENUM.at(buffer[14]);
#if  defined(USE_SELECT) || defined(USE_NUMBER)
      if (this->out_pin_level_select_ != nullptr && this->out_pin_level_select_->state != this->out_pin_level_) {
        this->out_pin_level_select_->publish_state(this->out_pin_level_);
      }
#endif
      /*
        Moving Sensitivities: 15~23th bytes
      */
      // for (std::vector<number::Number *>::size_type i = 0; i != this->gate_move_threshold_numbers_.size(); i++) {
      //   updates.push_back(set_number_value(this->gate_move_threshold_numbers_[i], buffer[14 + i]));
      // }
      // /*
      //   Still Sensitivities: 24~32th bytes
      // */
      // for (std::vector<number::Number *>::size_type i = 0; i != this->gate_still_threshold_numbers_.size(); i++) {
      //   updates.push_back(set_number_value(this->gate_still_threshold_numbers_[i], buffer[23 + i]));
      // }
      for (auto &update : updates) {
        update();
      }
#endif
    } break;
    default:
      break;
  }

  return true;
}

void LD2412Component::readline_(int readch, uint8_t *buffer, int len) {
  static int pos = 0;

  if (readch >= 0) {
    if (pos < len - 1) {
      buffer[pos++] = readch;
      buffer[pos] = 0;
    } else {
      pos = 0;
    }
    if (pos >= 4) {
      if (
        buffer[pos - 4] == DATA_FRAME_END[0] && 
        buffer[pos - 3] == DATA_FRAME_END[1] && 
        buffer[pos - 2] == DATA_FRAME_END[2] && 
        buffer[pos - 1] == DATA_FRAME_END[3]
      ) {
        ESP_LOGV(TAG, "Will handle Periodic Data:  %s", format_buffer(buffer, pos).c_str());
        this->handle_periodic_data_(buffer, pos);
        pos = 0;  // Reset position index ready for next time
      } else if (
        buffer[pos - 4] == CMD_FRAME_END[0] && 
        buffer[pos - 3] == CMD_FRAME_END[1] && 
        buffer[pos - 2] == CMD_FRAME_END[2] && 
        buffer[pos - 1] == CMD_FRAME_END[3]
      ) {
        ESP_LOGV(TAG, "Will handle ACK Data");
        if (this->handle_ack_data_(buffer, pos)) {
          pos = 0;  // Reset position index ready for next time
        } else {
          ESP_LOGV(TAG, "ACK Data incomplete");
        }
      }
    }
  }
}

void LD2412Component::set_config_mode_(bool enable) {
  uint8_t cmd = enable ? CMD_ENABLE_CONF : CMD_DISABLE_CONF;
  uint8_t cmd_value[2] = {0x01, 0x00};
  this->send_command_(cmd, enable ? cmd_value : nullptr, 2);
}

void LD2412Component::set_bluetooth(bool enable) {
  this->set_config_mode_(true);
  uint8_t enable_cmd_value[2] = {0x01, 0x00};
  uint8_t disable_cmd_value[2] = {0x00, 0x00};
  this->send_command_(CMD_BLUETOOTH, enable ? enable_cmd_value : disable_cmd_value, 2);
  this->set_timeout(200, [this]() { this->restart_and_read_all_info(); });
}

void LD2412Component::set_distance_resolution(const std::string &state) {
  this->set_config_mode_(true);
  uint8_t cmd_value[6] = {DISTANCE_RESOLUTION_ENUM_TO_INT.at(state), 0x00, 0x00, 0x00, 0x00, 0x00};
  this->send_command_(CMD_SET_DISTANCE_RESOLUTION, cmd_value, 6);
  this->set_timeout(200, [this]() { this->restart_and_read_all_info(); });
}

void LD2412Component::set_baud_rate(const std::string &state) {
  this->set_config_mode_(true);
  uint8_t cmd_value[2] = {BAUD_RATE_ENUM_TO_INT.at(state), 0x00};
  this->send_command_(CMD_SET_BAUD_RATE, cmd_value, 2);
  this->set_timeout(200, [this]() { this->restart_(); });
}

void LD2412Component::set_mode(const std::string &state) {
  this->set_config_mode_(true);
  uint8_t cmd;
  switch(MODE_ENUM_TO_INT.at(state)){
    case NORMAL_MODE:
      cmd = CMD_DISABLE_ENG;
      break;
    case ENGINEERING_MODE:
      cmd = CMD_ENABLE_ENG;
      break;
    case BACKGROUND_INIT_MODE:
      cmd = CMD_DYNAMIC_BACKGROUND_CORRECTION;
      break;
  }
  this->send_command_(cmd, nullptr, 0);
  this->set_config_mode_(false);
  if(cmd == CMD_DYNAMIC_BACKGROUND_CORRECTION){
    this->dynamic_bakground_correction_active_ = true;
    this->set_timeout(1000, [this]() { this->query_dymanic_background_correction_(); });
  }
}

void LD2412Component::query_dymanic_background_correction_(){
  this->set_config_mode_(true);
  this->send_command_(CMD_QUEY_DYNAMIC_BACKGROUND_CORRECTION, nullptr, 0);
  this->set_config_mode_(false);
}

// void LD2412Component::set_bluetooth_password(const std::string &password) {
//   if (password.length() != 6) {
//     ESP_LOGE(TAG, "set_bluetooth_password(): invalid password length, must be exactly 6 chars '%s'", password.c_str());
//     return;
//   }
//   this->set_config_mode_(true);
//   uint8_t cmd_value[6];
//   std::copy(password.begin(), password.end(), std::begin(cmd_value));
//   this->send_command_(CMD_BT_PASSWORD, cmd_value, 6);
//   this->set_config_mode_(false);
// }

void LD2412Component::set_engineering_mode(bool enable) {
  this->set_config_mode_(true);
  last_engineering_mode_change_millis_ = millis();
  uint8_t cmd = enable ? CMD_ENABLE_ENG : CMD_DISABLE_ENG;
  this->send_command_(cmd, nullptr, 0);
  this->set_config_mode_(false);
}

void LD2412Component::factory_reset() {
  this->set_config_mode_(true);
  this->send_command_(CMD_RESET, nullptr, 0);
#if  defined(USE_SELECT) || defined(USE_NUMBER)
  if (this->baud_rate_select_ != nullptr){
    this->baud_rate_select_->publish_state("115200");
  }
#endif
  this->set_timeout(2000, [this]() { this->restart_and_read_all_info(); });
}

void LD2412Component::restart_() { this->send_command_(CMD_RESTART, nullptr, 0); }

void LD2412Component::query_parameters_() { this->send_command_(CMD_QUERY, nullptr, 0); }
void LD2412Component::get_version_() { this->send_command_(CMD_VERSION, nullptr, 0); }
void LD2412Component::get_mac_() {
  uint8_t cmd_value[2] = {0x01, 0x00};
  this->send_command_(CMD_MAC, cmd_value, 2);
}
void LD2412Component::get_distance_resolution_() { this->send_command_(CMD_QUERY_DISTANCE_RESOLUTION, nullptr, 0); }

// void LD2412Component::get_light_control_() { this->send_command_(CMD_QUERY_LIGHT_CONTROL, nullptr, 0); }

#if  defined(USE_SELECT) || defined(USE_NUMBER)
void LD2412Component::set_basic_config() {
  if (
      !this->min_distance_gate_number_->has_state() || 
      !this->max_distance_gate_number_->has_state() ||
      !this->timeout_number_->has_state() ||
      !this->out_pin_level_select_->has_state()) {
    return;
  }
  uint8_t value[5] = {
    lowbyte(static_cast<int>(this->min_distance_gate_number_->state)),
    lowbyte(static_cast<int>(this->max_distance_gate_number_->state)+1),
    lowbyte(static_cast<int>(this->timeout_number_->state)),
    highbyte(static_cast<int>(this->timeout_number_->state)),
    OUT_PIN_LEVEL_ENUM_TO_INT.at(this->out_pin_level_select_->state)
  };
  // int max_moving_distance_gate_range = static_cast<int>(this->max_move_distance_gate_number_->state);
  // int max_still_distance_gate_range = static_cast<int>(this->max_still_distance_gate_number_->state);
  // int timeout = static_cast<int>(this->timeout_number_->state);
  // uint8_t value[18] = {0x00,
  //                      0x00,
  //                      lowbyte(max_moving_distance_gate_range),
  //                      highbyte(max_moving_distance_gate_range),
  //                      0x00,
  //                      0x00,
  //                      0x01,
  //                      0x00,
  //                      lowbyte(max_still_distance_gate_range),
  //                      highbyte(max_still_distance_gate_range),
  //                      0x00,
  //                      0x00,
  //                      0x02,
  //                      0x00,
  //                      lowbyte(timeout),
  //                      highbyte(timeout),
  //                      0x00,
  //                      0x00};
  this->set_config_mode_(true);
  this->send_command_(CMD_BASIC_CONF, value, 5);
  delay(50);  // NOLINT
  this->set_timeout(200, [this]() { this->restart_and_read_all_info(); });
  this->set_config_mode_(false);
}

void LD2412Component::set_gate_threshold() {
  this->set_config_mode_(true);
  uint8_t value[14];// = {0x00, 0x00, lowbyte(gate),   highbyte(gate),   0x00, 0x00,
                    //   0x01, 0x00, lowbyte(motion), highbyte(motion), 0x00, 0x00,
                    //   0x02, 0x00, lowbyte(still),  highbyte(still),  0x00, 0x00};
  for(int i = 0; i < this->gate_move_threshold_numbers_.size(); i++){
    value[i] = lowbyte(static_cast<int>(this->gate_move_threshold_numbers_[i]->state));
  }
  this->send_command_(CMD_MOTION_GATE_SENS, value, 14);
  delay(50);  // NOLINT
  for(int i = 0; i < this->gate_still_threshold_numbers_.size(); i++){
    value[i] = lowbyte(static_cast<int>(this->gate_still_threshold_numbers_[i]->state));
  }
  this->send_command_(CMD_STATIC_GATE_SENS, value, 14);
  delay(50);  // NOLINT
  this->set_config_mode_(false);
  // this->query_parameters_();
}

void LD2412Component::get_gate_threshold() {
  this->set_config_mode_(true);
  this->send_command_(CMD_QUERY_MOTION_GATE_SENS, nullptr, 0);
  delay(50);  // NOLINT
  this->send_command_(CMD_QUERY_STATIC_GATE_SENS, nullptr, 0);
  delay(50);  // NOLINT
  this->set_config_mode_(false);
}
void LD2412Component::set_gate_still_threshold_number(int gate, number::Number *n) {
  this->gate_still_threshold_numbers_[gate] = n;
}

void LD2412Component::set_gate_move_threshold_number(int gate, number::Number *n) {
  this->gate_move_threshold_numbers_[gate] = n;
}
#endif

void LD2412Component::set_light_out_control() {
// #if  defined(USE_SELECT) || defined(USE_NUMBER)
//   if (this->light_threshold_number_ != nullptr && this->light_threshold_number_->has_state()) {
//     this->light_threshold_ = this->light_threshold_number_->state;
//   }
// #endif
#if  defined(USE_SELECT) || defined(USE_NUMBER)
//   if (this->light_function_select_ != nullptr && this->light_function_select_->has_state()) {
//     this->light_function_ = this->light_function_select_->state;
//   }
  if (this->out_pin_level_select_ != nullptr && this->out_pin_level_select_->has_state()) {
    this->out_pin_level_ = this->out_pin_level_select_->state;
  }
#endif
  if (this->light_function_.empty() || this->out_pin_level_.empty() || this->light_threshold_ < 0) {
    return;
  }
  this->set_config_mode_(true);
  // uint8_t light_function = LIGHT_FUNCTION_ENUM_TO_INT.at(this->light_function_);
  // uint8_t light_threshold = static_cast<uint8_t>(this->light_threshold_);
  // uint8_t out_pin_level = OUT_PIN_LEVEL_ENUM_TO_INT.at(this->out_pin_level_);
  // uint8_t value[4] = {light_function, light_threshold, out_pin_level, 0x00};
  // this->send_command_(CMD_SET_LIGHT_CONTROL, value, 4);
  // delay(50);  // NOLINT
  // this->get_light_control_();
  this->set_timeout(200, [this]() { this->restart_and_read_all_info(); });
}

#ifdef USE_SENSOR
void LD2412Component::set_gate_move_sensor(int gate, sensor::Sensor *s) { this->gate_move_sensors_[gate] = s; }
void LD2412Component::set_gate_still_sensor(int gate, sensor::Sensor *s) { this->gate_still_sensors_[gate] = s; }
#endif

}  // namespace LD2412
}  // namespace esphome
