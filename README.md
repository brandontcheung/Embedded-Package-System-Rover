# Embedded-Package-System-Rover

Written in C and developed on 5 TI 3200 Logic boards; the system was composed of a rover, two robotic arms, two ultrasonic sensors, a pixy cam 2.0, and a MQTT server. The ultimate purpose of the system was to be a reduced scale model of a package identification and transfer system with the full scale system having multiple delivery and sorting stations communicating via a central MQTT server. In figure 1 below you can see the general conifugrations of the system and some of it's design goals.


Figure 1:
![Embedded System Design Team 11 Final Poster-1](https://github.com/brandontcheung/Embedded-Package-System-Rover/assets/17488525/676c63f4-0d4f-45a8-8eda-c8cbd253100d)

An overview of the Triangle rover, and technical information about the specific programming methods used in the hardware interfacing, can be seen below in Figure 2. The whole system communicated via a MQTT server using an Mosquito Library to interface with a single server running off of a Raspberry Pi 3. From this MQTT Server the Rover recieved generalized commands that it used to determine the necessary motor commands needed to move the rover in the correct direction.

Figure 2:
![Slide1](https://github.com/brandontcheung/Embedded-Package-System-Rover/assets/17488525/134ed1df-cab6-4c9b-993f-69d5de9c9387)

Due to the triangular nature of the rover a PID (Proportional, Intergral, Derivative) Algorithm was used to ensure the correct velocity of the rover at all times to ensure correct movement despite any slipping that may occur due to the omni-wheels present on the rover.

This project stands as a proof of concept about the effectiveness of these types of sorting systems and there eventual commercial use moving foreward.
