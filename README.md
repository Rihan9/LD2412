LD2412 support for EspHome
=======
Important Information
--
This component has been merged in the official esphome repository. So, this repository will be no longer maintained. Please, if you find an issue or want to suggest an improvement, use the official repositories from now on. Thank you all for the collaboration until now!


Introduction
--
Hi all! This repo contains the revision of the official LD2410 component adapted for the LD2412. Below you can find an example of the configuration, what works and what doesn’t compared to the LD2410 component. This repository is not to be considered production-ready, but rather a starting point for the implementation of this device on EspHome. Unfortunately, I don’t have much time to dedicate to it, so unless someone forks it, progress will be a bit slow.

Configuration example
--
```
external_components:
  - source:
      type: git
      url: https://github.com/Rihan9/LD2412
      ref: main
    components: [LD2412]

LD2412:
  id: ld2412
  throttle: 3s

binary_sensor:
  - platform: LD2412
    has_target:
      name: Presence
    has_moving_target:
      name: Moving Target
    has_still_target:
      name: Still Target

sensor:
  - platform: LD2412
    moving_distance:
      name : Moving Distance
    still_distance:
      name: Still Distance
    moving_energy:
      name: Move Energy
    still_energy:
      name: Still Energy
    detection_distance:
      name: Detection Distance
    light:
      name: light
    g0:
      move_energy:
        name: g00 move energy
      still_energy:
        name: g00 still energy
    g1:
      move_energy:
        name: g01 move energy
      still_energy:
        name: g01 still energy
    g2:
      move_energy:
        name: g02 move energy
      still_energy:
        name: g02 still energy
    g3:
      move_energy:
        name: g03 move energy
      still_energy:
        name: g03 still energy
    g4:
      move_energy:
        name: g04 move energy
      still_energy:
        name: g04 still energy
    g5:
      move_energy:
        name: g05 move energy
      still_energy:
        name: g05 still energy
    g6:
      move_energy:
        name: g06 move energy
      still_energy:
        name: g06 still energy
    g7:
      move_energy:
        name: g07 move energy
      still_energy:
        name: g07 still energy
    g8:
      move_energy:
        name: g08 move energy
      still_energy:
        name: g08 still energy
    g9:
      move_energy:
        name: g09 move energy
      still_energy:
        name: g09 still energy
    g10:
      move_energy:
        name: g10 move energy
      still_energy:
        name: g10 still energy
    g11:
      move_energy:
        name: g11 move energy
      still_energy:
        name: g11 still energy
    g12:
      move_energy:
        name: g12 move energy
      still_energy:
        name: g12 still energy
    g13:
      move_energy:
        name: g13 move energy
      still_energy:
        name: g13 still energy

number:
  - platform: LD2412
    timeout:
      name: "presence holding"
    min_distance_gate:
      name: "min distance gate"
    max_distance_gate:
      name: "max distance gate"
    g0:
      move_threshold:
        name: g00 move threshold
      still_threshold:
        name: g00 still threshold
    g1:
      move_threshold:
        name: g01 move threshold
      still_threshold:
        name: g01 still threshold
    g2:
      move_threshold:
        name: g02 move threshold
      still_threshold:
        name: g02 still threshold
    g3:
      move_threshold:
        name: g03 move threshold
      still_threshold:
        name: g03 still threshold
    g4:
      move_threshold:
        name: g04 move threshold
      still_threshold:
        name: g04 still threshold
    g5:
      move_threshold:
        name: g05 move threshold
      still_threshold:
        name: g05 still threshold
    g6:
      move_threshold:
        name: g06 move threshold
      still_threshold:
        name: g06 still threshold
    g7:
      move_threshold:
        name: g07 move threshold
      still_threshold:
        name: g07 still threshold
    g8:
      move_threshold:
        name: g08 move threshold
      still_threshold:
        name: g08 still threshold
    g9:
      move_threshold:
        name: g09 move threshold
      still_threshold:
        name: g09 still threshold
    g10:
      move_threshold:
        name: g10 move threshold
      still_threshold:
        name: g10 still threshold
    g11:
      move_threshold:
        name: g11 move threshold
      still_threshold:
        name: g11 still threshold
    g12:
      move_threshold:
        name: g12 move threshold
      still_threshold:
        name: g12 still threshold
    g13:
      move_threshold:
        name: g13 move threshold
      still_threshold:
        name: g13 still threshold
  
select:
  - platform: LD2412
    out_pin_level:
      name: 'Hardware output pin level'
    distance_resolution:
      name: 'Distance resolution'
    baud_rate:
      name: "baud rate"
      on_value:
        - delay: 3s
        - lambda: |-
            id(uart_bus).flush();
            uint32_t new_baud_rate = stoi(x);
            ESP_LOGD("change_baud_rate", "Changing baud rate from %i to %i",id(uart_bus).get_baud_rate(), new_baud_rate);
            if (id(uart_bus).get_baud_rate() != new_baud_rate) {
            id(uart_bus).set_baud_rate(new_baud_rate);
            id(uart_bus).load_settings();
            }
    mode:
      name: "Mode"
button:
  - platform: LD2412
    factory_reset:
      name: "factory reset"
    restart:
      name: "restart"
    query_params:
      name: query params

text_sensor:
  - platform: LD2412
    version:
      name: "firmware version"
    mac_address:
      name: "mac address"
switch:
  - platform: LD2412
    bluetooth:
      name: "Bluetooth"
```
What does work
--
- generic Presence and distance
- Still Presence, distance and energy
- Moving presence, distance and energy
- Start and End gate configuration
- Delay for presence off (aka: timeout)
- Firmware and Bluetooth mac address presentation
- Output pin configuration
- Restart and Query button
- Uart Band Rate configuration
- Distance resolution
- Factory reset button
- Bluetooth switch. 
- Engineering mode: switch to and back from, threshold configuration, gate sensing and light sensor
