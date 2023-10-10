/*
 * bmp280.h
 *
 *  Created on: 10 oct . 2023
 *      Author: GILLES Baptiste
 */

#ifndef DRV_BMP280_H_
#define DRV_BMP280_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum BMP280_power_mode_enum
{
	BMP280_SLEEP_MODE         = 0, // mode[1:0] bits 00
    BMP280_FORCED_MODE        = 2, // mode[1:0] bits 10 and 01
    BMP280_NORMAL_MODE        = 3, // mode[1:0] bits 11
} BMP280_power_t;

typedef enum BMP280_register_enum
{
	BMP280_CHIP_ID           = 0xD0,  // R Chip Id 0x58 (8 bits)
    BMP280_RESET             = 0xE0,  // R always 0x00 W 0xB6 to Force Reset (8 bits)
    BMP280_CONTROL           = 0xF4, // 7,6,5 osrs_t 4,3,2 osrs_p 1,0 mode(8 bits)
    BMP280_CONFIG            = 0xF5,  // 7,6,5 t_sb 4,3,2 filter 0 spi3w_en
    BMP280_PRESSURE_MSB      = 0xF7,
    BMP280_PRESSURE_LSB      = 0xF8,
    BMP280_PRESSURE_XLSB     = 0xF9,
    BMP280_TEMP_MSB          = 0xFA,
    BMP280_TEMP_LSB          = 0xFB,
    BMP280_TEMP_XLSB         = 0xFC,
} BMP280_register_t;

typedef enum BMP280_sample_enum
{
	BMP280_SAMPLE_0          = 0, 
    BMP280_SAMPLE_1          = 1, 
    BMP280_SAMPLE_2          = 2, 
    BMP280_SAMPLE_4          = 3, 
    BMP280_SAMPLE_8          = 4, 
    BMP280_SAMPLE_16         = 7, 
}BMP280_sample_t;

// Définition d'un type de fonction de transmission I2C.
typedef int (*BMP280_transmit_drv_t)(uint8_t address, uint8_t *p_data, uint16_t size);

// Définition d'un type de fonction de réception I2C.
typedef int (*BMP280_receive_drv_t)(uint8_t address, uint8_t *p_data, uint16_t size);

typedef struct BMP280_I2C_drv_struct
{
	BMP280_transmit_drv_t transmit;
	BMP280_receive_drv_t receive;
} BMP280_I2C_drv_t;

typedef struct h_BMP280_struct
{
    // Sample
	BMP280_sample_t sample;

    // Power mode
	BMP280_power_t power;

	// driver SPI
	BMP280_I2C_drv_t I2C_drv;
    // temperature, pressure
	float temp;
	float press;
    
    // Mesure dispo
	uint8_t data_available;

    // Skipped data
	uint32_t skipped_data;

} h_BMP280_t;

#endif /* DRV_BMP280_H_ */
