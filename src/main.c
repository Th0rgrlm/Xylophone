#include <avr/io.h>
#include "constants.h"
#include <util/delay.h>
#include "error_codes.h"
#include "structs.h"
#include "xylophone/xyl.h"
#include <display/disp.h>
#include "eeprom/eeprom.h"
#include <string.h>
#include "uart.h"
#include "stdio.h"
#include "twi.h"
#include "avr/interrupt.h"
#include "gpio.h"
#include "pins.h"
#include <avr/wdt.h>

/**
 * @brief Initialization function
 * 
 */
void init(void);


/**
 * @brief Main loop function
 * 
 */
void loop(void);

/**
 * @brief Creates a custom delay b/c the _delay_ms function REQUIRES a COMPILE TIME CONSTANT for some ******* reason
 * 
 * @param delay Delay in ms
 */
void delay_ms(uint16_t delay);

/**
 * @brief Initializes auxiliary pins
 * 
 */
void pin_init(void);

/**
 * @brief Fetches the song from the PC
 * 
 */
void song_fetch(void);

/**
 * @brief Plays a single note on the xylophone
 * 
 * @param note Note to be played
 */
void play_note(notes_e note);

/**
 * @brief Plays the song on EEPROM
 * 
 * @return int32_t error code the song has stopped playing with
 */
int32_t song_play(void);

/**
 * @brief Loads the song from EEPROM
 * 
 * @return int32_t Error code
 */
int32_t song_load(void);

/**
 * @brief Set the programming mode or resets the device
 * 
 * @param reset_on_change Whether board should reset on pin change
 */
void set_mode(uint8_t reset_on_change);

static uint8_t mode = 0; /** @brief Current Arduino mode */

// uint8_t song[] = {
// 	C1,
// 	NONE, 0x00, 0x80,
// 	D1,
// 	NONE, 0x00, 0x80,
// 	E1,
// 	NONE, 0x00, 0x80,
// 	F1,
// 	NONE, 0x00, 0x80,
// 	G1,
// 	NONE, 0x00, 0x80,
// 	A1,
// 	NONE, 0x00, 0x80,
// 	B1,
// 	NONE, 0x00, 0x80,
// 	C2,
// 	NONE, 0x01, 0x00,
// 	END
// };


uint8_t song[SONG_SIZE]; /** @brief Currently playing song */

int main(void)
{
	init();
    while (1) loop();

    return SUCCESS;
}


void init(void)
{
	uart_init(UART_BAUD_SELECT(115200, F_CPU)); // Enable UART
	sei(); // Enable global interrupts
	// pin_init(); // Initialize programming mode pin
	// GPIO_mode_input_pullup(&PORTD, GPIO_PROGRAMM_MODE); // Set programming pin to pullup (default high)
	xyl_init(); // Initialize xylophone pins
	twi_init(); // Initialize TWI
	// PCMSK1 |= (0b1 << PCINT8); // Enable pin change interrupt
	// PCICR |= (0b1 << PCIE1); // Enable block pin change interrupt
	display_init(); // Initialize display
	// if (mode) uart_putc('X');
	// else uart_putc('Y');
	// mode = GPIO_read(&PORTD, GPIO_PROGRAMM_MODE) ? 0 : 1; // Set initial mode
	// if (mode) uart_putc('X');
	// else uart_putc('Y');
	return;
}

void loop(void)
{
	song_fetch(); // Fetch song from UART
	
	// static uint8_t* song_ptr = song;
	// if (*song_ptr != END)
	// {
	// 	uint8_t value = *song_ptr;
	// 	if (value == NONE)
	// 	{
	// 		uint16_t delay = *(song_ptr + 1) * 256;
	// 		delay += *(song_ptr + 2);
	// 		delay *= 4;
	// 		song_ptr += 3;
	// 		delay_ms(delay);
	// 	}
	// 	else
	// 	{
	// 		song_ptr++;
	// 		xyl_play_note(value);
	// 		_delay_ms(40);
	// 		xyl_play_note(NONE);
	// 		display_note(value);
	// 	}
	// }
	
	song_play(); // Play song
	return;
}

int32_t song_play(void)
{
	uint16_t delay = 0; // Delay value
	note_status_e status = STATUS_NOTE; // Status value
	while (status != STATUS_END)
	{
		if (song_load()) return ERROR_SONG_LOAD; // Load song
		uint16_t song_pos = 0; // Song pointer
		while (song_pos < SONG_SIZE && status != STATUS_END) // While in song buffer and not at the song end
		{
			note_status_e new_status = status;
			switch(status)
			{
				case STATUS_NOTE: // On note/delay specifier:
				{
					notes_e value = (notes_e)song[song_pos++];
					if (value == NONE) // On delay specifier
					{
						new_status = STATUS_DELAY_1; // Next status is 1st delay byte
						break;
					}
					else if (value == END) // On song end
					{
						new_status = END; // End of the song
						break;
					}
					else if (value <= C2 && value >= C1) // On note specifier
					{
						play_note(value); // Play note
						break;
					}
					else
					{
						char str[64];
						snprintf(str, 64, "Invalid song character: 0x%02X, aborting", value);
						uart_puts(str); // Write error info to UART
						new_status = STATUS_END; // End  the song
					}
				}
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
				case STATUS_END: // On song end
				{
					return SUCCESS; // Stop song play
				}
			}
			status = new_status; // Update status
		}
	}
	return SUCCESS;
}

void play_note(notes_e note)
{
	xyl_play_note(note); // Turn on note
	delay_ms(40); // Wait for 40 ms
	xyl_play_note(NONE); // Turn off all notes
	display_note(note); // Display played note
	return;
}

int32_t song_load(void)
{
	static uint32_t pos = 0; // Position of the song in EEPROM to load from
	int32_t result = eeprom_read(pos, song, SONG_SIZE); // Load SONG_SIZE bytes of song
	pos += SONG_SIZE; // Increment pos pointer
	return result; // Return reading result
}

void song_fetch(void)
{
	static uint8_t eeprom_list_ptr = 0; // Pointer to EEPROM memory for writing
	while (1)
	{

		uint8_t rx_buffer[UART_RX_BUFFER_SIZE];
		uint8_t end_reached = 0;
		for (int i = 0; i < UART_RX_BUFFER_SIZE; i++)
		{
			uint16_t value;
			while (((value = uart_getc()) & 0x0100));
			rx_buffer[i] = (uint8_t)value; // Get character
			static uint8_t val = 0;
			if (!val) // If not in delay reading
			{
				switch((notes_e)rx_buffer[i])
				{
					case NONE: // On delay
					{
						val = 2; // Ignore next 2 bytes for checking purposes
						break;
					}
					case END:
					{
						end_reached = 1; // End reached
						break;
					}
					default: break;
				}
			}
			else {val--;} // Byte checking ignored
		}
		// Send to EEPROM
		for (int i = 0; i < UART_RX_BUFFER_SIZE / 8; i++)
		{
			eeprom_write(eeprom_list_ptr * UART_RX_BUFFER_SIZE + 8 * i, &rx_buffer[8 * i], 8);
		}
		eeprom_list_ptr++;
		if (end_reached) // On transmission end
		{
			uart_putc('A'); // End transmission
			return;
			while (1)
			{
				printf("Abort");
				delay_ms(1000);
			}
			uart_puts("Mode 1\n");	
		}
		else uart_putc('C'); // Ask for another chunk
	}
	return;
}

void delay_ms(uint16_t delay)
{
	while (delay--)
	{
		_delay_ms(1);
	}
}

void pin_init(void)
{
	GPIO_mode_input_pullup(&PORTD, GPIO_PROGRAMM_MODE); // Set programming pin to pullup (default high)
	return;
}	

void set_mode(uint8_t reset_on_change)
{
	mode = !GPIO_read(&PORTD, GPIO_PROGRAMM_MODE);
	if (mode == 0)
	{
		mode = 1;
		uart_puts("Changed to mode 1\n");
	}
	else if (reset_on_change)
	{
		mode = 0; // Should be unnecessary
		wdt_enable(0); // Enable watchdog
		while (1); // Reset device
	}
}


/**
 * @brief Interrupt Service Routine for changing the mode on pin change
 * 
 */
ISR(INT0_vect)
{
	uart_puts("ISR");
	set_mode(1);
}