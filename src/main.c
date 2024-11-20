#include <avr/io.h>
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

uint8_t song[] = {
	C1,
	NONE, 0x00, 0x80,
	D1,
	NONE, 0x00, 0x80,
	E1,
	NONE, 0x00, 0x80,
	F1,
	NONE, 0x00, 0x80,
	G1,
	NONE, 0x00, 0x80,
	A1,
	NONE, 0x00, 0x80,
	B1,
	NONE, 0x00, 0x80,
	C2,
	NONE, 0x01, 0x00,
	END
};


int main(void)
{
	
	twi_init();
	uart_init(UART_BAUD_SELECT(115200, F_CPU));
	sei();
    uart_puts("Hello, world!\n");
	eeprom_write(0, song, 4);
	uart_puts("EEPROM write done\n");
	memset(song, 0, 4);
	eeprom_read(0, song, 4);
	uart_puts("EEPROM read done\n");
	uint8_t str[16];
	snprintf(str, 16, "song: 0x%2X", song[3]);
	uart_puts(str);
	init();
    while (1) loop();

    return SUCCESS;
}


void init(void)
{
	xyl_init();
	display_init();
}

void loop(void)
{
	uint8_t* song_ptr = song;
	while (*song_ptr != END)
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