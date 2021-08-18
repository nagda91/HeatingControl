# HeatingControl
Small heating control program

This little program is created to act like a thermostat. In the current state it can control the house heating, and the boiler heating from different sources.
Heaters, pumps can be controlled through GPIOs.
Temperature sensors are the DS18B20-s. You can use wired sensors, and wireless ones. Wireless sensors use MQTT protocoll (and also the Android app). 
For monitoring and changing settings you have two options:  the Linux terminal and the HeatingControl Android app ( maybe the latest version has not been uploaded yet )
                                                                

    I have:     RaspberryPi ( I have a RaspberryPi 2 B )
                RaspberryPi compatible relays
                Some DS18B20(+) temperature sensors

    On the RaspberryPi: Linux system
                        Mosquitto library, broker for MQTT
                        1-wire set up
                        WiringPi library
                        SSH
                        FTP

To be continued...
