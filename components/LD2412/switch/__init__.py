import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv
from esphome.const import (
    DEVICE_CLASS_SWITCH,
    ICON_BLUETOOTH,
    ENTITY_CATEGORY_CONFIG,
    ICON_PULSE,
)
from .. import CONF_LD2412_ID, LD2412Component, LD2412_ns

BluetoothSwitch = LD2412_ns.class_("BluetoothSwitch", switch.Switch)
# EngineeringModeSwitch = LD2412_ns.class_("EngineeringModeSwitch", switch.Switch)

CONF_ENGINEERING_MODE = "engineering_mode"
CONF_BLUETOOTH = "bluetooth"

CONFIG_SCHEMA = {
    cv.GenerateID(CONF_LD2412_ID): cv.use_id(LD2412Component),
    # cv.Optional(CONF_ENGINEERING_MODE): switch.switch_schema(
    #     EngineeringModeSwitch,
    #     device_class=DEVICE_CLASS_SWITCH,
    #     entity_category=ENTITY_CATEGORY_CONFIG,
    #     icon=ICON_PULSE,
    # ),
    # cv.Optional(CONF_BLUETOOTH): switch.switch_schema(
    #     BluetoothSwitch,
    #     device_class=DEVICE_CLASS_SWITCH,
    #     entity_category=ENTITY_CATEGORY_CONFIG,
    #     icon=ICON_BLUETOOTH,
    # ),
}


async def to_code(config):
    LD2412_component = await cg.get_variable(config[CONF_LD2412_ID])
    # if engineering_mode_config := config.get(CONF_ENGINEERING_MODE):
    #     s = await switch.new_switch(engineering_mode_config)
    #     await cg.register_parented(s, config[CONF_LD2412_ID])
    #     cg.add(LD2412_component.set_engineering_mode_switch(s))
    # if bluetooth_config := config.get(CONF_BLUETOOTH):
    #     s = await switch.new_switch(bluetooth_config)
    #     await cg.register_parented(s, config[CONF_LD2412_ID])
    #     cg.add(LD2412_component.set_bluetooth_switch(s))
