/**
 * @file eeprom.h
 * @author Matúš Citor (246797@vutbr.cz)
 * @brief 
 * @version 0.1
 * @date 2024-11-20
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef __EEPROM_H__
#define __EEPROM_H__

#include "stdint.h"

/**
 * @brief Reads bytes from EEPROM
 * 
 * @param addr Address in the EEPROM to read from
 * @param buffer Output buffer the read data will be saved to
 * @param num_bytes Number of bytes to read
 * @return int32_t Return code
 */
extern int32_t eeprom_read(uint32_t addr, uint8_t* buffer, uint16_t num_bytes);

/**
 * @brief Writes bytes to EEPROM
 * 
 * @param addr Address in the EEPROM to write to
 * @param buffer Data to write to EEPROM
 * @param num_bytes Number of bytes to write
 * @return int32_t Return code
 */
extern int32_t eeprom_write(uint32_t addr, uint8_t* buffer, uint16_t num_bytes);


#endif