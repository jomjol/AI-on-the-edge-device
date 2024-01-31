# Parameter `IO1`
Default Value: `input disabled 10 false false`

!!! Warning
    This is an **Expert Parameter**! Only change it if you understand what it does!

This parameter can be used to configure the GPIO `IO1` pin.

!!! Warning
    This pin is by default used for the serial communication as TX pin (USB logging)!

Parameters:

- `GPIO 1 state`: One of `input`, `input pullup`, `input pulldown` or `output`.
- `GPIO 1 use interrupt`: Enable interrupt trigger
- `GPIO 1 PWM duty resolution`: LEDC PWM duty resolution in bit
- `GPIO 1 enable MQTT`: Enable MQTT publishing/subscribing
- `GPIO 1 enable HTTP`: Enable HTTP write/read
- `GPIO 1 name`: MQTT topic name (empty = `GPIO1`). Allowed characters: `a-z, A-Z, 0-9, _, -`.
