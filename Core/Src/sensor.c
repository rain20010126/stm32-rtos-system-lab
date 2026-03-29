#include "sensor.h"
#include "i2c.h"
#include "cmsis_os.h"   // for osDelay
#include <stdint.h>

#define BME680_ADDR (0x76 << 1)

// calibration
static uint16_t dig_T1;
static int16_t  dig_T2;
static int8_t   dig_T3;

static int32_t t_fine;

// ========================
// I2C write
// ========================

static int i2c_write(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = {reg, val};
    return HAL_I2C_Master_Transmit(&hi2c1, BME680_ADDR, buf, 2, 100);
}

// ========================
// Read calibration
// ========================

static int read_calibration(void)
{
    uint8_t calib1[25];
    uint8_t calib2[16];

    if (HAL_I2C_Mem_Read(&hi2c1, BME680_ADDR, 0x89,
                         I2C_MEMADD_SIZE_8BIT, calib1, 25, 100) != HAL_OK)
        return -1;

    if (HAL_I2C_Mem_Read(&hi2c1, BME680_ADDR, 0xE1,
                         I2C_MEMADD_SIZE_8BIT, calib2, 16, 100) != HAL_OK)
        return -1;

    // correct mapping + little endian
    dig_T1 = (uint16_t)(calib2[8] | (calib2[9] << 8));
    dig_T2 = (int16_t)(((int16_t)calib1[2] << 8) | calib1[1]);
    dig_T3 = (int8_t)calib1[2];

    return 0;
}

// ========================
// Init
// ========================

int sensor_init(void)
{
    uint8_t id;

    // soft reset
    if (i2c_write(0xE0, 0xB6) != HAL_OK)
        return -1;

    osDelay(10);

    // read chip id
    if (HAL_I2C_Mem_Read(&hi2c1, BME680_ADDR, 0xD0,
                         I2C_MEMADD_SIZE_8BIT, &id, 1, 100) != HAL_OK)
        return -1;

    if (id != 0x61)
        return -1;

    if (read_calibration() != 0)
        return -1;

    return 0;
}

// ========================
// Read temperature
// ========================

int sensor_read(sensor_data_t *data)
{
    if (data == NULL)
        return -1;

    uint8_t buf[3];

    // config register
    if (i2c_write(0x75, 0x00) != HAL_OK)
        return -1;

    // ctrl_meas: temp x1, forced mode
    if (i2c_write(0x74, 0x25) != HAL_OK)
        return -1;

    osDelay(50);

    // read temperature registers
    if (HAL_I2C_Mem_Read(&hi2c1, BME680_ADDR, 0x22,
                         I2C_MEMADD_SIZE_8BIT, buf, 3, 100) != HAL_OK)
        return -1;

    int32_t adc_T =
        ((int32_t)buf[0] << 12) |
        ((int32_t)buf[1] << 4) |
        ((int32_t)buf[2] >> 4);

    // compensation
    int32_t var1, var2, var3;

    var1 = (((int32_t)adc_T >> 3) - ((int32_t)dig_T1 << 1));
    var2 = (var1 * (int32_t)dig_T2) >> 11;
    var3 = (((var1 >> 1) * (var1 >> 1)) >> 12);
    var3 = (var3 * ((int32_t)dig_T3 << 4)) >> 14;

    t_fine = var2 + var3;

    int32_t temp_comp = ((t_fine * 5) + 128) >> 8;

    data->temperature = temp_comp;

    return 0;
}