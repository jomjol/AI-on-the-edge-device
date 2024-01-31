# Parameter `CACert`
Default Value: `""`

Example: `/config/certs/RootCA.pem`.

!!! Warning
    This is an **Expert Parameter**! Only change it if you understand what it does!

Path to the CA certificate file.

This is part of the configuration to enable TLS for MQTT.
The CA Certificate is used by the client to validate the broker is who it claims to be.
It allows the client to authenticate the server, which is the first part of the MTLS handshake.

Usually there is a common RootCA certificate for the MQTT broker

!!! Note
    This also means that you might have to change the protocol and port in [uri](https://jomjol.github.io/AI-on-the-edge-device-docs/Parameters/#parameter-uri) to `mqtts://example.com:8883`!
