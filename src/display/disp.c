/**
 * @file display.c
 * @author Jonas (114212657+VUT246843@users.noreply.github.com)
 * @brief 
 * @version 0.1
 * @date 2024-11-13
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "disp.h"

void display_init() {
    oled_init(OLED_DISP_ON);
    oled_clrscr();
    oled_charMode(DOUBLESIZE);
}

void display_note(notes_e note) {
    oled_clear_buffer();
    switch (note) {
        case C1: oled_gotoxy(0, 0); oled_puts("C1");
        break;
        case D1: oled_gotoxy(0, 0); oled_puts("D1");
        break;
        case E1: oled_gotoxy(0, 0); oled_puts("E1");
        break;
        case F1: oled_gotoxy(0, 0); oled_puts("F1");
        break;
        case G1: oled_gotoxy(0, 0); oled_puts("G1");
        break;
        case A1: oled_gotoxy(0, 0); oled_puts("A1");
        break;
        case B1: oled_gotoxy(0, 0); oled_puts("B1");
        break;
        case C2: oled_gotoxy(0, 0); oled_puts("C2");
        break;
        default: oled_gotoxy(0, 0); oled_puts("Pause");
        break;
    }

    oled_display();

    // while (1) {
    // }
}