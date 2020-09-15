# HeatingControl
Small heating control program

This little program is created to act like a thermostat. In the current state it can control the house heating, and the boiler heating from different sources.
Heaters, pumps can be controlled through GPIOs.
Temperature sensors are the DS18B20-s. You can use wired sensors, and wireless ones. Wireless sensors use MQTT protocoll. 
For monitoring and changing settings you have two options:
    1. Terminal
    2. HeatingControl Android app ( maybe the latest version has not been uploaded yet )

Setting up the system:
    You will need:  1. RaspberryPi ( I have a RaspberryPi 2 B )
                    2. Relays
                    3. Some DS18B20s
                    4. Wire

        On the RaspberryPi:     Linux system
                                Mosquitto library, broker for MQTT
                                1-wire set up
                                WiringPi library
                                SSH
