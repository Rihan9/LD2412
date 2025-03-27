#pragma once
#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_NUMBER
#include "esphome/components/number/number.h"
#endif
#ifdef USE_SWITCH
#include "esphome/components/switch/switch.h"
#endif
#ifdef USE_BUTTON
#include "esphome/components/button/button.h"
#endif
#ifdef USE_SELECT
#include "esphome/components/select/select.h"
#endif
#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif
#include "esphome/components/uart/uart.h"
#include "esphome/core/automation.h"
#include "esphome/core/helpers.h"

#include <map>

namespace esphome {
namespace LD2412 {

#define CHECK_BIT(var, pos) (((var) >> (pos)) & 1)

// Commands
static const uint8_t CMD_ENABLE_CONF = 0x00FF;
static const uint8_t CMD_DISABLE_CONF = 0x00FE;
static const uint8_t CMD_ENABLE_ENG = 0x0062;
static const uint8_t CMD_DISABLE_ENG = 0x0063;
static const uint8_t CMD_MAXDIST_DURATION = 0x0060;
static const uint8_t CMD_QUERY = 0x0012;
static const uint8_t CMD_BASIC_CONF = 0x0002;
static const uint8_t CMD_GATE_SENS = 0x0064;
static const uint8_t CMD_VERSION = 0x00A0;
static const uint8_t CMD_QUERY_DISTANCE_RESOLUTION = 0x0011;
static const uint8_t CMD_SET_DISTANCE_RESOLUTION = 0x0001;
static const uint8_t CMD_QUERY_LIGHT_CONTROL = 0x00AE;
static const uint8_t CMD_SET_LIGHT_CONTROL = 0x00AD;
static const uint8_t CMD_SET_BAUD_RATE = 0x00A1;
static const uint8_t CMD_BT_PASSWORD = 0x00A9;
static const uint8_t CMD_MAC = 0x00A5;
static const uint8_t CMD_RESET = 0x00A2;
static const uint8_t CMD_RESTART = 0x00A3;
static const uint8_t CMD_BLUETOOTH = 0x00A4;
static const uint8_t CMD_DYNAMIC_BACKGROUND_CORRECTION = 0x000B;
static const uint8_t CMD_QUEY_DYNAMIC_BACKGROUND_CORRECTION = 0x001B;
static const uint8_t CMD_MOTION_GATE_SENS = 0x0003;
static const uint8_t CMD_QUERY_MOTION_GATE_SENS = 0x0013;
static const uint8_t CMD_STATIC_GATE_SENS = 0x0004;
static const uint8_t CMD_QUERY_STATIC_GATE_SENS = 0x0014;
static const uint8_t CMD_NONE = 0x0000;

enum BaudRateStructure : uint8_t {
  BAUD_RATE_9600 = 1,
  BAUD_RATE_19200 = 2,
  BAUD_RATE_38400 = 3,
  BAUD_RATE_57600 = 4,
  BAUD_RATE_115200 = 5,
  BAUD_RATE_230400 = 6,
  BAUD_RATE_256000 = 7,
  BAUD_RATE_460800 = 8
};

static const std::map<std::string, uint8_t> BAUD_RATE_ENUM_TO_INT{
    {"9600", BAUD_RATE_9600},     {"19200", BAUD_RATE_19200},   {"38400", BAUD_RATE_38400},
    {"57600", BAUD_RATE_57600},   {"115200", BAUD_RATE_115200}, {"230400", BAUD_RATE_230400},
    {"256000", BAUD_RATE_256000}, {"460800", BAUD_RATE_460800}};

enum ModeStructure : uint8_t {
  NORMAL_MODE = 1,
  ENGINEERING_MODE = 2,
  BACKGROUND_INIT_MODE = 3
};

static const std::map<std::string, uint8_t> MODE_ENUM_TO_INT{
    {"Normal", NORMAL_MODE},{"Engineering", ENGINEERING_MODE},{"Dynamic background correction", BACKGROUND_INIT_MODE}
};

enum DistanceResolutionStructure : uint8_t { DISTANCE_RESOLUTION_0_2 = 0x03, DISTANCE_RESOLUTION_0_5 = 0x01, DISTANCE_RESOLUTION_0_75 = 0x00 };

static const std::map<std::string, uint8_t> DISTANCE_RESOLUTION_ENUM_TO_INT{{"0.2m", DISTANCE_RESOLUTION_0_2},
                                                                            {"0.5m", DISTANCE_RESOLUTION_0_5},
                                                                            {"0.75m", DISTANCE_RESOLUTION_0_75}};
static const std::map<uint8_t, std::string> DISTANCE_RESOLUTION_INT_TO_ENUM{{DISTANCE_RESOLUTION_0_2, "0.2m"},
                                                                            {DISTANCE_RESOLUTION_0_5, "0.5m"},
                                                                            {DISTANCE_RESOLUTION_0_75, "0.75m"}};

// enum LightFunctionStructure : uint8_t {
//   LIGHT_FUNCTION_OFF = 0x00,
//   LIGHT_FUNCTION_BELOW = 0x01,
//   LIGHT_FUNCTION_ABOVE = 0x02
// };
// 
// static const std::map<std::string, uint8_t> LIGHT_FUNCTION_ENUM_TO_INT{
//     {"off", LIGHT_FUNCTION_OFF}, {"below", LIGHT_FUNCTION_BELOW}, {"above", LIGHT_FUNCTION_ABOVE}};
// static const std::map<uint8_t, std::string> LIGHT_FUNCTION_INT_TO_ENUM{
//     {LIGHT_FUNCTION_OFF, "off"}, {LIGHT_FUNCTION_BELOW, "below"}, {LIGHT_FUNCTION_ABOVE, "above"}};

enum OutPinLevelStructure : uint8_t { OUT_PIN_LEVEL_LOW = 0x01, OUT_PIN_LEVEL_HIGH = 0x00 };

static const std::map<std::string, uint8_t> OUT_PIN_LEVEL_ENUM_TO_INT{{"low", OUT_PIN_LEVEL_LOW},
                                                                      {"high", OUT_PIN_LEVEL_HIGH}};
static const std::map<uint8_t, std::string> OUT_PIN_LEVEL_INT_TO_ENUM{{OUT_PIN_LEVEL_LOW, "low"},
                                                                      {OUT_PIN_LEVEL_HIGH, "high"}};

// Commands values
static const uint8_t CMD_MAX_MOVE_VALUE = 0x0000;
static const uint8_t CMD_MAX_STILL_VALUE = 0x0001;
static const uint8_t CMD_DURATION_VALUE = 0x0002;
// Command Header & Footer
static const uint8_t CMD_FRAME_HEADER[4] = {0xFD, 0xFC, 0xFB, 0xFA};
static const uint8_t CMD_FRAME_END[4] = {0x04, 0x03, 0x02, 0x01};
// Data Header & Footer
static const uint8_t DATA_FRAME_HEADER[4] = {0xF4, 0xF3, 0xF2, 0xF1};
static const uint8_t DATA_FRAME_END[4] = {0xF8, 0xF7, 0xF6, 0xF5};

static const char HEX_POSITIONING_CONVERSION[] = "0123456789ABCDEF"; 

/*
Data Type: 6th byte
Target states: 9th byte
    Moving target distance: 10~11th bytes
    Moving target energy: 12th byte
    Still target distance: 13~14th bytes
    Still target energy: 15th byte
    Detect distance: 16~17th bytes
*/
enum PeriodicDataStructure : uint8_t {
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
enum PeriodicDataValue : uint8_t { HEAD = 0XAA, END = 0x55, CHECK = 0x00 };

enum AckDataStructure : uint8_t { COMMAND = 6, COMMAND_STATUS = 7 };

//  char cmd[2] = {enable ? 0xFF : 0xFE, 0x00};
class LD2412Component : public Component, public uart::UARTDevice {
#ifdef USE_SENSOR
  SUB_SENSOR(moving_target_distance)
  SUB_SENSOR(still_target_distance)
  SUB_SENSOR(moving_target_energy)
  SUB_SENSOR(still_target_energy)
  SUB_SENSOR(light)
  SUB_SENSOR(detection_distance)
#endif
#ifdef USE_BINARY_SENSOR
  SUB_BINARY_SENSOR(target)
  SUB_BINARY_SENSOR(moving_target)
  SUB_BINARY_SENSOR(still_target)
  SUB_BINARY_SENSOR(out_pin_presence_status)
#endif
#ifdef USE_TEXT_SENSOR
  SUB_TEXT_SENSOR(version)
  SUB_TEXT_SENSOR(mac)
#endif
#ifdef USE_SELECT
  SUB_SELECT(distance_resolution)
  SUB_SELECT(baud_rate)
  SUB_SELECT(light_function)
  SUB_SELECT(out_pin_level)
  SUB_SELECT(mode)
#endif
#ifdef USE_SWITCH
  SUB_SWITCH(engineering_mode)
  SUB_SWITCH(bluetooth)
#endif
#ifdef USE_BUTTON
  SUB_BUTTON(reset)
  SUB_BUTTON(restart)
  SUB_BUTTON(query)
#endif
#ifdef USE_NUMBER
  SUB_NUMBER(max_distance_gate)
  SUB_NUMBER(min_distance_gate)
  SUB_NUMBER(timeout)
  SUB_NUMBER(light_threshold)
#endif

 public:
  LD2412Component();
  void setup() override;
  void dump_config() override;
  void loop() override;
  void set_light_out_control();
#ifdef USE_NUMBER
  void set_gate_still_threshold_number(int gate, number::Number *n);
  void set_gate_move_threshold_number(int gate, number::Number *n);
  void set_basic_config();
  void set_gate_threshold();
  void get_gate_threshold();
#endif
#if !defined(USE_NUMBER) && defined(USE_SELECT)
  void set_basic_config();
#endif
#ifdef USE_SENSOR
  void set_gate_move_sensor(int gate, sensor::Sensor *s);
  void set_gate_still_sensor(int gate, sensor::Sensor *s);
#endif
  void set_throttle(uint16_t value) { this->throttle_ = value; };
  void set_bluetooth_password(const std::string &password);
  void set_engineering_mode(bool enable);
  void set_mode(const std::string &state);
  void read_all_info();
  void restart_and_read_all_info();
  void set_bluetooth(bool enable);
  void set_distance_resolution(const std::string &state);
  void set_baud_rate(const std::string &state);
  void factory_reset();

 protected:
  int two_byte_to_int_(char firstbyte, char secondbyte) { return (int16_t) (secondbyte << 8) + firstbyte; }
  void send_command_(uint8_t command_str, const uint8_t *command_value, int command_value_len);
  void set_config_mode_(bool enable);
  void handle_periodic_data_(uint8_t *buffer, int len);
  bool handle_ack_data_(uint8_t *buffer, int len);
  void readline_(int readch, uint8_t *buffer, int len);
  void query_parameters_();
  void get_version_();
  void get_mac_();
  void get_distance_resolution_();
  void get_light_control_();
  void restart_();
  void query_dymanic_background_correction_();

  std::string format_buffer(uint8_t* buffer, size_t len)
  {
    std::string version;
    version.resize(len*2+1);
    for (size_t i = 0; i < len; i++)
    {
      version[(i * 2) + 0] = HEX_POSITIONING_CONVERSION[((buffer[i] & 0xF0) >> 4)];
      version[(i * 2) + 1] = HEX_POSITIONING_CONVERSION[((buffer[i] & 0x0F) >> 0)];
    }
    return version;
  }

  int32_t last_periodic_millis_ = millis();
  int32_t last_engineering_mode_change_millis_ = millis();
  uint16_t throttle_;
  std::string version_;
  std::string mac_;
  std::string out_pin_level_;
  bool dynamic_bakground_correction_active_;
  std::string light_function_;
  float light_threshold_ = -1;
#ifdef USE_NUMBER
  std::vector<number::Number *> gate_still_threshold_numbers_ = std::vector<number::Number *>(14);
  std::vector<number::Number *> gate_move_threshold_numbers_ = std::vector<number::Number *>(14);
#endif
#ifdef USE_SENSOR
  std::vector<sensor::Sensor *> gate_still_sensors_ = std::vector<sensor::Sensor *>(14);
  std::vector<sensor::Sensor *> gate_move_sensors_ = std::vector<sensor::Sensor *>(14);
#endif
};

}  // namespace LD2412
}  // namespace esphome
