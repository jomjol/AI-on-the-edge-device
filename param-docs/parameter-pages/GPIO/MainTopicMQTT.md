# Parameter `MainTopicMQTT`
Default Value: `wasserzaehler/GPIO`

!!! Note
    This parameter is not accessible through the Web Interface Configuration Page!

The GPIO Interface is prepared to report it's status and status changes as a MQTT topic. With this parameter you configure the MQTT main topic, under which the status is published.
As this parameter is still experimental it can only be set manually in the `config.ini` itself and has not been tested in detail so far.
