// original source: https://github.com/FaBoPlatform/FaBoPWM-PCA9685-Library/blob/master/src/FaBoPWM_PCA9685.h

/** PCA9685 Slave Address register */
#define PCA9685_SLAVE_ADDRESS 0x40
/** Mode register1 */
#define PCA9685_MODE1 0x00

#define PCA9685_ALL_LED_ON_L_REG 0xFA
#define PCA9685_ALL_LED_ON_H_REG 0xFB
#define PCA9685_ALL_LED_OFF_L_REG 0xFC
#define PCA9685_ALL_LED_OFF_H_REG 0xFD
#define PCA9685_MODE1_REG 0x00
#define PCA9685_MODE2_REG 0x01
#define PCA9685_PRE_SCALE_REG 0xFE
/** LED0 output and brightness control byte 0. */
#define PCA9685_LED0_ON_L_REG 0x06 
/** LED0 output and brightness control byte 1. */
#define PCA9685_LED0_ON_H_REG 0x07 
/** LED0 output and brightness control byte 2. */
#define PCA9685_LED0_OFF_L_REG 0x08 
/** LED0 output and brightness control byte 3. */
#define PCA9685_LED0_OFF_H_REG 0x09

#define PCA9685_OSC_CLOCK 25000000.0

#define PCA9685_DEFAULT_VALUE 300

/** Mode1 */
#define PCA9685_RESTART 0x80
#define PCA9685_SLEEP 0x10
#define PCA9685_ALLCALL 0x01
/** Mode2 */
#define PCA9685_OUTDRV 0x04
