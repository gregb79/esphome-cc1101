# See: https://esphome.io/guides/contributing.html#extras

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.const import (
    CONF_ID,
    CONF_FREQUENCY,
)

# Mark the component to depend on other components.
# If the user hasn’t explicitly added these components in their configuration, a validation error will be generated.
DEPENDENCIES = [ ]
# Automatically load a component if the user hasn’t added it manually
AUTO_LOAD = [ ]
# Mark this component to accept an array of configurations.
# If this is an integer instead of a boolean, validation will only permit the given number of entries.
MULTI_CONF = False

# GitHub usernames or team names of people that are responsible for this component
CODEOWNERS = ["@vinsce"]

# Define constants for configuration keys
CONF_SCK_PIN = "sck_pin"
CONF_MISO_PIN = "miso_pin"
CONF_MOSI_PIN = "mosi_pin"
CONF_CSN_PIN = "csn_pin"
CONF_GDO0_PIN = "gdo0_pin"
CONF_BANDWIDTH = "bandwidth"

# C++ namespace
ns = cg.esphome_ns.namespace("cc1101")
CC1101 = ns.class_("CC1101", cg.Component)

# The configuration schema to validate the user config against
CONFIG_SCHEMA = cv.COMPONENT_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(CC1101),
        #cv.Required(CONF_NAME): cv.string,
        # TODO review input/output pins
        cv.Required(CONF_SCK_PIN): pins.gpio_output_pin_schema,
        cv.Required(CONF_MISO_PIN): pins.gpio_output_pin_schema,
        cv.Required(CONF_MOSI_PIN): pins.gpio_output_pin_schema,
        cv.Required(CONF_CSN_PIN): pins.gpio_output_pin_schema,
        cv.Required(CONF_GDO0_PIN): pins.gpio_input_pin_schema,
        cv.Optional(CONF_BANDWIDTH, default=200.0): cv.float_,
        cv.Optional(CONF_FREQUENCY, default=868.65): cv.float_,
    }
)


async def to_code(config):
    sck_pin = await cg.gpio_pin_expression(config[CONF_SCK_PIN])
    miso_pin = await cg.gpio_pin_expression(config[CONF_MISO_PIN])
    mosi_pin = await cg.gpio_pin_expression(config[CONF_MOSI_PIN])
    csn_pin = await cg.gpio_pin_expression(config[CONF_CSN_PIN])
    gd0_pin = await cg.gpio_pin_expression(config[CONF_GDO0_PIN])

    # Declare new component
    # Will create a new pointer:
    #   transceiver = new cc1101::CC1101();
    var = cg.new_Pvariable(config[CONF_ID], sck_pin, miso_pin, mosi_pin, csn_pin, gd0_pin)

    # Will configure and register the component:
    #   transceiver->set_update_interval(60000);
    #   transceiver->set_component_source("cc1101");
    #   App.register_component(transceiver);
    await cg.register_component(var, config)

    # cg.add is used to add a piece of code to the generated main.cpp
    #   transceiver->set_bandwidth(200);
    cg.add(var.set_bandwidth(config[CONF_BANDWIDTH]))
    cg.add(var.set_frequency(config[CONF_FREQUENCY]))
