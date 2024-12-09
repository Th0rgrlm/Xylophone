# DE2 project: Electronically controlled xylophone

## Table of Contents
- [Original assignment](#original-assignment)
- [Team](#team)
- [Theoretical Description and Explanation](#theoretical-description-and-explanation)
- [Hardware description](#hardware-description)
- [Software Description](#software-description)
- [What we learned](#what-we-learned)
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

The project involves programming an Arduino microcontroller. The microcontroller will control a xylophone that can play 8 keys (C1-C2, 1 octave) using coils. We decided to expand the functionality to enable automatic playing of any MID/MIDI file. This requires an additional pre-processing subprogram to extract the relevant and usable information from the file and an EEPROM memory to store the note instructions on the microcontroller. The note being played is to be displayed on an OLED display.

The MID file is first processed in a program on PC, written in C. The [MIDI format](https://www.music.mcgill.ca/~ich/classes/mumt306/StandardMIDIfileformat.html) is an extensive specification and only a few of the events (note on, key, note length, etc) are relevant to our use case. This means that much of the MID file can be erased. For the practicality of debugging and also beacause of the limited resources of the microcontroller, pre-processing is done on PC.

The data saved in a file with custom extension `.MXY` is sent to the microcontroller via USB. For PC - Arduino communication, UART is used. The blocks of data are stored in the board's RAM so they can be sent via I<sup>2</sup>C to the EEPROM. Once the song is saved on EEPROM, no communication between the PC and the board is necessary. In our implementation, the playing mode immediately follows the upload process. I<sup>2</sup>C is used to load the blocks of data from EEPROM (and memory) and signals are sent to the xylophone pins. 

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
    │     ├── constants.h   # Buffer size constants
    │     └── error_codes.h # Error codes definitions
    ├── lib
    │     ├── gpio            # GPIO library (c) Tomas Fryza
    │     ├── oled            # OLED display library (c) Skie-Systems
    │     ├── twi             # I2C/TWI library (c) Tomas Fryza
    │     └── uart            # UART library with r/t circular buffers (c) Peter Fleury
    ├── src
    │     ├── main.c          # main program loop, init logic and interrupts
    │     ├── display         # functions to display data on OLED
    │     ├── eeprom          # EEPROM read/write functions
    │     └── xylophone       # play note functions
    ├── pc                    # PC utility for MID pre-processing
    |     ├── main.c          # MID convertor to MXY
    |     ├── uart_pc.h       # PC UART communication with AVR
    │     └── error_codes.h   # Error codes definitions for PC utility
    └── platformio.ini        # PlatformIO framework config

### Pins <sup>[pins.h](https://github.com/Th0rgrlm/Xylophone/blob/main/include/pins.h)</sup>

In total, 8 pins are used for the notes of the xylophone. One other pin is reserved for the eventual manual control of the programming and playing mode.

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

### MIDI parser <sup>[pc/main.c](https://github.com/Th0rgrlm/Xylophone/blob/main/pc/main.c)</sup>

The PC sub-program reads and parses a MID/MIDI file to extract select MIDI events listed in a table below. Once the file is opened, the MIDI file’s header is verified in `verify_header(FILE* file)`. An output file with the extension `.mxy` is created and the parsed contents are written there. The function `read_track` invokes reading of the individual commands:

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
The PC app is a console app and requires two additional arguments to be run. The first one is exact path of the file to be converted, the second one is COM port number the converted should be sent to. After that, the program will confirm the supplied file is an actual .mid file and converts it to a .mxy file. After that, the UART communication between PC and Arduino will begin. The file is sent in chunks of 256 bytes (because of RX receive buffer of the AVR). After each chunk is sent, Arduino checks for track end and writes the chunk to EEPROM. If no track end was found, it requests another chunk by sending `'C'`, otherwise it ends the communication by sending `'A'`.

### MIDI events and their codes

| **Event Type**      | **Event Code**  | **Note**   | **MIDI Code** | **MXY Code**  | 
|---------------------|-----------------|------------|---------------|---------------|
| Note Off            | `0x80`          | C1         | `60`          | `1`           |
| Note On             | `0x90`          | D1         | `62`          | `2`           |
| Control Change      | `0xB0`          | E1         | `64`          | `3`           |
| Track Name          | `0x03`          | F1         | `65`          | `4`           |
| Instrument Name     | `0x04`          | G1         | `67`          | `5`           |
| Set Tempo           | `0x51`          | A1         | `69`          | `6`           |
| Time Signature      | `0x58`          | B1         | `71`          | `7`           | 
| Key Signature       | `0x59`          | C2         | `72`          | `8`           |
| SysEx Start         | `0xF0`          | Track End  | `0x2F`        | `0xFF`        |
| SysEx Restart       | `0xF7`          | Delay      |               | `0x00 D1 D2`  |

The new file in .mxy format is sent via UART to the Arduino board and saved in EEPROM (function `song_fetch()` on AVR). Then, the activation of playing mode is done by `song_play()`.

### Interrupt

The original intention was to manually toggle between programming and playing mode by changing the voltage on a pin. An interrupt service routine would reset the program in either of the modes. Due to time constraints, this functionality was left out.

### Playing the xylophone <sup>[xyl.c](https://github.com/Th0rgrlm/Xylophone/blob/main/src/xylophone/xyl.c), [main.c](https://github.com/Th0rgrlm/Xylophone/blob/main/src/main.c)</sup>

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

The function is called again after a note is played in the main program loop, as indicated in the following snippet. Calling `xyl_play_note(notes_e note)` with `NONE` argument sets all note pins to low.

```c
song_ptr++;
xyl_play_note(value);
_delay_ms(40);
xyl_play_note(NONE);
display_note(value);
```

The delay of 40 milliseconds was chosen as a compromise between delay length and the force of the shooting coils. Do not confuse this delay with the delay *between notes*, which is specified in the Delay message (code from `main.c`):

```c
case STATUS_DELAY_1: // On 1st delay byte
{
	delay += 256 * song[song_pos++]; // Get 1st delay byte
	new_status = STATUS_DELAY_2; // Next byte is 2nd byte of delay
	break;
}
case STATUS_DELAY_2: // On 2nd delay byte
{
	delay += song[song_pos++]; // Get 2nd delay byte
	delay_ms(delay); // Play delay
	delay = 0; // Reset delay
	new_status = STATUS_NOTE; // Next byte is note specifier
	break;
}
```

The tempo of the song can be easily changed in the MID file header. Tempo has the identifier `0x51`.

## What we learned

Delay function's argument must be a copmile time constant.
```c
void delay_ms(uint16_t delay)
{
	while (delay--)
	{
		_delay_ms(1);
	}
}
```

Xylophone can't play notes simultaneously. There is an audible delay of a few ms.

EEPROM page size is tiny (8 bytes).

## How to use
Connect xylophone to pins 3-10 and to ground, after this, the logical zero will be sent to pin A0 and by that the programming code will be initialized. Next the data will be uploaded to the EEPROM memory via the PC app. After a successful upload, the program for playing the xylophone will begin. The display will show the currently played note and you will be able to listen to its beautiful sound.

add info how to use parser and how to prepare and export notes


## Short video



## Tools

[PlatformIO](https://platformio.org/)

[Make](https://www.make.com/en)

[Inkscape](https://inkscape.org/)

[MuseScore 4](https://musescore.org/) - sheet music editor with MID export

[pianotify](https://pianotify.com) - online MID player

[bitmidi.com](https://bitmidi.com) - free MID files

## References

[Standard MIDI-File Format Spec. 1.1, updated](https://www.music.mcgill.ca/~ich/classes/mumt306/StandardMIDIfileformat.html)

Digital Electronics 2 presentation by Tomas Fryza
