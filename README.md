# ELEC3300-AirDrums

> **_AUDIO_**: Air Drum project code \
> **_wav extraction script_**: Script for extracting and resampling raw data from .wav files \
> **_..separate work_**: Unmerged code for GUI and IMU that Ivan and Terence were working on\
> **_Project Report_**: Contains powerpoint, demo videos and pictures of our product

## Project Summary
<!-- ![ProjectSummary_Group44_v5](https://user-images.githubusercontent.com/90342420/205483997-fbf42bf1-da19-4e9c-998b-b0f36d36b89f.svg) -->
<!-- ![image](https://user-images.githubusercontent.com/90342420/228476126-04faaeb8-54e4-495c-9526-38fb821523dc.png) -->

![205483997-fbf42bf1-da19-4e9c-998b-b0f36d36b89f](https://user-images.githubusercontent.com/90342420/228476762-aa77ab16-d9b9-4f86-a18a-542bb1d98945.jpg)


Please note that our music player only supports playing music at 22050Hz.

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

