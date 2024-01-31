# Parameter `TimeServer`
Default Value: `pool.ntp.org`

!!! Warning
    This is an **Expert Parameter**! Only change it if you understand what it does!

Time server to synchronize system time. If it is disabled or `undefined`, `pool.ntp.org` will be used.
You can also set it to the IP of your router. Many routers like Fritzboxes can act as a local NTP server.
To disable NTP, you need to activate it but set the TimeServer config to be empty (`""`).
In such case the time always starts at `01.01.1970` after each power cycle!
