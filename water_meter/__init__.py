import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    UNIT_SECOND,
)

DEPENDENCIES = ["sensor"]

water_meter_ns = cg.esphome_ns.namespace("water_meter")
WaterMeterComponent = water_meter_ns.class_("WaterMeterComponent", cg.Component)

CONF_FLOW_SENSOR = "flow_sensor"
CONF_VOLUME_SENSOR = "volume_sensor"
CONF_DURATION_SENSOR = "duration_sensor"
CONF_SENSOR_PIN = "sensor_pin"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(WaterMeterComponent),
        cv.Optional(CONF_SENSOR_PIN, default=15): cv.int_range(min=0, max=39),
        cv.Required(CONF_FLOW_SENSOR): sensor.sensor_schema(
            unit_of_measurement="L/min",
            accuracy_decimals=2,
        ),
        cv.Required(CONF_VOLUME_SENSOR): sensor.sensor_schema(
            unit_of_measurement="L",
            accuracy_decimals=1,
        ),
        cv.Required(CONF_DURATION_SENSOR): sensor.sensor_schema(
            unit_of_measurement=UNIT_SECOND,
            accuracy_decimals=0,
        ),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_sensor_pin(config[CONF_SENSOR_PIN]))

    flow_sens = await sensor.new_sensor(config[CONF_FLOW_SENSOR])
    cg.add(var.set_flow_sensor(flow_sens))

    volume_sens = await sensor.new_sensor(config[CONF_VOLUME_SENSOR])
    cg.add(var.set_volume_sensor(volume_sens))

    duration_sens = await sensor.new_sensor(config[CONF_DURATION_SENSOR])
    cg.add(var.set_duration_sensor(duration_sens))
