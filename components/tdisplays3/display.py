import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import display
from esphome.const import (
  CONF_HEIGHT,
  CONF_ID,
  CONF_LAMBDA,
  CONF_WIDTH,
)
from . import tdisplays3_ns

TDISPLAYS3 = tdisplays3_ns.class_(
    "TDisplayS3", cg.PollingComponent, display.DisplayBuffer
)

def validate_tdisplays3(config):
   return config

CONFIG_SCHEMA = cv.All(
    display.FULL_DISPLAY_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(TDISPLAYS3),
            cv.Optional(CONF_HEIGHT): cv.int_,
            cv.Optional(CONF_WIDTH): cv.int_,
        }
    ).extend(cv.polling_component_schema("5s")),
    validate_tdisplays3,
)

async def to_code(config):
    cg.add_library("SPI", None)
    cg.add_library("FS", None)
    cg.add_library("SPIFFS", None)
    cg.add_library("TFT_eSPI", None)

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await display.register_display(var, config)

    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA], [(display.DisplayBufferRef, "it")], return_type=cg.void
        )
        cg.add(var.set_writer(lambda_))
