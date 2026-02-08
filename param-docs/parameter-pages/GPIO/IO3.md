# Parameter `IO3`
Default Value: `input disabled 10 false false`

!!! Warning
    This is an **Expert Parameter**! Only change it if you understand what it does!

This parameter can be used to configure the GPIO `IO3` pin.

!!! Warning
    This pin is by default used for serial communication as RX pin (USB logging)!
    
    **Using this pin for other purposes (I²C, 1-Wire, etc.) will automatically disable USB serial logging**, as the pin cannot serve both functions simultaneously. This is a hardware limitation, not a configuration option.

Parameters:

- `GPIO 3 state`: One of `input`, `input pullup`, `input pulldown` or `output`.
- `GPIO 3 use interrupt`: Enable interrupt trigger
- `GPIO 3 PWM duty resolution`: LEDC PWM duty resolution in bit
- `GPIO 3 enable MQTT`: Enable MQTT publishing/subscribing
- `GPIO 3 enable HTTP`: Enable HTTP write/read
- `GPIO 3 name`: MQTT topic name (empty = `GPIO3`). Allowed characters: `a-z, A-Z, 0-9, _, -`.
