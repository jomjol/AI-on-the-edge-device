# Parameter `ClientCert`
Default Value: `""`

Example: `/config/certs/client.pem.crt`.

!!! Warning
    This is an **Expert Parameter**! Only change it if you understand what it does!

Path to the Client Certificate file.

This is part of the configuration to enable TLS for MQTT.
The Client Certificate is used by the client to prove its identity to the server, in conjunction with the Client Key. 
It is the second part of the MTLS handshake.

Usually there is a one pair of Client Certificate/Key for each client that connects to the MQTT broker

!!! Note
    If set, `ClientKey` must be set too
    This also means that you might have to change the protocol and port in [uri](https://jomjol.github.io/AI-on-the-edge-device-docs/Parameters/#parameter-uri) to `mqtts://example.com:8883`!
