LD2412 support for EspHome
=======
Introduction
--
Hi all! This repo contains the revision of the official LD2410 component adapted for the LD2412. Below you can find an example of the configuration, what works and what doesn’t compared to the LD2410 component. This repository is not to be considered production-ready, but rather a starting point for the implementation of this device on EspHome. Unfortunately, I don’t have much time to dedicate to it, so unless someone forks it, progress will be a bit slow.

Configuration example
--
```
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

number:
  - platform: LD2412
    timeout:
      name: "presence holding"
    min_distance_gate:
      name: "min distance gate"
    max_distance_gate:
      name: "max distance gate"
  
select:
  - platform: LD2412
    out_pin_level:
      name: 'Hardware output pin level'
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
- Uart Band Rate configuration **NOT TESTED

What doesn't
--
- Factory reset button. I still don't know why but i'm working on it
- Bluetooth switch. Not implemented by the production company in the serial protocol
- Distance resolution. The worst lacking for me but it doesn't seem implemented by the production company (no information available in the documentation)
- Engineering mode: forgive me, but it's a bit too much for me
- Light configuration / threashold and light sensor ot implemented by the production company in the serial protocol for the first two. The light sensor is available only in engineering mode, quite useless.