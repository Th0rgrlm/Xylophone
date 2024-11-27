# DE2 project: Electronically controlled xylophone

## Table of Contents
- [Original assignment](#original-assignment)
- [Team](#team)
- [Theoretical Description and Explanation](#theoretical-description-and-explanation)
- [Hardware description](#hardware-description)
- [Software Description](#software-description)
- [How to use](#how-to-use)
- [Video](#short-video)
- [Tools and References](#tools-and-references)

## Original assignment

Build an electronic xylophone instrument that can be played using an AVR microcontroller. The project will combine hardware and software components to create an interactive and programmable musical instrument capable of producing different tones and melodies. Incorporating a graphical user interface (GUI) to visualize the notes being played.

## Team
    Martin G:
    Matúš C:
    Jonáš H:
    Max G:

## Theoretical description and explanation
We wanna play a xylophone, but at the same time we don't! We want it to play by itself...

There will be aa MID-file from D disc, on the computer it will be parsed, then this will sent to the Arduino board via UART and will be loaded to the EEPROM memory. After that it will be gradually sent to the xylophone via I2C.



## Hardware description
### Xylophone
<img src="https://github.com/user-attachments/assets/db7c0745-f5c5-4218-b780-6ea34aa5dd7e" width="400"/>

##
### OLED display
<img src="https://github.com/user-attachments/assets/00d42a6d-9a74-4857-b003-9d14eb6a1511" width="400"/>
<img src="https://github.com/user-attachments/assets/08feb6dd-066e-454e-af2b-172b3e059c08" width="400"/>

##
### Clock: DS3231SN 0912A3 208AB
### EEPROM: ATHYC532
<img src="https://github.com/user-attachments/assets/13526339-5759-4702-8f3d-8d95bcd76df1" width="400" height="260"/>
<img src="https://github.com/user-attachments/assets/ec665f28-bb57-4d0e-bc47-78641f2cdeb4" width="400"/>

##
### Arduino + breadboard
<img src="https://github.com/user-attachments/assets/c600a844-5449-4f9d-9961-a646d0627320" width="400"/>


## Software description
abych nezapomnel:

parse on PC -> using UART to board's RAM (block-wise) -> using I2C to EEPROM -> load blocks from EEPROM (to RAM) and play


## How to use
Connect xylophone to pins 2-9 and to ground, after this, the logical zero will be sent to pin A0 and by that the programming code will be initialized. Next the data will be uploaded to the EEPROM memory via the PC app. After a successful upload, the program for playing the xylophone will begin. The display will show the currently played note and you will be able to listen to its beautiful sound.


## Short video



## Tools and References

[PlatformIO](https://platformio.org/)

[SimulIDE](https://simulide.com/p/)

[Inkscape](https://inkscape.org/)

Digital Electronics 2 presentation by Tomas Fryza
