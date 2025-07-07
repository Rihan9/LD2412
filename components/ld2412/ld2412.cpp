#include "ld2412.h"
#include "esphome/core/application.h"
#include "esphome/core/helpers.h"

#include <utility>
#ifdef USE_NUMBER
#include "esphome/components/number/number.h"
#endif
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif

#define CHECK_BIT(var, pos) (((var) >> (pos)) & 1)
#define highbyte(val) (uint8_t)((val) >> 8)
#define lowbyte(val) (uint8_t)((val) &0xff)

namespace esphome {
namespace ld2412 {

static const char *const TAG = "ld2412";
static const char *const UNKNOWN_MAC = "unknown";
static const char *const VERSION_FMT = "%u.%02X.%02X%02X%02X%02X";

enum BaudRate : uint8_t {
  BAUD_RATE_9600 = 1,
  BAUD_RATE_19200 = 2,
  BAUD_RATE_38400 = 3,
  BAUD_RATE_57600 = 4,
  BAUD_RATE_115200 = 5,
  BAUD_RATE_230400 = 6,
  BAUD_RATE_256000 = 7,
  BAUD_RATE_460800 = 8,
};

enum DistanceResolution : uint8_t {
  DISTANCE_RESOLUTION_0_2 = 0x03,
  DISTANCE_RESOLUTION_0_5 = 0x01,
  DISTANCE_RESOLUTION_0_75 = 0x00,
};

enum LightFunction : uint8_t {
  LIGHT_FUNCTION_OFF = 0x00,
  LIGHT_FUNCTION_BELOW = 0x01,
  LIGHT_FUNCTION_ABOVE = 0x02,
};

enum OutPinLevel : uint8_t {
  OUT_PIN_LEVEL_LOW = 0x01,
  OUT_PIN_LEVEL_HIGH = 0x00,
};

/*
Data Type: 6th byte
Target states: 9th byte
    Moving target distance: 10~11th bytes
    Moving target energy: 12th byte
    Still target distance: 13~14th bytes
    Still target energy: 15th byte
    Detect distance: 16~17th bytes
*/
enum PeriodicData : uint8_t {
  DATA_TYPES = 6,
  TARGET_STATES = 8,
  MOVING_TARGET_LOW = 9,
  MOVING_TARGET_HIGH = 10,
  MOVING_ENERGY = 11,
  STILL_TARGET_LOW = 12,
  STILL_TARGET_HIGH = 13,
  STILL_ENERGY = 14,
  MOVING_SENSOR_START = 17,
  STILL_SENSOR_START = 31,
  LIGHT_SENSOR = 45,
  OUT_PIN_SENSOR = 38,
};

enum PeriodicDataValue : uint8_t {
  HEADER = 0XAA,
  FOOTER = 0x55,
  CHECK = 0x00,
};

enum AckData : uint8_t {
  COMMAND = 6,
  COMMAND_STATUS = 7,
};

// Memory-efficient lookup tables
struct StringToUint8 {
  const char *str;
  const uint8_t value;
};

struct Uint8ToString {
  const uint8_t value;
  const char *str;
};

constexpr StringToUint8 BAUD_RATES_BY_STR[] = {
    {"9600", BAUD_RATE_9600},     {"19200", BAUD_RATE_19200},   {"38400", BAUD_RATE_38400},
    {"57600", BAUD_RATE_57600},   {"115200", BAUD_RATE_115200}, {"230400", BAUD_RATE_230400},
    {"256000", BAUD_RATE_256000}, {"460800", BAUD_RATE_460800},
};

constexpr StringToUint8 DISTANCE_RESOLUTIONS_BY_STR[] = {
    {"0.2m", DISTANCE_RESOLUTION_0_2},
    {"0.5m", DISTANCE_RESOLUTION_0_5},
    {"0.75m", DISTANCE_RESOLUTION_0_75},
};

constexpr Uint8ToString DISTANCE_RESOLUTIONS_BY_UINT[] = {
    {DISTANCE_RESOLUTION_0_2, "0.2m"},
    {DISTANCE_RESOLUTION_0_5, "0.5m"},
    {DISTANCE_RESOLUTION_0_75, "0.75m"},
};

constexpr StringToUint8 LIGHT_FUNCTIONS_BY_STR[] = {
    {"off", LIGHT_FUNCTION_OFF},
    {"below", LIGHT_FUNCTION_BELOW},
    {"above", LIGHT_FUNCTION_ABOVE},
};

constexpr Uint8ToString LIGHT_FUNCTIONS_BY_UINT[] = {
    {LIGHT_FUNCTION_OFF, "off"},
    {LIGHT_FUNCTION_BELOW, "below"},
    {LIGHT_FUNCTION_ABOVE, "above"},
};

constexpr StringToUint8 OUT_PIN_LEVELS_BY_STR[] = {
    {"low", OUT_PIN_LEVEL_LOW},
    {"high", OUT_PIN_LEVEL_HIGH},
};

constexpr Uint8ToString OUT_PIN_LEVELS_BY_UINT[] = {
    {OUT_PIN_LEVEL_LOW, "low"},
    {OUT_PIN_LEVEL_HIGH, "high"},
};

// Helper functions for lookups
template<size_t N> uint8_t find_uint8(const StringToUint8 (&arr)[N], const std::string &str) {
  for (const auto &entry : arr) {
    if (str == entry.str) {
      return entry.value;
    }
  }
  return 0xFF;  // Not found
}

template<size_t N> const char *find_str(const Uint8ToString (&arr)[N], uint8_t value) {
  for (const auto &entry : arr) {
    if (value == entry.value) {
      return entry.str;
    }
  }
  return "";  // Not found
}

static const uint8_t DEFAULT_PRESENCE_TIMEOUT = 5;  // Default used when number component is not defined
// Commands
static const uint8_t CMD_ENABLE_CONF = 0xFF;
static const uint8_t CMD_DISABLE_CONF = 0xFE;
static const uint8_t CMD_ENABLE_ENG = 0x62;
static const uint8_t CMD_DISABLE_ENG = 0x63;
static const uint8_t CMD_QUERY_BASIC_CONF = 0x12;
static const uint8_t CMD_BASIC_CONF = 0x02;
static const uint8_t CMD_QUERY_VERSION = 0xA0;
static const uint8_t CMD_QUERY_DISTANCE_RESOLUTION = 0x11;
static const uint8_t CMD_SET_DISTANCE_RESOLUTION = 0x01;
static const uint8_t CMD_QUERY_LIGHT_CONTROL = 0x1C;
static const uint8_t CMD_SET_LIGHT_CONTROL = 0x0C;
static const uint8_t CMD_SET_BAUD_RATE = 0xA1;
static const uint8_t CMD_QUERY_MAC_ADDRESS = 0xA5;
static const uint8_t CMD_FACTORY_RESET = 0xA2;
static const uint8_t CMD_RESTART = 0xA3;
static const uint8_t CMD_BLUETOOTH = 0xA4;
static const uint8_t CMD_DYNAMIC_BACKGROUND_CORRECTION = 0x0B;
static const uint8_t CMD_QUERY_DYNAMIC_BACKGROUND_CORRECTION = 0x1B;
static const uint8_t CMD_MOTION_GATE_SENS = 0x03;
static const uint8_t CMD_QUERY_MOTION_GATE_SENS = 0x13;
static const uint8_t CMD_STATIC_GATE_SENS = 0x04;
static const uint8_t CMD_QUERY_STATIC_GATE_SENS = 0x14;
static const uint8_t CMD_NONE = 0x00;
// Commands values
static const uint8_t CMD_MAX_MOVE_VALUE = 0x00;
static const uint8_t CMD_MAX_STILL_VALUE = 0x01;
static const uint8_t CMD_DURATION_VALUE = 0x02;
// Header & Footer size
static const uint8_t HEADER_FOOTER_SIZE = 4;
// Command Header & Footer
static const uint8_t CMD_FRAME_HEADER[HEADER_FOOTER_SIZE] = {0xFD, 0xFC, 0xFB, 0xFA};
static const uint8_t CMD_FRAME_FOOTER[HEADER_FOOTER_SIZE] = {0x04, 0x03, 0x02, 0x01};
// Data Header & Footer
static const uint8_t DATA_FRAME_HEADER[HEADER_FOOTER_SIZE] = {0xF4, 0xF3, 0xF2, 0xF1};
static const uint8_t DATA_FRAME_FOOTER[HEADER_FOOTER_SIZE] = {0xF8, 0xF7, 0xF6, 0xF5};
// MAC address the module uses when Bluetooth is disabled
static const uint8_t NO_MAC[] = {0x08, 0x05, 0x04, 0x03, 0x02, 0x01};

static inline int two_byte_to_int(char firstbyte, char secondbyte) { return (int16_t) (secondbyte << 8) + firstbyte; }

static bool validate_header_footer(const uint8_t *header_footer, const uint8_t *buffer) {
  for (uint8_t i = 0; i < HEADER_FOOTER_SIZE; i++) {
    if (header_footer[i] != buffer[i]) {
      return false;  // Mismatch in header/footer
    }
  }
  return true;  // Valid header/footer
}

void LD2412Component::dump_config() {
  std::string mac_str =
      mac_address_is_valid(this->mac_address_) ? format_mac_address_pretty(this->mac_address_) : UNKNOWN_MAC;
  std::string version = str_sprintf(VERSION_FMT, this->version_[1], this->version_[0], this->version_[5],
                                    this->version_[4], this->version_[3], this->version_[2]);
  ESP_LOGCONFIG(TAG,
                "LD2412:\n"
                "  Firmware version: %s\n"
                "  MAC address: %s\n"
                "  Throttle: %u ms",
                version.c_str(), mac_str.c_str(), this->throttle_);
#ifdef USE_BINARY_SENSOR
  LOG_BINARY_SENSOR("  ", "DynamicBackgroundCorrectionStatus",
                    this->dynamic_background_correction_status_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "OutPinPresenceStatusBinarySensor", this->out_pin_presence_status_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "MovingTargetBinarySensor", this->moving_target_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "StillTargetBinarySensor", this->still_target_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "TargetBinarySensor", this->target_binary_sensor_);
#endif
#ifdef USE_SENSOR
  LOG_SENSOR("  ", "LightSensor", this->light_sensor_);
  LOG_SENSOR("  ", "DetectionDistanceSensor", this->detection_distance_sensor_);
  LOG_SENSOR("  ", "MovingTargetDistanceSensor", this->moving_target_distance_sensor_);
  LOG_SENSOR("  ", "MovingTargetEnergySensor", this->moving_target_energy_sensor_);
  LOG_SENSOR("  ", "StillTargetDistanceSensor", this->still_target_distance_sensor_);
  LOG_SENSOR("  ", "StillTargetEnergySensor", this->still_target_energy_sensor_);
  for (sensor::Sensor *s : this->gate_still_sensors_) {
    LOG_SENSOR("  ", "NthGateStillSesnsor", s);
  }
  for (sensor::Sensor *s : this->gate_move_sensors_) {
    LOG_SENSOR("  ", "NthGateMoveSesnsor", s);
  }
#endif
#ifdef USE_TEXT_SENSOR
  LOG_TEXT_SENSOR("  ", "MacTextSensor", this->mac_text_sensor_);
  LOG_TEXT_SENSOR("  ", "VersionTextSensor", this->version_text_sensor_);
#endif
#ifdef USE_NUMBER
  LOG_NUMBER("  ", "LightThresholdNumber", this->light_threshold_number_);
  LOG_NUMBER("  ", "MaxDistanceGateNumber", this->max_distance_gate_number_);
  LOG_NUMBER("  ", "MinDistanceGateNumber", this->min_distance_gate_number_);
  LOG_NUMBER("  ", "TimeoutNumber", this->timeout_number_);
  for (number::Number *n : this->gate_move_threshold_numbers_) {
    LOG_NUMBER("  ", "Move Thresholds Number", n);
  }
  for (number::Number *n : this->gate_still_threshold_numbers_) {
    LOG_NUMBER("  ", "Still Thresholds Number", n);
  }
#endif
#ifdef USE_SELECT
  LOG_SELECT("  ", "BaudRateSelect", this->baud_rate_select_);
  LOG_SELECT("  ", "DistanceResolutionSelect", this->distance_resolution_select_);
  LOG_SELECT("  ", "LightFunctionSelect", this->light_function_select_);
  LOG_SELECT("  ", "OutPinLevelSelect", this->out_pin_level_select_);
#endif
#ifdef USE_SWITCH
  LOG_SWITCH("  ", "BluetoothSwitch", this->bluetooth_switch_);
  LOG_SWITCH("  ", "EngineeringModeSwitch", this->engineering_mode_switch_);
#endif
#ifdef USE_BUTTON
  LOG_BUTTON("  ", "FactoryResetButton", this->factory_reset_button_);
  LOG_BUTTON("  ", "QueryButton", this->query_button_);
  LOG_BUTTON("  ", "RestartButton", this->restart_button_);
  LOG_BUTTON("  ", "StartDynamicBackgroundCorrectionButton", this->start_dynamic_background_correction_button_);
#endif
}

void LD2412Component::setup() {
  ESP_LOGCONFIG(TAG, "Running setup");
  this->read_all_info();
}

void LD2412Component::read_all_info() {
  this->set_config_mode_(true);
  this->get_version_();
  delay(10);  // NOLINT
  this->get_mac_();
  delay(10);  // NOLINT
  this->get_distance_resolution_();
  delay(10);  // NOLINT
  this->query_parameters_();
  delay(10);  // NOLINT
  this->query_dynamic_background_correction_();
  delay(10);  // NOLINT
  this->query_light_control_();
  delay(10);  // NOLINT
#ifdef USE_NUMBER
  this->get_gate_threshold();
  delay(10);  // NOLINT
#endif
  this->set_config_mode_(false);
#ifdef USE_SELECT
  const auto baud_rate = std::to_string(this->parent_->get_baud_rate());
  if (this->baud_rate_select_ != nullptr) {
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
  while (this->available()) {
    this->readline_(this->read());
  }
}

void LD2412Component::send_command_(uint8_t command, const uint8_t *command_value, uint8_t command_value_len) {
  ESP_LOGV(TAG, "Sending COMMAND %02X", command);
  // frame header bytes
  this->write_array(CMD_FRAME_HEADER, HEADER_FOOTER_SIZE);
  // length bytes
  uint8_t len = 2;
  if (command_value != nullptr) {
    len += command_value_len;
  }
  uint8_t len_cmd[] = {lowbyte(len), highbyte(len), command, 0x00};
  this->write_array(len_cmd, sizeof(len_cmd));

  // command value bytes
  if (command_value != nullptr) {
    for (uint8_t i = 0; i < command_value_len; i++) {
      this->write_byte(command_value[i]);
    }
  }
  // frame footer bytes
  this->write_array(CMD_FRAME_FOOTER, HEADER_FOOTER_SIZE);
  // FIXME to remove
  delay(50);  // NOLINT
}

void LD2412Component::handle_periodic_data_() {
  // 4 frame header bytes + 2 length bytes + 1 data end byte + 1 crc byte + 4 frame footer bytes
  // data header=0xAA, data footer=0x55, crc=0x00
  if (this->buffer_pos_ < 12 || !ld2412::validate_header_footer(DATA_FRAME_HEADER, this->buffer_data_) ||
      this->buffer_data_[7] != HEADER || this->buffer_data_[this->buffer_pos_ - 6] != FOOTER) {
    return;
  }

  /*
    Reduce data update rate to reduce home assistant database growth
  */
  if (App.get_loop_component_start_time() - this->last_periodic_millis_ < this->throttle_) {
    return;
  }
  this->last_periodic_millis_ = App.get_loop_component_start_time();

  /*
    Data Type: 7th
    0x01: Engineering mode
    0x02: Normal mode
  */
  bool engineering_mode = this->buffer_data_[DATA_TYPES] == 0x01;
#ifdef USE_SWITCH
  if (this->engineering_mode_switch_ != nullptr) {
    this->engineering_mode_switch_->publish_state(engineering_mode);
  }
#endif

#ifdef USE_BINARY_SENSOR
  /*
    Target states: 9th
    0x00 = No target
    0x01 = Moving targets
    0x02 = Still targets
    0x03 = Moving+Still targets
  */
  char target_state = this->buffer_data_[TARGET_STATES];
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
    int new_moving_target_distance =
        target_state != 0x00
            ? ld2412::two_byte_to_int(this->buffer_data_[MOVING_TARGET_LOW], this->buffer_data_[MOVING_TARGET_HIGH])
            : 0;
    if (this->moving_target_distance_sensor_->get_state() != new_moving_target_distance) {
      this->moving_target_distance_sensor_->publish_state(new_moving_target_distance);
    }
  }
  if (this->moving_target_energy_sensor_ != nullptr) {
    int new_moving_target_energy = target_state != 0x00 ? this->buffer_data_[MOVING_ENERGY] : 0;
    if (this->moving_target_energy_sensor_->get_state() != new_moving_target_energy) {
      this->moving_target_energy_sensor_->publish_state(new_moving_target_energy);
    }
  }
  if (this->still_target_distance_sensor_ != nullptr) {
    int new_still_target_distance =
        target_state != 0x00
            ? ld2412::two_byte_to_int(this->buffer_data_[STILL_TARGET_LOW], this->buffer_data_[STILL_TARGET_HIGH])
            : 0;
    if (this->still_target_distance_sensor_->get_state() != new_still_target_distance) {
      this->still_target_distance_sensor_->publish_state(new_still_target_distance);
    }
  }
  if (this->still_target_energy_sensor_ != nullptr) {
    int new_still_target_energy = target_state != 0x00 ? this->buffer_data_[STILL_ENERGY] : 0;
    if (this->still_target_energy_sensor_->get_state() != new_still_target_energy) {
      this->still_target_energy_sensor_->publish_state(new_still_target_energy);
    }
  }
  if (this->detection_distance_sensor_ != nullptr) {
    int new_detect_distance = 0;
    if (target_state != 0x00 && CHECK_BIT(target_state, 0)) {
      new_detect_distance =
          ld2412::two_byte_to_int(this->buffer_data_[MOVING_TARGET_LOW], this->buffer_data_[MOVING_TARGET_HIGH]);
    } else if (target_state != 0x00) {
      new_detect_distance =
          ld2412::two_byte_to_int(this->buffer_data_[STILL_TARGET_LOW], this->buffer_data_[STILL_TARGET_HIGH]);
    }
    if (this->detection_distance_sensor_->get_state() != new_detect_distance) {
      this->detection_distance_sensor_->publish_state(new_detect_distance);
    }
  }
  if (engineering_mode) {
    /*
      Moving distance range: 18th byte
      Still distance range: 19th byte
      Moving energy: 20~28th bytes
    */
    for (std::vector<sensor::Sensor *>::size_type i = 0; i != this->gate_move_sensors_.size(); i++) {
      sensor::Sensor *s = this->gate_move_sensors_[i];
      if (s != nullptr) {
        s->publish_state(this->buffer_data_[MOVING_SENSOR_START + i]);
      }
    }
    /*
      Still energy: 29~37th bytes
    */
    for (std::vector<sensor::Sensor *>::size_type i = 0; i != this->gate_still_sensors_.size(); i++) {
      sensor::Sensor *s = this->gate_still_sensors_[i];
      if (s != nullptr) {
        s->publish_state(this->buffer_data_[STILL_SENSOR_START + i]);
      }
    }
    /*
      Light sensor: 38th bytes
    */
    if (this->light_sensor_ != nullptr) {
      int new_light_sensor = (this->buffer_data_[LIGHT_SENSOR] * 100) / 255;
      if (this->light_sensor_->get_state() != new_light_sensor) {
        this->light_sensor_->publish_state(new_light_sensor);
      }
    }
  } else {
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
#endif
#ifdef USE_BINARY_SENSOR
  if (engineering_mode) {
    if (this->out_pin_presence_status_binary_sensor_ != nullptr) {
      this->out_pin_presence_status_binary_sensor_->publish_state(this->buffer_data_[OUT_PIN_SENSOR] == 0x01);
    }
  } else {
    if (this->out_pin_presence_status_binary_sensor_ != nullptr) {
      this->out_pin_presence_status_binary_sensor_->publish_state(false);
    }
  }
#endif
  // the radar module won't tell us when it's done, so we just have to keep polling...
  if (this->dynamic_background_correction_active_) {
    this->set_config_mode_(true);
    this->query_dynamic_background_correction_();
    this->set_config_mode_(false);
  }
}

#ifdef USE_NUMBER
std::function<void(void)> set_number_value(number::Number *n, float value) {
  if (n != nullptr && (!n->has_state() || n->state != value)) {
    n->state = value;
    return [n, value]() { n->publish_state(value); };
  }
  return []() {};
}
#endif

bool LD2412Component::handle_ack_data_() {
  ESP_LOGV(TAG, "Handling ACK DATA for COMMAND %02X", this->buffer_data_[COMMAND]);
  if (this->buffer_pos_ < 10) {
    ESP_LOGW(TAG, "Invalid length");
    return true;
  }
  if (!ld2412::validate_header_footer(CMD_FRAME_HEADER, this->buffer_data_)) {
    ESP_LOGW(TAG, "Invalid header: %s", format_hex_pretty(this->buffer_data_, HEADER_FOOTER_SIZE).c_str());
    return true;
  }
  if (this->buffer_data_[COMMAND_STATUS] != 0x01) {
    ESP_LOGW(TAG, "Invalid status");
    return true;
  }
  if (ld2412::two_byte_to_int(this->buffer_data_[8], this->buffer_data_[9]) != 0x00) {
    ESP_LOGW(TAG, "Invalid command: %02X, %02X", this->buffer_data_[8], this->buffer_data_[9]);
    return true;
  }

  switch (this->buffer_data_[COMMAND]) {
    case CMD_ENABLE_CONF:
      ESP_LOGV(TAG, "Enable conf");
      break;

    case CMD_DISABLE_CONF:
      ESP_LOGV(TAG, "Disabled conf");
      break;

    case CMD_SET_BAUD_RATE:
      ESP_LOGV(TAG, "Baud rate change");
#ifdef USE_SELECT
      if (this->baud_rate_select_ != nullptr) {
        ESP_LOGW(TAG, "Change baud rate to %s and reinstall", this->baud_rate_select_->state.c_str());
      }
#endif
      break;

    case CMD_QUERY_VERSION: {
      std::memcpy(this->version_, &this->buffer_data_[12], sizeof(this->version_));
      std::string version = str_sprintf(VERSION_FMT, this->version_[1], this->version_[0], this->version_[5],
                                        this->version_[4], this->version_[3], this->version_[2]);
      ESP_LOGV(TAG, "Firmware version: %s", version.c_str());
#ifdef USE_TEXT_SENSOR
      if (this->version_text_sensor_ != nullptr) {
        this->version_text_sensor_->publish_state(version);
      }
#endif
      break;
    }
    case CMD_QUERY_DISTANCE_RESOLUTION: {
      const auto *distance_resolution = find_str(DISTANCE_RESOLUTIONS_BY_UINT, this->buffer_data_[10]);
      ESP_LOGV(TAG, "Distance resolution: %s", distance_resolution);
#ifdef USE_SELECT
      if (this->distance_resolution_select_ != nullptr) {
        this->distance_resolution_select_->publish_state(distance_resolution);
      }
#endif
      break;
    }

    case CMD_QUERY_LIGHT_CONTROL: {
      this->light_function_ = this->buffer_data_[10];
      this->light_threshold_ = this->buffer_data_[11];
      const auto *light_function_str = find_str(LIGHT_FUNCTIONS_BY_UINT, this->light_function_);
      ESP_LOGV(TAG,
               "Light function is: %s\n"
               "Light threshold is: %u",
               light_function_str, this->light_threshold_);
#ifdef USE_SELECT
      if (this->light_function_select_ != nullptr) {
        this->light_function_select_->publish_state(light_function_str);
      }
#endif
#ifdef USE_NUMBER
      if (this->light_threshold_number_ != nullptr) {
        this->light_threshold_number_->publish_state(static_cast<float>(this->light_threshold_));
      }
#endif
      break;
    }

    case CMD_QUERY_MAC_ADDRESS: {
      if (this->buffer_pos_ < 20) {
        return false;
      }

      this->bluetooth_on_ = std::memcmp(&this->buffer_data_[10], NO_MAC, sizeof(NO_MAC)) != 0;
      if (this->bluetooth_on_) {
        std::memcpy(this->mac_address_, &this->buffer_data_[10], sizeof(this->mac_address_));
      }

      std::string mac_str =
          mac_address_is_valid(this->mac_address_) ? format_mac_address_pretty(this->mac_address_) : UNKNOWN_MAC;
      ESP_LOGV(TAG, "MAC address: %s", mac_str.c_str());
#ifdef USE_TEXT_SENSOR
      if (this->mac_text_sensor_ != nullptr) {
        this->mac_text_sensor_->publish_state(mac_str);
      }
#endif
#ifdef USE_SWITCH
      if (this->bluetooth_switch_ != nullptr) {
        this->bluetooth_switch_->publish_state(this->bluetooth_on_);
      }
#endif
      break;
    }

    case CMD_SET_DISTANCE_RESOLUTION:
      ESP_LOGV(TAG, "Handled set distance resolution command");
      break;

    case CMD_QUERY_DYNAMIC_BACKGROUND_CORRECTION: {
      ESP_LOGV(TAG, "Handled query dynamic background correction");
      bool dynamic_background_correction_active = (this->buffer_data_[10] != 0x00);
#ifdef USE_BINARY_SENSOR
      if (this->dynamic_background_correction_status_binary_sensor_ != nullptr) {
        this->dynamic_background_correction_status_binary_sensor_->publish_state(dynamic_background_correction_active);
      }
#endif
      this->dynamic_background_correction_active_ = dynamic_background_correction_active;
      break;
    }

    case CMD_BLUETOOTH:
      ESP_LOGV(TAG, "Handled bluetooth command");
      break;

    case CMD_SET_LIGHT_CONTROL:
      ESP_LOGV(TAG, "Handled set light control command");
      break;

    case CMD_QUERY_MOTION_GATE_SENS: {
#ifdef USE_NUMBER
      std::vector<std::function<void(void)>> updates;
      updates.reserve(this->gate_still_threshold_numbers_.size());
      for (size_t i = 0; i < this->gate_still_threshold_numbers_.size(); i++) {
        updates.push_back(set_number_value(this->gate_move_threshold_numbers_[i], this->buffer_data_[10 + i]));
      }
      for (auto &update : updates) {
        update();
      }
#endif
      break;
    }

    case CMD_QUERY_STATIC_GATE_SENS: {
#ifdef USE_NUMBER
      std::vector<std::function<void(void)>> updates;
      updates.reserve(this->gate_still_threshold_numbers_.size());
      for (size_t i = 0; i < this->gate_still_threshold_numbers_.size(); i++) {
        updates.push_back(set_number_value(this->gate_still_threshold_numbers_[i], this->buffer_data_[10 + i]));
      }
      for (auto &update : updates) {
        update();
      }
#endif
      break;
    }

    case CMD_QUERY_BASIC_CONF:  // Query parameters response
    {
#ifdef USE_NUMBER
      /*
        Moving distance range: 9th byte
        Still distance range: 10th byte
      */
      std::vector<std::function<void(void)>> updates;
      updates.push_back(set_number_value(this->min_distance_gate_number_, this->buffer_data_[10]));
      updates.push_back(set_number_value(this->max_distance_gate_number_, this->buffer_data_[11] - 1));
      ESP_LOGV(TAG, "min_distance_gate_number_: %u, max_distance_gate_number_ %u", this->buffer_data_[10],
               this->buffer_data_[11]);
      /*
        None Duration: 11~12th bytes
      */
      updates.push_back(set_number_value(this->timeout_number_,
                                         ld2412::two_byte_to_int(this->buffer_data_[12], this->buffer_data_[13])));
      ESP_LOGV(TAG, "timeout_number_: %u", ld2412::two_byte_to_int(this->buffer_data_[12], this->buffer_data_[13]));
      /*
        Output pin configuration: 13th bytes
      */
      this->out_pin_level_ = this->buffer_data_[14];
#ifdef USE_SELECT
      const auto *out_pin_level_str = find_str(OUT_PIN_LEVELS_BY_UINT, this->out_pin_level_);
      if (this->out_pin_level_select_ != nullptr) {
        this->out_pin_level_select_->publish_state(out_pin_level_str);
      }
#endif
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

void LD2412Component::readline_(int readch) {
  if (readch < 0) {
    return;  // No data available
  }
  if (this->buffer_pos_ == 0 && readch != DATA_FRAME_HEADER[0] && readch != CMD_FRAME_HEADER[0]) {
    return;
  }
  if (this->buffer_pos_ == 1 && readch != DATA_FRAME_HEADER[1] && readch != CMD_FRAME_HEADER[1]) {
    this->buffer_pos_ = 0;
    return;
  }
  if (this->buffer_pos_ == 2 && readch != DATA_FRAME_HEADER[2] && readch != CMD_FRAME_HEADER[2]) {
    this->buffer_pos_ = 0;
    return;
  }
  if (this->buffer_pos_ == 3 && readch != DATA_FRAME_HEADER[3] && readch != CMD_FRAME_HEADER[3]) {
    this->buffer_pos_ = 0;
    return;
  }
  if (this->buffer_pos_ < MAX_LINE_LENGTH - 1) {
    this->buffer_data_[this->buffer_pos_++] = readch;
    this->buffer_data_[this->buffer_pos_] = 0;
  } else {
    // We should never get here, but just in case...
    ESP_LOGW(TAG, "Max command length exceeded; ignoring");
    this->buffer_pos_ = 0;
  }
  if (this->buffer_pos_ < 4) {
    return;  // Not enough data to process yet
  }
  if (this->buffer_data_[this->buffer_pos_ - 4] == DATA_FRAME_FOOTER[0] &&
      this->buffer_data_[this->buffer_pos_ - 3] == DATA_FRAME_FOOTER[1] &&
      this->buffer_data_[this->buffer_pos_ - 2] == DATA_FRAME_FOOTER[2] &&
      this->buffer_data_[this->buffer_pos_ - 1] == DATA_FRAME_FOOTER[3]) {
    ESP_LOGV(TAG, "Handling Periodic Data: %s", format_hex_pretty(this->buffer_data_, this->buffer_pos_).c_str());
    this->handle_periodic_data_();
    this->buffer_pos_ = 0;  // Reset position index for next message
  } else if (this->buffer_data_[this->buffer_pos_ - 4] == CMD_FRAME_FOOTER[0] &&
             this->buffer_data_[this->buffer_pos_ - 3] == CMD_FRAME_FOOTER[1] &&
             this->buffer_data_[this->buffer_pos_ - 2] == CMD_FRAME_FOOTER[2] &&
             this->buffer_data_[this->buffer_pos_ - 1] == CMD_FRAME_FOOTER[3]) {
    ESP_LOGV(TAG, "Handling Ack Data: %s", format_hex_pretty(this->buffer_data_, this->buffer_pos_).c_str());
    if (this->handle_ack_data_()) {
      this->buffer_pos_ = 0;  // Reset position index for next message
    } else {
      ESP_LOGV(TAG, "Ack Data incomplete");
    }
  }
}

void LD2412Component::set_config_mode_(bool enable) {
  const uint8_t cmd = enable ? CMD_ENABLE_CONF : CMD_DISABLE_CONF;
  const uint8_t cmd_value[2] = {0x01, 0x00};
  this->send_command_(cmd, enable ? cmd_value : nullptr, sizeof(cmd_value));
}

void LD2412Component::set_bluetooth(bool enable) {
  this->set_config_mode_(true);
  const uint8_t cmd_value[2] = {enable ? (uint8_t) 0x01 : (uint8_t) 0x00, 0x00};
  this->send_command_(CMD_BLUETOOTH, cmd_value, sizeof(cmd_value));
  this->set_timeout(200, [this]() { this->restart_and_read_all_info(); });
}

void LD2412Component::set_distance_resolution(const std::string &state) {
  this->set_config_mode_(true);
  const uint8_t cmd_value[6] = {find_uint8(DISTANCE_RESOLUTIONS_BY_STR, state), 0x00, 0x00, 0x00, 0x00, 0x00};
  this->send_command_(CMD_SET_DISTANCE_RESOLUTION, cmd_value, sizeof(cmd_value));
  this->set_timeout(200, [this]() { this->restart_and_read_all_info(); });
}

void LD2412Component::set_baud_rate(const std::string &state) {
  this->set_config_mode_(true);
  const uint8_t cmd_value[2] = {find_uint8(BAUD_RATES_BY_STR, state), 0x00};
  this->send_command_(CMD_SET_BAUD_RATE, cmd_value, sizeof(cmd_value));
  this->set_timeout(200, [this]() { this->restart_(); });
}

void LD2412Component::query_dynamic_background_correction_() {
  this->send_command_(CMD_QUERY_DYNAMIC_BACKGROUND_CORRECTION, nullptr, 0);
}

void LD2412Component::start_dynamic_background_correction() {
  if (this->dynamic_background_correction_active_) {
    return;  // Already in progress
  }
#ifdef USE_BINARY_SENSOR
  if (this->dynamic_background_correction_status_binary_sensor_ != nullptr) {
    this->dynamic_background_correction_status_binary_sensor_->publish_state(true);
  }
#endif
  this->dynamic_background_correction_active_ = true;
  this->set_config_mode_(true);
  this->send_command_(CMD_DYNAMIC_BACKGROUND_CORRECTION, nullptr, 0);
  this->set_config_mode_(false);
}

void LD2412Component::set_engineering_mode(bool enable) {
  const uint8_t cmd = enable ? CMD_ENABLE_ENG : CMD_DISABLE_ENG;
  this->set_config_mode_(true);
  this->send_command_(cmd, nullptr, 0);
  this->set_config_mode_(false);
}

void LD2412Component::factory_reset() {
  this->set_config_mode_(true);
  this->send_command_(CMD_FACTORY_RESET, nullptr, 0);
  this->set_timeout(2000, [this]() { this->restart_and_read_all_info(); });
}

void LD2412Component::restart_() { this->send_command_(CMD_RESTART, nullptr, 0); }

void LD2412Component::query_parameters_() { this->send_command_(CMD_QUERY_BASIC_CONF, nullptr, 0); }

void LD2412Component::get_version_() { this->send_command_(CMD_QUERY_VERSION, nullptr, 0); }

void LD2412Component::get_mac_() {
  const uint8_t cmd_value[2] = {0x01, 0x00};
  this->send_command_(CMD_QUERY_MAC_ADDRESS, cmd_value, sizeof(cmd_value));
}

void LD2412Component::get_distance_resolution_() { this->send_command_(CMD_QUERY_DISTANCE_RESOLUTION, nullptr, 0); }

void LD2412Component::query_light_control_() { this->send_command_(CMD_QUERY_LIGHT_CONTROL, nullptr, 0); }

void LD2412Component::set_basic_config() {
#ifdef USE_NUMBER
  if (!this->min_distance_gate_number_->has_state() || !this->max_distance_gate_number_->has_state() ||
      !this->timeout_number_->has_state()) {
    return;
  }
#endif
#ifdef USE_SELECT
  if (!this->out_pin_level_select_->has_state()) {
    return;
  }
#endif

  uint8_t value[5] = {
#ifdef USE_NUMBER
      lowbyte(static_cast<int>(this->min_distance_gate_number_->state)),
      lowbyte(static_cast<int>(this->max_distance_gate_number_->state) + 1),
      lowbyte(static_cast<int>(this->timeout_number_->state)),
      highbyte(static_cast<int>(this->timeout_number_->state)),
#else
      1,    TOTAL_GATES, DEFAULT_PRESENCE_TIMEOUT, 0,
#endif
#ifdef USE_SELECT
      find_uint8(OUT_PIN_LEVELS_BY_STR, this->out_pin_level_select_->state),
#else
      0x01,  // Default value if not using select
#endif
  };
  this->set_config_mode_(true);
  this->send_command_(CMD_BASIC_CONF, value, sizeof(value));
  delay(50);  // NOLINT
  this->set_config_mode_(false);
}

#ifdef USE_NUMBER
void LD2412Component::set_gate_threshold() {
  if (this->gate_move_threshold_numbers_.empty() && this->gate_still_threshold_numbers_.empty()) {
    return;  // No gate threshold numbers set; nothing to do here
  }
  uint8_t value[TOTAL_GATES] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  this->set_config_mode_(true);
  if (!this->gate_move_threshold_numbers_.empty()) {
    for (size_t i = 0; i < this->gate_move_threshold_numbers_.size(); i++) {
      value[i] = lowbyte(static_cast<int>(this->gate_move_threshold_numbers_[i]->state));
    }
    this->send_command_(CMD_MOTION_GATE_SENS, value, sizeof(value));
    delay(50);  // NOLINT
  }
  if (!this->gate_still_threshold_numbers_.empty()) {
    for (size_t i = 0; i < this->gate_still_threshold_numbers_.size(); i++) {
      value[i] = lowbyte(static_cast<int>(this->gate_still_threshold_numbers_[i]->state));
    }
    this->send_command_(CMD_STATIC_GATE_SENS, value, sizeof(value));
    delay(50);  // NOLINT
  }
  this->set_config_mode_(false);
}

void LD2412Component::get_gate_threshold() {
  this->send_command_(CMD_QUERY_MOTION_GATE_SENS, nullptr, 0);
  delay(50);  // NOLINT
  this->send_command_(CMD_QUERY_STATIC_GATE_SENS, nullptr, 0);
  delay(50);  // NOLINT
}

void LD2412Component::set_gate_still_threshold_number(uint8_t gate, number::Number *n) {
  this->gate_still_threshold_numbers_[gate] = n;
}

void LD2412Component::set_gate_move_threshold_number(uint8_t gate, number::Number *n) {
  this->gate_move_threshold_numbers_[gate] = n;
}
#endif

void LD2412Component::set_light_out_control() {
#ifdef USE_NUMBER
  if (this->light_threshold_number_ != nullptr && this->light_threshold_number_->has_state()) {
    this->light_threshold_ = static_cast<uint8_t>(this->light_threshold_number_->state);
  }
#endif
#ifdef USE_SELECT
  if (this->light_function_select_ != nullptr && this->light_function_select_->has_state()) {
    this->light_function_ = find_uint8(LIGHT_FUNCTIONS_BY_STR, this->light_function_select_->state);
  }
#endif
  uint8_t value[2] = {this->light_function_, this->light_threshold_};
  this->set_config_mode_(true);
  this->send_command_(CMD_SET_LIGHT_CONTROL, value, sizeof(value));
  delay(50);  // NOLINT
  this->query_light_control_();
  this->set_timeout(200, [this]() { this->restart_and_read_all_info(); });
}

#ifdef USE_SENSOR
void LD2412Component::set_gate_move_sensor(uint8_t gate, sensor::Sensor *s) { this->gate_move_sensors_[gate] = s; }
void LD2412Component::set_gate_still_sensor(uint8_t gate, sensor::Sensor *s) { this->gate_still_sensors_[gate] = s; }
#endif

}  // namespace ld2412
}  // namespace esphome
