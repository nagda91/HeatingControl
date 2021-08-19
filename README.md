# HeatingControl
Small heating control program

This little program is created to act like a thermostat. In the current state it can control the house heating, and the boiler heating from different sources.
Heaters, pumps can be controlled through GPIOs.
Temperature sensors are the DS18B20-s. You can use wired sensors, and wireless ones. Wireless sensors use MQTT protocoll (and also the Android app). 
For monitoring and changing settings you have two options:  the Linux terminal and the HeatingControl Android app ( maybe the latest version has not been uploaded yet )
                                                                

    I have:     RaspberryPi 2B
                RaspberryPi compatible relays
                Some DS18B20(+) temperature sensors

    On the RaspberryPi: Linux system
                        Mosquitto library, broker for MQTT
                        1-wire set up
                        WiringPi library
                        SSH
                        FTP

I usually start the program in a separeted Screen through SSH, so it can run in the backround and if it is necessary I can come back to it.

    The program has five threads: the terminal user interface,
                                  the MQTT client,
                                  the basic/core function, it loads the information from the files, and set all the things up,
                                  the temperarure updating function, it refreshs the measured values and monitoring the availableality of the sensors,
                                  the heating function, but it runs only if the heating is ON.








To be continued...
