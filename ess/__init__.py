import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.core import CORE
from esphome.components.esp32 import add_idf_sdkconfig_option
from esphome.const import CONF_NAME, CONF_UART_ID
from esphome.components import uart

DEPENDENCIES = ["uart"]
AUTO_LOAD = []

victron_ns = cg.esphome_ns.namespace("victron")
Ess = victron_ns.class_("Ess", cg.Component, uart.UARTDevice)



CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(Ess),
        }
    ).extend(uart.UART_DEVICE_SCHEMA),
  
)

async def to_code(config):
    uart_component = await cg.get_variable(config[CONF_UART_ID])
    var = cg.new_Pvariable(config[CONF_ID],uart_component)
    
    await cg.register_component(var, config)
    #cg.add_library("https://github.com/GitNik1/VEBus", None)
    #if CORE.using_esp_idf:
    #    add_idf_sdkconfig_option("CONFIG_ESP32_WIFI_AMPDU_RX_ENABLED", False)
    #    add_idf_sdkconfig_option("CONFIG_ESP32_WIFI_AMPDU_TX_ENABLED", False)

