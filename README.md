# DE2 project: Electronically controlled xylophone

## Table of Contents
- [Original assignment](#original-assignment)
- [Team](#team)
- [Theoretical Description and Explanation](#theoretical-description-and-explanation)
- [Hardware description](#hardware-description)
- [Software Description](#software-description)
- [How to use](#how-to-use)
- [Video](#short-video)
- [Tools](#tools)
- [References](#references)

## Original assignment

Build an electronic xylophone instrument that can be played using an Arduino microcontroller. The project will combine hardware and software components to create an interactive and programmable musical instrument capable of producing different tones and melodies. Incorporating a graphical user interface (GUI) to visualize the notes being played.

## Team
    Martin G:
    Matúš C:
    Jonáš H:
    Max G:

## Theoretical description and explanation
Max: *We wanna play a xylophone, but at the same time we don't! We want it to play by itself...*

The project involves programming an Arduino microcontroller. The microcontroller will control a xylophone that can play 8 keys (C1-C2, 1 octave) using coils. We decided to expand the functionality to enable automatic playing of any MID/MIDI file. This requires an additional pre-processing subprogram to extract the relevant and usable information from the file and an EEPROM memory to store the note instructions on the microcontroller. Relevant information such as the note being played will be displayed on an OLED display.

The MID file is first processed in a C program on PC. The [MIDI format](https://www.music.mcgill.ca/~ich/classes/mumt306/StandardMIDIfileformat.html) is an extensive specification. Only a few of the events (note on, key, note length, etc) are relevant to our use case. This means that much of the MID file can be erased. For the practicality of debugging and also the limited resources of the microcontroller, the pre-processing is done on PC.

The pre-processed data is sent to the microcontroller via USB. For PC - Arduino communication, UART is used. The blocks of data are stored in the board's memory so they can be sent via I2C to the EEPROM. Once the song is saved on EEPROM, no communication between the PC and the board is necessary. The data transfer "programming" mode and playing mode is determined by a shorted pin on the board. In the playing mode, I2C is again used to load the blocks of data from EEPROM. 

## Hardware description
#### Xylophone
<img src="https://github.com/user-attachments/assets/db7c0745-f5c5-4218-b780-6ea34aa5dd7e" width="400"/>

##
#### OLED display
<img src="https://github.com/user-attachments/assets/00d42a6d-9a74-4857-b003-9d14eb6a1511" width="400"/>
<img src="https://github.com/user-attachments/assets/08feb6dd-066e-454e-af2b-172b3e059c08" width="400"/>

##
#### Clock: DS3231SN 0912A3 208AB (larger), EEPROM: ATHYC532 (smaller)
<img src="https://github.com/user-attachments/assets/13526339-5759-4702-8f3d-8d95bcd76df1" width="400" height="260"/>
<img src="https://github.com/user-attachments/assets/ec665f28-bb57-4d0e-bc47-78641f2cdeb4" width="400"/>

##
#### Arduino + breadboard
<img src="https://github.com/user-attachments/assets/c600a844-5449-4f9d-9961-a646d0627320" width="400"/>


## Software description

This is the structure of the project's source files including the libraries and PC subprogram:

    .
    ├── include
    │     ├── pins.h        # Arduino pins
    │     ├── structs.h     # Enum of note, pause and end events
    │     └── ...
    ├── lib
    │     ├── gpio            # GPIO library (c) Tomas Fryza
    │     ├── midi            
    │     ├── oled            # OLED display library (c) Skie-Systems
    │     ├── twi             # I2C/TWI library (c) Tomas Fryza
    │     └── uart            # UART library with r/t circular buffers (c) Peter Fleury
    ├── src
    │     ├── main.c          # main program loop, init logic and interrupts
    │     ├── display         # functions to display data on OLED
    │     ├── eeprom          # EEPROM read/write functions
    │     └── xylophone       # play note functions
    ├── pc                    # PC program for MID pre-processing
    └── platformio.ini        # PlatformIO framework config

### MIDI parser

### Event mapping

### Data transfer

### Details: interrupt, timing, block size

## How to use
Connect xylophone to pins 2-9 and to ground, after this, the logical zero will be sent to pin A0 and by that the programming code will be initialized. Next the data will be uploaded to the EEPROM memory via the PC app. After a successful upload, the program for playing the xylophone will begin. The display will show the currently played note and you will be able to listen to its beautiful sound.


## Short video



## Tools

[PlatformIO](https://platformio.org/)

[SimulIDE](https://simulide.com/p/)

[Inkscape](https://inkscape.org/)

## References

[Standard MIDI-File Format Spec. 1.1, updated](https://www.music.mcgill.ca/~ich/classes/mumt306/StandardMIDIfileformat.html)

Digital Electronics 2 presentation by Tomas Fryza
