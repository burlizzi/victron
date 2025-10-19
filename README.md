# victron

example:
```
external_components:
    - source: github://burlizzi/victron
      components: [ ess ]
      refresh: 300s

uart:
  id: uart_bus
  rx_pin:  GPIO16
  tx_pin:  GPIO17
  flow_control_pin: GPIO4
  baud_rate: 256000

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
ess:
  id: ess_unit
  uart_id: uart_bus
  ```