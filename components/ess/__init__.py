import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_POWER,
    CONF_POWER_SUPPLY,
    CONF_BATTERY_VOLTAGE,
    UNIT_VOLT,
    UNIT_WATT,
    DEVICE_CLASS_VOLTAGE,
    DEVICE_CLASS_POWER,
    STATE_CLASS_MEASUREMENT,
    CONF_MAX_VALUE,
    CONF_MIN_VALUE,
    CONF_STEP,

)
from esphome.core import CORE
from esphome.components.esp32 import (
    add_idf_sdkconfig_option,
)
from esphome.components import (
    uart,
    sensor,
    binary_sensor,
    number,
    template,
)
from esphome.components.template import (
    number as template_number
)
from esphome.const import CONF_NAME, CONF_UART_ID

DEPENDENCIES = ["uart"]
AUTO_LOAD = []

victron_ns = cg.esphome_ns.namespace("victron")
Ess = victron_ns.class_("Ess", cg.Component, uart.UARTDevice)

EssNumber = victron_ns.class_(
    "EssNumber", number.Number, cg.PollingComponent
)


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(Ess),
            cv.Optional(CONF_POWER): sensor.sensor_schema(
                unit_of_measurement=UNIT_WATT,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_POWER,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_BATTERY_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),

            cv.Optional(CONF_POWER_SUPPLY):     number.number_schema(
                            EssNumber,
                            #entity_category=ENTITY_CATEGORY_CONFIG,
                            
                            unit_of_measurement=UNIT_WATT,
                        )
                        .extend(
                            {
                                #cv.Optional(CONF_MODE, default="BOX"): cv.enum(number.NUMBER_MODES),
                                cv.Optional(CONF_MAX_VALUE, default=10): cv.All(
                                    cv.float_, cv.Range(max=3000)
                                ),
                                cv.Optional(CONF_MIN_VALUE, default=-10): cv.All(
                                    cv.float_, cv.Range(min=-3000)
                                ),
                                cv.Optional(CONF_STEP, default=1): cv.positive_float,
                                #cv.Optional(CONF_RESTORE_VALUE): cv.boolean,
                            }
                        )

        }
    ).extend(uart.UART_DEVICE_SCHEMA),
  
)

async def to_code(config):
    uart_component = await cg.get_variable(config[CONF_UART_ID])
    var = cg.new_Pvariable(config[CONF_ID],uart_component)
    if CONF_POWER in config:
        sens = await sensor.new_sensor(config[CONF_POWER])
        cg.add(var.set_power_sensor(sens))
    if CONF_BATTERY_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_BATTERY_VOLTAGE])
        cg.add(var.set_battery_sensor(sens))
    if CONF_POWER_SUPPLY in config:
        sens = await number.new_number(config[CONF_POWER_SUPPLY], min_value=-3000, max_value=3000, step=1)
        cg.add(var.set_power_number(sens))
    
    await cg.register_component(var, config)
    #cg.add_library("https://github.com/GitNik1/VEBus", None)
    #if CORE.using_esp_idf:
    #    add_idf_sdkconfig_option("CONFIG_ESP32_WIFI_AMPDU_RX_ENABLED", False)
    #    add_idf_sdkconfig_option("CONFIG_ESP32_WIFI_AMPDU_TX_ENABLED", False)

