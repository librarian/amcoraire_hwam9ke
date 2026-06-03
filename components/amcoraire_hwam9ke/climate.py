import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate
from esphome.const import CONF_ID, CONF_PIN

AUTO_LOAD = ["climate"]

amcoraire_ns = cg.esphome_ns.namespace("amcoraire_hwam9ke")

AmcoraireHWAM9KEClimate = amcoraire_ns.class_(
    "AmcoraireHWAM9KEClimate",
    climate.Climate,
    cg.Component,
)

CONFIG_SCHEMA = climate.climate_schema(AmcoraireHWAM9KEClimate).extend(
    {
        cv.GenerateID(): cv.declare_id(AmcoraireHWAM9KEClimate),
        # Use numeric GPIO number, e.g. pin: 4 for GPIO4.
        cv.Required(CONF_PIN): cv.int_,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await climate.register_climate(var, config)

    cg.add(var.set_pin(config[CONF_PIN]))
    cg.add_library("crankyoldgit/IRremoteESP8266", None)
