#include <avr/io.h>
#include <util/delay.h>
#include "error_codes.h"
#include "structs.h"
#include "xyl.h"


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

uint8_t song[] = {
	C1,
	NONE, 0x00, 0x80,
	C1,
	NONE, 0x00, 0x80,
	G1,
	NONE, 0x00, 0x80,
	G1,
	NONE, 0x00, 0x80,
	A1,
	NONE, 0x00, 0x80,
	A1,
	NONE, 0x00, 0x80,
	G1,
	NONE, 0x01, 0x00,
	END
};


int main(void)
{
    init();

    while (1) loop();

    return SUCCESS;
}


void init(void)
{
	xyl_init();
}

void loop(void)
{
	uint8_t* song_ptr = &song;
	while (*song_ptr++ != END)
	{
		if (*song_ptr == NONE)
		{
			_delay_ms(1000);
		}
		else xyl_play_note(*song_ptr);
	}
}