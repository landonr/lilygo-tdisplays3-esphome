import esphome.codegen as cg
import esphome.config_validation as cv

from esphome import pins
from esphome.components import i2c, touchscreen
from esphome.const import CONF_ID, CONF_INTERRUPT_PIN, CONF_RESET_PIN
from esphome.const import __version__ as ESPHOME_VERSION

from .. import tdisplays3_ns

DEPENDENCIES = ["i2c"]

if cv.Version.parse(ESPHOME_VERSION) < cv.Version.parse("2023.12.0"):
    LilygoTDisplayS3Touchscreen = tdisplays3_ns.class_(
        "LilygoTDisplayS3Touchscreen",
        touchscreen.Touchscreen,
        cg.Component,
        i2c.I2CDevice,
    )
else:
    LilygoTDisplayS3Touchscreen = tdisplays3_ns.class_(
        "LilygoTDisplayS3Touchscreen",
        touchscreen.Touchscreen,
        i2c.I2CDevice,
    )

CONF_OFFSET_X = "x_offset"
CONF_OFFSET_Y = "y_offset"

CONFIG_SCHEMA = touchscreen.TOUCHSCREEN_SCHEMA.extend(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(LilygoTDisplayS3Touchscreen),
            cv.Required(CONF_INTERRUPT_PIN): cv.All(
                pins.internal_gpio_input_pin_schema
            ),
            cv.Optional(CONF_OFFSET_X): cv.int_range(min=-32768, max=32767),
            cv.Optional(CONF_OFFSET_Y): cv.int_range(min=-32768, max=32767),
        }
    )
    .extend(i2c.i2c_device_schema(0x15))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])

    if cv.Version.parse(ESPHOME_VERSION) < cv.Version.parse("2023.12.0"):
        await cg.register_component(var, config)
    await touchscreen.register_touchscreen(var, config)
    await i2c.register_i2c_device(var, config)

    interrupt_pin = await cg.gpio_pin_expression(config[CONF_INTERRUPT_PIN])
    cg.add(var.set_interrupt_pin(interrupt_pin))

    x_offset = config.get(CONF_OFFSET_X, 0)
    y_offset = config.get(CONF_OFFSET_Y, 0)
    cg.add(var.set_offset(x_offset, y_offset))
