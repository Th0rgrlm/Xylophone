/**
 * @file xyl.c
 * @author Matúš Citor (246797@vutbr.cz)
 * @brief 
 * @version 0.1
 * @date 2024-11-13
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "xylophone/xyl.h"
#include <gpio.h>
#include <structs.h>
#include <util/delay.h>

#define C1_PIN PD2
#define D1_PIN PD3
#define E1_PIN PD4
#define F1_PIN PD5
#define G1_PIN PD6
#define A1_PIN PD7
#define B1_PIN PB0
#define C2_PIN PB1
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
        case E1:
            GPIO_write_high(&PORTD, E1_PIN);
            break;
        case F1:
            GPIO_write_high(&PORTD, F1_PIN);
            break;
        case G1:
            GPIO_write_high(&PORTD, G1_PIN);
            break;
        case A1:
            GPIO_write_high(&PORTD, A1_PIN);
            break;
        case B1:
            GPIO_write_high(&PORTB, B1_PIN);
            break;
        case C2:
            GPIO_write_high(&PORTB, C2_PIN);
            break;
        case NONE:
            GPIO_write_low(&PORTD, C1_PIN);
            GPIO_write_low(&PORTD, D1_PIN);
            GPIO_write_low(&PORTD, E1_PIN);
            GPIO_write_low(&PORTD, F1_PIN);
            GPIO_write_low(&PORTD, G1_PIN);
            GPIO_write_low(&PORTD, A1_PIN);
            GPIO_write_low(&PORTB, B1_PIN);
            GPIO_write_low(&PORTB, C2_PIN);
            break;
    default:
        break;
    }
}

void xyl_init(void){
    GPIO_mode_output(&DDRD, C1_PIN);
    GPIO_mode_output(&DDRD, D1_PIN);
    GPIO_mode_output(&DDRD, E1_PIN);
    GPIO_mode_output(&DDRD, F1_PIN);
    GPIO_mode_output(&DDRD, G1_PIN);
    GPIO_mode_output(&DDRD, A1_PIN);
    GPIO_mode_output(&DDRB, B1_PIN);
    GPIO_mode_output(&DDRB, C2_PIN);
}