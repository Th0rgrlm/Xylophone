/**
 * @file uart_pc.h
 * @author Martin Garncarz (246815@vutbr.cz)
 * @brief PC COM port communication
 * @version 0.1
 * @date 2024-11-26
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

/**
 * @brief Sends parsed .mxy file to COM port
 * 
 * @param parsed pointer to the parsed MXY file
 * @param COM COM port to communicate on
 * @return int16_t Error code
 */
int16_t uart_send(FILE* parsed, uint8_t COM);
