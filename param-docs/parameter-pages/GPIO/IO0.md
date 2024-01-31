# Parameter `IO0`
Default Value: `input disabled 10 false false`

!!! Warning
    This is an **Expert Parameter**! Only change it if you understand what it does!

This parameter can be used to configure the GPIO `IO0` pin.

!!! Warning
    This pin is only usable with restrictions!
    It must be disabled when the camera is used.
    Additionally, it is used to activate Bootloader mode and must therefore be HIGH after a reset!

Parameters:

- `GPIO 0 state`: One of `input`, `input pullup`, `input pulldown` or `output`.
- `GPIO 0 use interrupt`: Enable interrupt trigger
- `GPIO 0 PWM duty resolution`: LEDC PWM duty resolution in bit
- `GPIO 0 enable MQTT`: Enable MQTT publishing/subscribing
- `GPIO 0 enable HTTP`: Enable HTTP write/read
- `GPIO 0 name`: MQTT topic name (empty = `GPIO0`). Allowed characters: `a-z, A-Z, 0-9, _, -`.
