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

### Pins

In total, 8 pins are used for the notes of the xylophone. One other pin is reserved for the toggling of programming and playing mode.

```c
#define C1_PIN PD3
#define D1_PIN PD4
#define E1_PIN PD5
#define F1_PIN PD6
#define G1_PIN PD7
#define A1_PIN PB0
#define B1_PIN PB1
#define C2_PIN PB2
#define GPIO_PROGRAMM_MODE PC0
```

### MIDI parser

The PC sub-program reads and parses a MID/MIDI file to extract select MIDI events listed in a table below. Once the file is opened, the MIDI file’s header is verified in `verify_header(FILE* file)`. An output file is created where the parsed contents are written. The function `read_track` invokes reading of the individual commands:

```c
void read_track(FILE* midi_file, track_info_t* track, FILE* out_file)
{
    int16_t result = 0;
    while (result != END && result != ERROR_POS) 
    {
        for (int i = 0; i < N_TRACKS; i++)
        {
            result = read_command(midi_file,  &track[i], out_file);
        }
    }
    printf("Parsing result: %d\n", result == END ? SUCCESS : result);
}

int16_t read_command(FILE* midi_file, track_info_t* track, FILE* out_file)
...
```

#### Parsed MIDI events and their codes

| **Event Type**      | **Event Code**  | **Note**   | **MIDI Code** |
|---------------------|-----------------|------------|---------------|
| Note Off            | `0x80`          | C1         | `60`          |
| Note On             | `0x90`          | D1         | `62`          |
| Control Change      | `0xB0`          | E1         | `64`          |
| Track Name          | `0x03`          | F1         | `65`          |
| Instrument Name     | `0x04`          | G1         | `67`          |
| Track End           | `0x2F`          | A1         | `69`          |
| Set Tempo           | `0x51`          | B1         | `71`          |
| Time Signature      | `0x58`          | C2         | `72`          |
| Key Signature       | `0x59`          |            |               |
| SysEx Start         | `0xF0`          |            |               |
| SysEx Restart       | `0xF7`          |            |               |

The new file is sent via UART to the Arduino board and saved in EEPROM. Then, a pin change will be used to switch to the playing mode.

### Interrupt

A change of voltage on a pin triggers an interrupt service routine that is responsible for rebooting the program in either the programming or playing mode.

```c
ISR(PCINT0_vect)
{
	mode = !GPIO_read(&PORTC, GPIO_PROGRAMM_MODE);
	if (mode == 0)
	{
        mode = 1;
	}
	else 
	{
		wdt_enable(0);
		while (1);
	}
}
```

### Playing the xylophone

The function responsible for making the xylophone play is straightforward. It sets the corresponding pin of the note to high:

```c
#define DELAY_MS 40

void xyl_play_note(notes_e note){
    switch (note)
    {
        case C1:
            GPIO_write_high(&PORTD, C1_PIN);
            break;
        case D1:
            GPIO_write_high(&PORTD, D1_PIN);
            break;
            ...
        case NONE:
            GPIO_write_low(&PORTD, C1_PIN);
            GPIO_write_low(&PORTD, D1_PIN);
            ...
            break;
    }
}
```

The function is again called in the main program loop, as indicated in the following snippet. Note that the value is also shown on the OLED display. The delay of 40 milliseconds was chosen as a compromise between delay length and the force of the shooting coils.

```c
song_ptr++;
xyl_play_note(value);
_delay_ms(40);
xyl_play_note(NONE);
display_note(value);
```

## How to use
Connect xylophone to pins 3-10 and to ground, after this, the logical zero will be sent to pin A0 and by that the programming code will be initialized. Next the data will be uploaded to the EEPROM memory via the PC app. After a successful upload, the program for playing the xylophone will begin. The display will show the currently played note and you will be able to listen to its beautiful sound.


## Short video



## Tools

[PlatformIO](https://platformio.org/)

[SimulIDE](https://simulide.com/p/)

[Inkscape](https://inkscape.org/)

## References

[Standard MIDI-File Format Spec. 1.1, updated](https://www.music.mcgill.ca/~ich/classes/mumt306/StandardMIDIfileformat.html)

Digital Electronics 2 presentation by Tomas Fryza
