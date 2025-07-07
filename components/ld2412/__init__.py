from esphome import automation
from esphome.automation import maybe_simple_id
import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_PASSWORD, CONF_THROTTLE

DEPENDENCIES = ["uart"]
CODEOWNERS = ["@Rihan9"]
MULTI_CONF = True

LD2412_ns = cg.esphome_ns.namespace("ld2412")
LD2412Component = LD2412_ns.class_("LD2412Component", cg.Component, uart.UARTDevice)

CONF_LD2412_ID = "ld2412_id"

CONF_MAX_MOVE_DISTANCE = "max_move_distance"
CONF_MAX_STILL_DISTANCE = "max_still_distance"
CONF_MOVE_THRESHOLDS = [f"g{x}_move_threshold" for x in range(9)]
CONF_STILL_THRESHOLDS = [f"g{x}_still_threshold" for x in range(9)]

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(LD2412Component),
            cv.Optional(CONF_THROTTLE, default="1000ms"): cv.All(
                cv.positive_time_period_milliseconds,
                cv.Range(min=cv.TimePeriod(milliseconds=1)),
            ),
        }
    )
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)

FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "ld2412",
    require_tx=True,
    require_rx=True,
    parity="NONE",
    stop_bits=1,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    cg.add(var.set_throttle(config[CONF_THROTTLE]))


CALIBRATION_ACTION_SCHEMA = maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(LD2412Component),
    }
)


BluetoothPasswordSetAction = LD2412_ns.class_(
    "BluetoothPasswordSetAction", automation.Action
)


BLUETOOTH_PASSWORD_SET_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_ID): cv.use_id(LD2412Component),
        cv.Required(CONF_PASSWORD): cv.templatable(cv.string_strict),
    }
)


@automation.register_action(
    "bluetooth_password.set", BluetoothPasswordSetAction, BLUETOOTH_PASSWORD_SET_SCHEMA
)
async def bluetooth_password_set_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)
    template_ = await cg.templatable(config[CONF_PASSWORD], args, cg.std_string)
    cg.add(var.set_password(template_))
    return var
