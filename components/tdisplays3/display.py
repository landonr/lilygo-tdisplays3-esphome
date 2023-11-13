import esphome.codegen as cg
import esphome.config_validation as cv

from esphome import pins
from esphome.components import display
from esphome.const import (
    CONF_HEIGHT,
    CONF_ID,
    CONF_LAMBDA,
    CONF_WIDTH,
    CONF_CS_PIN,
    CONF_DC_PIN,
    CONF_RESET_PIN,
    CONF_NUMBER,
)
from esphome.const import __version__ as ESPHOME_VERSION
from . import tdisplays3_ns

AUTO_LOAD = ["psram"]
DEPENDENCIES = ["esp32"]

CONF_BACKLIGHT = "backlight"
CONF_LOAD_FONTS = "load_fonts"
CONF_LOAD_SMOOTH_FONTS = "load_smooth_fonts"
CONF_ENABLE_LIBRARY_WARNINGS = "enable_library_warnings"

TDISPLAYS3 = tdisplays3_ns.class_(
    "TDisplayS3", cg.PollingComponent, display.DisplayBuffer
)

CONFIG_SCHEMA = cv.All(
    display.FULL_DISPLAY_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(TDISPLAYS3),
            cv.Optional(CONF_HEIGHT, default=320): cv.uint16_t,
            cv.Optional(CONF_WIDTH, default=170): cv.uint16_t,
            cv.Optional(CONF_BACKLIGHT, default=False): cv.boolean,
            cv.Optional(CONF_LOAD_FONTS, default=False): cv.boolean,
            cv.Optional(CONF_LOAD_SMOOTH_FONTS, default=False): cv.boolean,
            cv.Optional(CONF_ENABLE_LIBRARY_WARNINGS, default=False): cv.boolean,
            cv.Optional(CONF_RESET_PIN, default=5): pins.gpio_output_pin_schema,
            cv.Optional(CONF_CS_PIN, default=6): pins.gpio_output_pin_schema,
            cv.Optional(CONF_DC_PIN, default=7): pins.gpio_output_pin_schema,
        }
    ).extend(cv.polling_component_schema("5s")),
)


async def to_code(config):
    # Add platformio build_flags for the correct TFT_eSPI settings for the T-Display-S3
    # This allows using current, unpatched versions of TFT_eSPI
    cg.add_build_flag("-DUSER_SETUP_LOADED")
    cg.add_build_flag("-DST7789_DRIVER")
    cg.add_build_flag("-DINIT_SEQUENCE_3")
    cg.add_build_flag("-DCGRAM_OFFSET")
    cg.add_build_flag("-DTFT_RGB_ORDER=TFT_RGB")
    cg.add_build_flag("-DTFT_INVERSION_ON")
    cg.add_build_flag("-DTFT_PARALLEL_8_BIT")
    cg.add_build_flag(f"-DTFT_WIDTH={config[CONF_WIDTH]}")
    cg.add_build_flag(f"-DTFT_HEIGHT={config[CONF_HEIGHT]}")

    cg.add_build_flag(f"-DTFT_RST={config[CONF_RESET_PIN][CONF_NUMBER]}")
    cg.add_build_flag(f"-DTFT_CS={config[CONF_CS_PIN][CONF_NUMBER]}")
    cg.add_build_flag(f"-DTFT_DC={config[CONF_DC_PIN][CONF_NUMBER]}")

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

    cg.add_build_flag("-DSPI_FREQUENCY=40000000")

    if config[CONF_LOAD_FONTS]:
        cg.add_build_flag("-DLOAD_GLCD")
        cg.add_build_flag("-DLOAD_FONT2")
        cg.add_build_flag("-DLOAD_FONT4")
        cg.add_build_flag("-DLOAD_FONT6")
        cg.add_build_flag("-DLOAD_FONT7")
        cg.add_build_flag("-DLOAD_FONT8")
        cg.add_build_flag("-DLOAD_GFXFF")

    if config[CONF_LOAD_SMOOTH_FONTS]:
        cg.add_build_flag("-DSMOOTH_FONT")
        cg.add_library("FS", None)
        cg.add_library("SPIFFS", None)

    if not config[CONF_ENABLE_LIBRARY_WARNINGS]:
        cg.add_build_flag("-DDISABLE_ALL_LIBRARY_WARNINGS")

    # TODO:
    # PIN_POWER_ON 15
    # Bat_Volt  4
    # Touch_reset 21

    if config[CONF_BACKLIGHT]:
        cg.add_build_flag("-DTFT_BL=38")
        cg.add_build_flag("-DTFT_BACKLIGHT_ON=HIGH")

    cg.add_library("TFT_eSPI", None)
    #cg.add_library("SPI", None)

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await display.register_display(var, config)
    cg.add(var.set_dimensions(config[CONF_WIDTH], config[CONF_HEIGHT]));

    if CONF_LAMBDA in config:
        if cv.Version.parse(ESPHOME_VERSION) < cv.Version.parse("2023.7.0"):
            displayRef = display.DisplayBufferRef
        else:
            displayRef = display.DisplayRef
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA], [(displayRef, "it")], return_type=cg.void
        )
        cg.add(var.set_writer(lambda_))
