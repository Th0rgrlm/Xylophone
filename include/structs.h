/**
 * @file structs.h
 * @author Martin Garncarz (246815@vutbr.cz)
 * @brief Basic structure definitions
 * @version 0.1
 * @date 2024-12-04
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef __STRUCTS_H__
#define __STRUCTS_H__

/**
 * @brief Enum for all notes
 * 
 */
typedef enum notes {
    NONE = 0,
    C1 = 1,
    D1 = 2,
    E1 = 3,
    F1 = 4,
    G1 = 5,
    A1 = 6,
    B1 = 7,
    C2 = 8,
    END = 0xFF
} notes_e;

/**
 * @brief Enum for status of byte reading/parsing
 * 
 */
typedef enum note_status {
    STATUS_DELAY_1 = 0,
    STATUS_DELAY_2 = 1,
    STATUS_NOTE = 2,
    STATUS_END = 0xFF
} note_status_e;

#endif