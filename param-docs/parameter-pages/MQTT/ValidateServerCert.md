# Parameter `ValidateServerCert`

Default Value: `true`

!!! Warning
    This is an **Expert Parameter**! Only change it if you understand what it does!

Enable or disable the validation of the server certificate.

If `enabled (true)`, the certificate sent by the server is validated using the configured [Root CA Certificate file](https://jomjol.github.io/AI-on-the-edge-device-docs/Parameters/#parameter-cacert).<br>
The server name in [uri](https://jomjol.github.io/AI-on-the-edge-device-docs/Parameters/#parameter-uri) is compared with the CN field of the server certificate.<br>
A connection will be only established if these match.<br>
It ensures the origin of the server.

If `disabled (false)`, only the validity of the certificate (e.g. expiry) is checked, not the origin (CN field).<br>
If you use public brokers, make sure to set this parameter to "enabled", to avoid potential MITM-Attacks!

!!! Note
    This also means that you might have to change the protocol and port in  to `mqtts://example.com:8883`!
