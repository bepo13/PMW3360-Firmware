/* MIT License
 *
 * Copyright (c) 2023 Brent Peterson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef PMW3360_H__
#define PMW3360_H__

#include <stdint.h>
#include <stdbool.h>

// PMW3360 register definitions
#define PMW3360_REG_PRODUCT_ID                      0x00
#define PMW3360_REG_REVISION_ID                     0x01
#define PMW3360_REG_MOTION                          0x02
#define PMW3360_REG_DELTA_X_L                       0x03
#define PMW3360_REG_DELTA_X_H                       0x04
#define PMW3360_REG_DELTA_Y_L                       0x05
#define PMW3360_REG_DELTA_Y_H                       0x06
#define PMW3360_REG_SQUAL                           0x07
#define PMW3360_REG_RAW_DATA_SUM                    0x08
#define PMW3360_REG_MAXIMUM_RAW_DATA                0x09
#define PMW3360_REG_MINIMUM_RAW_DATA                0x0A
#define PMW3360_REG_SHUTTER_LOWER                   0x0B
#define PMW3360_REG_SHUTTER_UPPER                   0x0C
#define PMW3360_REG_CONTROL                         0x0D
#define PMW3360_REG_CONFIG1                         0x0F
#define PMW3360_REG_CONFIG2                         0x10
#define PMW3360_REG_ANGLE_TUNE                      0x11
#define PMW3360_REG_FRAME_CAPTURE                   0x12
#define PMW3360_REG_SROM_ENABLE                     0x13
#define PMW3360_REG_RUN_DOWNSHIFT                   0x14
#define PMW3360_REG_REST1_RATE_LOWER                0x15
#define PMW3360_REG_REST1_RATE_UPPER                0x16
#define PMW3360_REG_REST1_DOWNSHIFT                 0x17
#define PMW3360_REG_REST2_RATE_LOWER                0x18
#define PMW3360_REG_REST2_RATE_UPPER                0x19
#define PMW3360_REG_REST2_DOWNSHIFT                 0x1A
#define PMW3360_REG_REST3_RATE_LOWER                0x1B
#define PMW3360_REG_REST3_RATE_UPPER                0x1C
#define PMW3360_REG_OBSERVATION                     0x24
#define PMW3360_REG_DATA_OUT_LOWER                  0x25
#define PMW3360_REG_DATA_OUT_UPPER                  0x26
#define PMW3360_REG_RAW_DATA_DUMP                   0x29
#define PMW3360_REG_SROM_ID                         0x2A
#define PMW3360_REG_MIN_SQ_RUN                      0x2B
#define PMW3360_REG_RAW_DATA_THRESHOLD              0x2C
#define PMW3360_REG_CONFIG5                         0x2F
#define PMW3360_REG_POWER_UP_RESET                  0x3A
#define PMW3360_REG_SHUTDOWN                        0x3B
#define PMW3360_REG_INVERSE_PRODUCT_ID              0x3F
#define PMW3360_REG_LIFTCUTOFF_TUNE3                0x41
#define PMW3360_REG_ANGLE_SNAP                      0x42
#define PMW3360_REG_LIFTCUTOFF_TUNE1                0x4A
#define PMW3360_REG_MOTION_BURST                    0x50
#define PMW3360_REG_LIFTCUTOFF_TUNE_TIMEOUT         0x58
#define PMW3360_REG_LIFTCUTOFF_TUNE_MIN_LENGTH      0x5A
#define PMW3360_REG_SROM_LOAD_BURST                 0x62
#define PMW3360_REG_LIFT_CONFIG                     0x63
#define PMW3360_REG_RAW_DATA_BURST                  0x64
#define PMW3360_REG_LIFTCUTOFF_TUNE2                0x65

/**
 * @brief Data structure used to read burst data from sensor
 */
typedef struct PMW3360_data
{
    bool motion;            /**< True if a motion is detected */
    bool surface;           /**< True when a chip is on a surface */
    int16_t dx;             /**< Displacement on x directions */
    int16_t dy;             /**< Displacement on y directions */
    uint8_t SQUAL;          /**< Surface Quality register */
    uint8_t rawDataSum;     /**< Upper byte of 18-bit counter which sums all raw data */
    uint8_t maxRawData;     /**< Max raw data value in current frame */
    uint8_t minRawData;     /**< Min raw data value in current frame */
    uint16_t shutter;       /**< Clock cycles of the internal oscillator */
} PMW3360_data;

/**
 * @brief Initialize the PMW3360 sensor.
 *
 * @return none
 */
bool PMW3360_init();

/**
 * @brief Shutdown the PMW3360 sensor.
 *
 * @return none
 */
void PMW3360_shutdown();

/**
 * @brief Read one frame of motion data.
 *
 * @param data Pointer to PMW3360_data structure to read data into.
 * @return none
 */
void PMW3360_read(PMW3360_data *data);

/**
 * @brief Set the DPI level of the PMW3360 sensor.
 *
 * @param DPI DPI value to set, rounds down to multiples of 100.
 * @return Describe what the function returns.
 */
void PMW3360_setDPI(uint16_t DPI);

/**
 * @brief Get the DPI level of the PMW3360 sensor.
 *
 * @return DPI level
 */
uint16_t PMW3360_getDPI();

#endif //PMW3360_H__
