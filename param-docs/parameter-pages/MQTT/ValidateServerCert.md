# Parameter `ValidateServerCert`

Default Value: `true`

!!! Warning
    This is an **Expert Parameter**! Only change it if you understand what it does!

Enable or disable the validation of the server certificate CN field.<br>

If `enabled (true)`, the certificate sent by the server is validated using the configured [Root CA Certificate file](https://jomjol.github.io/AI-on-the-edge-device-docs/Parameters/#parameter-cacert).<br>
The server name in [uri](https://jomjol.github.io/AI-on-the-edge-device-docs/Parameters/#parameter-uri) is compared with the CN field of the server certificate.<br>
A connection is only established if they agree. It ensures the origin of the server.

If `disabled (false)`, the ESP32 skipped any validation of server certificate CN field.<br>
This reduces the security of TLS and makes the *MQTT* client susceptible to MITM attacks.

!!! Note
    This also means that you might have to change the protocol and port in  to `mqtts://example.com:8883`!
	
    If you use public brokers, is recommended to set this parameter to "enabled (true)".
