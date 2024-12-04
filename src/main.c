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


uint8_t song[128]; /** @brief Currently playing song */

int main(void)
{
	init();
    uart_puts("Hello, world!\n");
	eeprom_read(0, song, 128);
	uart_puts("EEPROM read done\n");
	for (int i = 0; i < 128; i++)
	{
		char c[4];
		snprintf(c, 3, "%02X ", song[i]);
		uart_puts(c);
	}

	// memset(song, 0, 4);
	// eeprom_read(0, song, 4);
	// uart_puts("EEPROM read done\n");
	// uint8_t str[16];
	// snprintf((char*)str, 16, "song: 0x%2X", song[3]);
	// uart_puts((char*)str);
    while (1) loop();

    return SUCCESS;
}


void init(void)
{
	pin_init(); // Initialize programming mode pin
	xyl_init(); // Initialize xylophone pins
	twi_init();
	uart_init(UART_BAUD_SELECT(115200, F_CPU));
	PCMSK1 |= (0b1 << PCINT8); // Enable pin change interrupt
	PCICR |= (0b1 << PCIE1); // Enable block pin change interrupt
	sei();
	display_init(); // Initialize display
}

void loop(void)
{
	if (mode == 1)
	{
		uart_puts("Mode 1");
		static uint8_t eeprom_list_ptr = 0; // Pointer to EEPROM memory for writing
		uint8_t rx_buffer[UART_RX_BUFFER_SIZE];
		uint8_t end_reached = 0;
		for (int i = 0; i < UART_RX_BUFFER_SIZE; i++)
		{
			rx_buffer[i] = uart_getc(); // Get character
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
		eeprom_write(eeprom_list_ptr++ * UART_RX_BUFFER_SIZE, rx_buffer, UART_RX_BUFFER_SIZE);
		if (end_reached) // On transmission end
		{
			uart_putc('A'); // End transmission
			while (1)
			{
				printf("Abort");
				delay_ms(1000);
			}
		}
		else uart_putc('C'); // Ask for another chunk
		return;
	}
	static uint8_t* song_ptr = song;
	if (*song_ptr != END)
	{
		uint8_t value = *song_ptr;
		if (value == NONE)
		{
			uint16_t delay = *(song_ptr + 1) * 256;
			delay += *(song_ptr + 2);
			delay *= 4;
			song_ptr += 3;
			delay_ms(delay);
		}
		else
		{
			song_ptr++;
			xyl_play_note(value);
			_delay_ms(40);
			xyl_play_note(NONE);
			display_note(value);
		}
	}
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


/**
 * @brief Interrupt Service Routine for changing the mode on pin change
 * 
 */
ISR(INT0_vect)
{
	uart_puts("ISR");
	mode = !GPIO_read(&PORTD, GPIO_PROGRAMM_MODE);
	if (mode == 0) mode = 1;
	else 
	{
		wdt_enable(0); // Enable watchdog
		while (1); // Reset device
	}
}