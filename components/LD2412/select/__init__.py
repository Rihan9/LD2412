import esphome.codegen as cg
from esphome.components import select
import esphome.config_validation as cv
from esphome.const import (
    ENTITY_CATEGORY_CONFIG,
    CONF_BAUD_RATE,
    ICON_THERMOMETER,
    ICON_SCALE,
    ICON_LIGHTBULB,
    ICON_RULER,
)
from .. import CONF_LD2412_ID, LD2412Component, LD2412_ns

BaudRateSelect = LD2412_ns.class_("BaudRateSelect", select.Select)
DistanceResolutionSelect = LD2412_ns.class_("DistanceResolutionSelect", select.Select)
LightOutControlSelect = LD2412_ns.class_("LightOutControlSelect", select.Select)
ModeSelect = LD2412_ns.class_("ModeSelect", select.Select)

CONF_DISTANCE_RESOLUTION = "distance_resolution"
CONF_LIGHT_FUNCTION = "light_function"
CONF_OUT_PIN_LEVEL = "out_pin_level"
CONF_MODE = "mode"


CONFIG_SCHEMA = {
    cv.GenerateID(CONF_LD2412_ID): cv.use_id(LD2412Component),
    cv.Optional(CONF_DISTANCE_RESOLUTION): select.select_schema(
        DistanceResolutionSelect,
        entity_category=ENTITY_CATEGORY_CONFIG,
        icon=ICON_RULER,
    ),
    # cv.Optional(CONF_LIGHT_FUNCTION): select.select_schema(
    #     LightOutControlSelect,
    #     entity_category=ENTITY_CATEGORY_CONFIG,
    #     icon=ICON_LIGHTBULB,
    # ),
    cv.Optional(CONF_OUT_PIN_LEVEL): select.select_schema(
        LightOutControlSelect,
        entity_category=ENTITY_CATEGORY_CONFIG,
        icon=ICON_SCALE,
    ),
    cv.Optional(CONF_BAUD_RATE): select.select_schema(
        BaudRateSelect,
        entity_category=ENTITY_CATEGORY_CONFIG,
        icon=ICON_THERMOMETER,
    ),
    cv.Optional(CONF_MODE): select.select_schema(
        ModeSelect,
        entity_category=ENTITY_CATEGORY_CONFIG
    ),
}


async def to_code(config):
    LD2412_component = await cg.get_variable(config[CONF_LD2412_ID])
    if distance_resolution_config := config.get(CONF_DISTANCE_RESOLUTION):
        s = await select.new_select(
            distance_resolution_config, options=["0.2m", "0.5m", "0.75m"]
        )
        await cg.register_parented(s, config[CONF_LD2412_ID])
        cg.add(LD2412_component.set_distance_resolution_select(s))
    if out_pin_level_config := config.get(CONF_OUT_PIN_LEVEL):
        s = await select.new_select(out_pin_level_config, options=["low", "high"])
        await cg.register_parented(s, config[CONF_LD2412_ID])
        cg.add(LD2412_component.set_out_pin_level_select(s))
    # if light_function_config := config.get(CONF_LIGHT_FUNCTION):
    #     s = await select.new_select(
    #         light_function_config, options=["off", "below", "above"]
    #     )
    #     await cg.register_parented(s, config[CONF_LD2412_ID])
    #     cg.add(LD2412_component.set_light_function_select(s))
    if baud_rate_config := config.get(CONF_BAUD_RATE):
        s = await select.new_select(
            baud_rate_config,
            options=[
                "9600",
                "19200",
                "38400",
                "57600",
                "115200",
                "230400",
                "256000",
                "460800",
            ],
        )
        await cg.register_parented(s, config[CONF_LD2412_ID])
        cg.add(LD2412_component.set_baud_rate_select(s))
    if mode_config := config.get(CONF_MODE):
        s = await select.new_select(
            mode_config,
            options=[
                "Normal",
                "Engineering",
                "Dynamic background correction",
            ],
        )
        await cg.register_parented(s, config[CONF_LD2412_ID])
        cg.add(LD2412_component.set_mode_select(s))
