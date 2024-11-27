#include "eeprom.h"
#include "twi.h"
#include "stdint.h"
#include "string.h"
#include "error_codes.h"
#include <util/delay.h>

#define EEPROM_ADDR_MEMORY 0x57 // EEPROM I2C address

int32_t eeprom_read(uint32_t addr, uint8_t* buffer, uint16_t num_bytes)
{
    int32_t ret;
    uint8_t i2c_addr = EEPROM_ADDR_MEMORY; // EEPROM I2C address
    uint8_t addr_bytes[2];
    addr_bytes[0] = (addr & 0x00007F00) >> 8; // EEPROM address MSB
    addr_bytes[1] = (addr & 0x000000FF); // EEPROM address LSB

    memset(buffer, 0x00, num_bytes); // Clear output buffer

    if ((ret = twi_write_bites(i2c_addr, addr_bytes, 2, 1)) != 0) return ret; // Write address to read from

    if ((ret = twi_read_bites(i2c_addr, buffer, num_bytes, 0)) != 0) return ret; // Read data
    return SUCCESS;
}

int32_t eeprom_write(uint32_t addr, uint8_t* buffer, uint16_t num_bytes)
{
    int32_t ret;
    uint8_t i2c_addr = EEPROM_ADDR_MEMORY; // EEPROM I2C address
    uint8_t write_buffer[num_bytes + 2];
    memset(write_buffer, 0x00, num_bytes + 2); // Clear write buffer
    write_buffer[0] = (addr & 0x00007F00) >> 8; // EEPROM address MSB
    write_buffer[1] = (addr & 0x000000FF); // EEPROM address LSB

    if ((uint16_t)write_buffer[1] + (uint16_t)num_bytes > 0x100) return EEPROM_ERROR_PAGE_OVERFLOW; // Check for buffer overflow

    memcpy(&write_buffer[2], buffer, num_bytes); // Copy data to write

    if ((ret = twi_write_bites(i2c_addr, write_buffer, num_bytes + 2,0)) != 0) return ret; // Write data
    _delay_ms(10); // Wait for data write in EEPROM
    return SUCCESS;
}

