# Parameter `CACert`
Default Value: `""`

Example: `/config/certs/RootCA.crt`.

!!! Warning
    This is an **Expert Parameter**! Only change it if you understand what it does!

Path to the CA certificate file.

This is part of the configuration to enable TLS 1.2 for MQTT.<br>

The CA Certificate is used by the client to validate the broker is who it claims to be.
It allows the client to authenticate the server, which is the first part of the MTLS handshake.

Usually there is a common RootCA certificate for the MQTT broker.
More information is available [here](https://jomjol.github.io/AI-on-the-edge-device-docs/MQTT-API/#mqtt-tls).

For more information on how to create your own certificate, see: [mosquitto.org](https://mosquitto.org/man/mosquitto-tls-7.html) or [emqx.com](https://www.emqx.com/en/blog/emqx-server-ssl-tls-secure-connection-configuration-guide).

!!! Note
    This also means that you might have to change the protocol and port in [uri](https://jomjol.github.io/AI-on-the-edge-device-docs/Parameters/#parameter-uri) to `mqtts://example.com:8883`!

    Only Certificates up to 4096 Bit are supported!
