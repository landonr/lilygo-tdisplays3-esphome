import logging

import esphome.codegen as cg
import esphome.config_validation as cv
import esphome.final_validate as fv

from esphome import pins
from esphome.components import i2c, touchscreen
from esphome.components.touchscreen import CONF_DISPLAY, CONF_TRANSFORM, CONF_SWAP_XY, CONF_MIRROR_X, CONF_MIRROR_Y
from esphome.const import CONF_ID, CONF_INTERRUPT_PIN, CONF_RESET_PIN
from esphome.const import __version__ as ESPHOME_VERSION

from .. import tdisplays3_ns

_LOGGER = logging.getLogger(__name__)

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
CONF_ROTATION = "rotation"

def inherit_transform(touchscreen_config):
    """
    If the display configuration has a 'transform' option and the touchscreen doesn't,
    copy that configuration.
    If the display configuration has a 'rotation' option and the touchscreen doesn't have a
    'transform' option, translate the rotation to the needed transform configurations and add
    it to the touchscreen configuration.
    """

    if CONF_TRANSFORM in touchscreen_config:
        _LOGGER.debug("'transform' manually specified on the touchscreen. Do not inherit form display")
        return touchscreen_config

    fconf = fv.full_config.get()
    display_path = fconf.get_path_for_id(touchscreen_config[CONF_DISPLAY])[:-1]
    display_config = fconf.get_config_for_path(display_path)

    if CONF_TRANSFORM in display_config:
        transform = display_config[CONF_TRANSFORM]
        _LOGGER.info("Copying transform from display %s: %s", display_config[CONF_ID], transform)
        touchscreen_config[CONF_TRANSFORM] = transform
    elif CONF_ROTATION in display_config:
        rotation = display_config[CONF_ROTATION]
        transform = {}
        if rotation == 0:
            transform[CONF_SWAP_XY] = False
            transform[CONF_MIRROR_X] = False
            transform[CONF_MIRROR_Y] = False
        elif rotation == 90:
            transform[CONF_SWAP_XY] = True
            transform[CONF_MIRROR_X] = False
            transform[CONF_MIRROR_Y] = False
        elif rotation == 180:
            transform[CONF_SWAP_XY] = False
            transform[CONF_MIRROR_X] = True
            transform[CONF_MIRROR_Y] = False
        elif rotation == 270:
            transform[CONF_SWAP_XY] = True
            transform[CONF_MIRROR_X] = True
            transform[CONF_MIRROR_Y] = False
        _LOGGER.info("Translating rotation %d from display %s: %s", display_config[CONF_ROTATION], display_config[CONF_ID], transform)
        touchscreen_config[CONF_TRANSFORM] = transform

    return touchscreen_config


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

FINAL_VALIDATE_SCHEMA = cv.All(inherit_transform)

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
