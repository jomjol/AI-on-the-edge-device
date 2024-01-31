# Parameter `IO4`
Default Value: `built-in-led disabled 10 false false`

!!! Warning
    This is an **Expert Parameter**! Only change it if you understand what it does!

This parameter can be used to configure the GPIO `IO4` pin.

!!! Warning
    This pin is only usable with restrictions!
    By default, it is used for build-in flash light (onboard LED).

Parameters:

- `GPIO 4 state`: One of `built-in-led`, `input`, `input pullup`, `input pulldown` or `output`.
- `GPIO 4 use interrupt`: Enable interrupt trigger
- `GPIO 4 PWM duty resolution`: LEDC PWM duty resolution in bit
- `GPIO 4 enable MQTT`: Enable MQTT publishing/subscribing
- `GPIO 4 enable HTTP`: Enable HTTP write/read
- `GPIO 4 name`: MQTT topic name (empty = `GPIO4`). Allowed characters: `a-z, A-Z, 0-9, _, -`.
