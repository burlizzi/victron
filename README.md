# Victron Component Documentation

## 📖 Introduction

The **Victron** component is an ESPHome integration that enables communication with Victron Energy devices via the UART protocol. It supports control and monitoring of ESS (Energy Storage System) systems and other compatible devices.

## ⚠️ Important Prerequisites

Before using this component, please read carefully:
- **[DISCLAIMER.md](./DISCLAIMER.md)** - Legal liability waiver
- This complete documentation

## 🔧 System Requirements

- **ESPHome**: Version 2023.0 or later
- **Hardware**: Microcontroller with UART support (ESP32, ESP8266, etc.)
- **Serial Connection**: Properly configured UART for the Victron device
- **Dependencies**: ESPHome (external libraries)

## 📦 Installation

### 1. Configuration in Your ESPHome Project

Add the external component to your `configuration.yaml` file:

```yaml
external_components:
    - source: github://burlizzi/victron
      components: [ ess ]
      refresh: 300s
```

### 2. UART Configuration

Configure the UART connection to your Victron device:

```yaml
uart:
  id: uart_bus
  rx_pin: GPIO16          # RX pin for your board
  tx_pin: GPIO17          # TX pin for your board
  flow_control_pin: GPIO4 # Flow control pin (optional)
  baud_rate: 256000       # Standard Victron speed
```

**Note**: Verify the correct pins for your microcontroller in the ESPHome documentation.

### 3. ESS Component Configuration

```yaml
ess:
  id: ess_unit
  uart_id: uart_bus
```

## 🎮 Control and Monitoring

### Switch for ON/OFF Control

Create a template switch to control the system:

```yaml
switch:
  - platform: template
    name: "ESS ON/OFF"
    optimistic: true
    turn_on_action:
      - lambda: |
          auto ess = id(ess_unit);
          ess->on();
    turn_off_action:
      - lambda: |
          auto ess = id(ess_unit);
          ess->off();
```

### Sensors (Example)

Expose data through ESPHome sensors (configure according to your device's capabilities):

```yaml
sensor:
  - platform: template
    name: "ESS Status"
    unit_of_measurement: ""
    lambda: return 0.0;  # Implement reading logic
```

## 🔌 Physical Connections

### Typical UART Connection Diagram

```
Victron Device          ESP Board
─────────────────────────────────────
GND          ────────► GND
RX (out)     ────────► GPIO16 (RX)
TX (in)      ────────► GPIO17 (TX)
Flow Control ────────► GPIO4 (optional)
```

**Important**:
- Verify voltage levels (some devices may require pull-up resistors)
- Use shielded cables to reduce interference
- Keep cables as short as possible

## 🐛 Troubleshooting

### Issue: No Communication with Device

**Solutions**:
1. Verify the configured UART pins
2. Check the baud rate (should be 256000)
3. Verify physical connections and cables
4. Check voltage levels

### Issue: Communication Errors

**Solutions**:
1. Reduce the number of simultaneous UART peripherals
2. Add pull-up resistors (4.7kΩ typically)
3. Shorten UART cables
4. Verify Victron device power supply

### Issue: Device Not Responding to Commands

**Solutions**:
1. Verify the device is powered on and connected
2. Check status in ESPHome logs
3. Try reprogramming the microcontroller
4. Verify Victron device firmware version

## 📊 Complete Configuration Example

```yaml
external_components:
    - source: github://burlizzi/victron
      components: [ ess ]
      refresh: 300s

uart:
  id: uart_bus
  rx_pin: GPIO16
  tx_pin: GPIO17
  flow_control_pin: GPIO4
  baud_rate: 256000

ess:
  id: ess_unit
  uart_id: uart_bus

switch:

  - platform: template
    name: "ESS Charge Only"
    optimistic: true
    turn_on_action:
      - lambda: |
          auto ess = id(ess_unit);
          ess->chargeOnly(true);
    turn_off_action:
      - lambda: |
          auto ess = id(ess_unit);
          ess->chargeOnly(false);

  - platform: template
    name: "ESS ON/OFF"
    optimistic: true
    turn_on_action:
      - lambda: |
          auto ess = id(ess_unit);
          ess->on();
    turn_off_action:
      - lambda: |
          auto ess = id(ess_unit);
          ess->off();
```

## 🔐 Security Notes

- **Do not expose** the component on public networks without authentication
- **Implement** appropriate hardware protections if used in critical systems
- **Always test** in a controlled environment before deployment
- **Continuously monitor** in production

## 📞 Support and Bug Reporting

- **GitHub Issues**: Report bugs and problems
- **ESPHome Documentation**: https://esphome.io/
- **Victron Documentation**: https://www.victronenergy.com/

## 📝 License

See the LICENSE file in the repository for licensing details.

## ⚖️ Legal Disclaimer

This software is provided **WITHOUT WARRANTY**. Please read the [DISCLAIMER.md](./DISCLAIMER.md) before using it, especially if you plan to use it in critical environments.

---

**Documentation Version**: 1.0  
**Last Updated**: 2026-05-11  
**Author**: Burlizzi
