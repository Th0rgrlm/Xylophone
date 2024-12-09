/**
 * @file xyl.h
 * @author MGA (246815@vutbr.cz)
 * @version 0.1
 * @date 2024-11-13
 * @defgroup xylophone_xyl Xylophone Control Library <xyl.h>
 * @code #include <xyl.h> @endcode
 * @brief Xylophone library.
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <structs.h>

/**
 * @brief Plays a single note
 * 
 * @param note Note to be played
 */
void xyl_play_note(notes_e note);

/**
 * @brief Initializes xylophone pins
 * 
 */
void xyl_init(void);