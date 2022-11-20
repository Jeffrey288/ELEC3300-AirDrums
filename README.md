# ELEC3300-Project

## to do list for imu
- roll SHOULD NOT depend on pitch
- pitch should not depend on yaw
- find some way to use mahony
- use roll data to determine rod orientation and hence determine vertial-component acceleration
- add calibration sequence

## To-do list
- (Ivan) board drawing
- music playing and mixing (headphone jack)
- analog pedal (tof + uart)
- imu data processing (digital)
- sd card storage (sd card reader)
- some buttons (init, start recording, stop recording)

# ELEC 3300 – Introduction to Embedded Systems
## Peer Discussion Worksheet

Project Group Number: **44**
1.	Please describe your Project in 120 to 150 words, which includes 
•	Functional description of your project in laymen terms
•	Role of the microcontroller

- We would like to build an air dum set.
- Basically, we have two drum sticks equipped with gyroscope sensors. The user will swing the drum sticks as if they're playing real drums, and based on how they orient their drumsticks, different drum sounds will be played.
- There will also be two pedals for the bass.
- The microcontroller will be responsible for processing the gyroscope data and playing the appropiate sounds.
- It will also provide utility functions such as metronome, playing the drum sounds through AUX port or by connecting to a bluetooth speaker (tentative).
- More tentative feature if time allows:
  - playing music from sd card while performing
  - recording the performance and stroing into sd card
  - wireless drumsticks using nrf
  - a button for muting

2.	List of hardware (like sensors, device, motor that you might need for your project)

For the drumsticks,
- two gyroscopes for each drum stick
- long wire connecting to the mainboard

For the mainboard,
- lcd for displaying what drums are hit and for setting up the system
- buttons for adjusting settings
- - sd card for data storage of sound

3.	Can the STM32F103 Development Board satisfy your Project need?  
If not, what development board you intend to use? 

- should be enough
- we might draw our own boards

