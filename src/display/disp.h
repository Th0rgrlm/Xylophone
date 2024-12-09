/**
 * @file disp.h
 * @defgroup xylophone_disp Display Library <disp.h>
 * @code #include <disp.h> @endcode
 * @brief Display control.
 * @author Jonas (114212657+VUT246843@users.noreply.github.com)
 * @version 0.1
 * @date 2024-11-13
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <oled.h>
#include "structs.h"

/**
 * @brief Initialise the display
 * 
 */
void display_init();

/**
 * @brief Display a note on OLED display
 * 
 * @param note Note to be displayed
 */
void display_note(notes_e note);