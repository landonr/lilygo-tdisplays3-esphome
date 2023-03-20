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
    # Add platformio build_flags for the correct TFT_eSPI settings for the T-Display-S3
    # This allows using current, unpatched versions of TFT_eSPI
    cg.add_build_flag("-DUSER_SETUP_LOADED=1")
    cg.add_build_flag("-DST7789_DRIVER=1")
    cg.add_build_flag("-DINIT_SEQUENCE_3=1")
    cg.add_build_flag("-DCGRAM_OFFSET")
    cg.add_build_flag("-DTFT_RGB_ORDER=TFT_RGB")
    cg.add_build_flag("-DTFT_INVERSION_ON=1")
    cg.add_build_flag("-DTFT_PARALLEL_8_BIT=1")
    cg.add_build_flag("-DTFT_WIDTH=170")
    cg.add_build_flag("-DTFT_HEIGHT=320")
    cg.add_build_flag("-DTFT_DC=7")
    cg.add_build_flag("-DTFT_RST=5")
    cg.add_build_flag("-DTFT_WR=8")
    cg.add_build_flag("-DTFT_RD=9")
    cg.add_build_flag("-DTFT_D0=39")
    cg.add_build_flag("-DTFT_D1=40")
    cg.add_build_flag("-DTFT_D2=41")
    cg.add_build_flag("-DTFT_D3=42")
    cg.add_build_flag("-DTFT_D4=45")
    cg.add_build_flag("-DTFT_D5=46")
    cg.add_build_flag("-DTFT_D6=47")
    cg.add_build_flag("-DTFT_D7=48")
    cg.add_build_flag("-DLOAD_GLCD=1")
    cg.add_build_flag("-DLOAD_FONT2=1")
    cg.add_build_flag("-DLOAD_FONT4=1")
    cg.add_build_flag("-DLOAD_FONT6=1")
    cg.add_build_flag("-DLOAD_FONT7=1")
    cg.add_build_flag("-DLOAD_FONT8=1")
    cg.add_build_flag("-DLOAD_GFXFF=1")
    cg.add_build_flag("-DSMOOTH_FONT=1")
    # If you don't care about control of the backlight you can uncomment the two lines below")
    #cg.add_build_flag("-DTFT_BL=38")
    #cg.add_build_flag("-DTFT_BACKLIGHT_ON=HIGH")

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
