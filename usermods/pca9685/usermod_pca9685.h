#pragma once
#include "wled.h"
#include "./usermod_pca9685_registers.h"

// Original source of PCA9685 code: https://github.com/FaBoPlatform/FaBoPWM-PCA9685-Library/blob/master/src/FaBoPWM_PCA9685.cpp

#ifndef ARDUINO_ARCH_ESP32
// 8266 does not support analogRead on user selectable pins
#error only ESP32 is supported by usermod LDR_DUSK_DAWN
#endif

class PCA9685 : public Usermod
{
private:
  // Defaults
  bool enabled = false;
  int i2cAddress = 0x40;
  bool calculateWhiteFromRGB = false;

  // Variables
  static const char _name[];

public:
  void setup()
  {
    if (i2c_scl < 0 || i2c_sda < 0)
    {
      enabled = false;
      return;
    }

    uint16_t value = 0; // initial brightnes
    writeI2c(PCA9685_ALL_LED_ON_L_REG, 0x00);
    writeI2c(PCA9685_ALL_LED_ON_H_REG, 0x00);
    writeI2c(PCA9685_ALL_LED_OFF_L_REG, value & 0xFF);
    writeI2c(PCA9685_ALL_LED_OFF_L_REG, value >> 8);

    writeI2c(PCA9685_MODE2_REG, PCA9685_OUTDRV);
    writeI2c(PCA9685_MODE1_REG, PCA9685_ALLCALL);
    delay(100);

    uint8_t buffer[1];
    readI2c(PCA9685_MODE1_REG, 1, buffer);
    uint8_t mode = buffer[0] & ~PCA9685_SLEEP;
    writeI2c(PCA9685_MODE1_REG, mode);
  }

  void loop()
  {
    uint16_t l = strip.getLengthTotal();
    if (l > 16)
    {
      l = 16;
    }

    for (uint8_t i = 0; i < l; ++i)
    {
      uint32_t color = strip.getPixelColor(i);
      if (calculateWhiteFromRGB)
      {
        // 0.299 ∙ Red + 0.587 ∙ Green + 0.114 ∙ Blue.
        // and << 4 (8 to 12 bits) = *16
        set_channel_value(i,
                          uint16_t(float(color & 0xff) * 16.0 * 0.114 +
                                   float(color >> 8 & 0xff) * 16.0 * 0.587 +
                                   float(color >> 16 & 0xff) * 16.0 * 0.299));
      }
      else
      {
        set_channel_value(i, uint16_t(color >> 20 & 0xff0));
      }
    }
  }

  void addToConfig(JsonObject &root)
  {
    JsonObject top = root.createNestedObject(FPSTR(_name));
    top["Enabled"] = enabled;
    top["I2C Address"] = i2cAddress;
    top["Calculate white from RGB"] = calculateWhiteFromRGB;
  }

  bool readFromConfig(JsonObject &root)
  {
    JsonObject top = root[FPSTR(_name)];
    bool configComplete = !top.isNull();
    configComplete &= getJsonValue(top["Enabled"], enabled);
    configComplete &= getJsonValue(top["I2C Address"], i2cAddress);
    configComplete &= getJsonValue(top["Calculate white from RGB"], calculateWhiteFromRGB);
    return configComplete;
  }

  uint16_t getId()
  {
    return USERMOD_ID_LDR_DUSK_DAWN;
  }

  /**
   @brief convert value hz to prescale.
   @param [in] hz hz.
  */
  uint16_t calc_prescale(uint16_t hz)
  {
    return uint16_t(round(PCA9685_OSC_CLOCK / 4096 / hz) - 1);
  }

  /**
   @brief set hz.
   @param [in] hz hz.
  */
  void set_hz(uint16_t hz)
  {
    uint16_t prescale = calc_prescale(hz);
    uint8_t buffer[1];
    readI2c(PCA9685_MODE1_REG, 1, buffer);
    uint8_t oldmode = buffer[0];
    uint8_t newmode = oldmode | PCA9685_SLEEP;
    writeI2c(PCA9685_MODE1_REG, newmode);
    writeI2c(PCA9685_PRE_SCALE_REG, prescale);
    writeI2c(PCA9685_MODE1_REG, oldmode);
    delay(100);
    uint8_t restart = oldmode | PCA9685_RESTART;
    writeI2c(PCA9685_MODE1_REG, restart);
  }

  /**
 @brief Write value to channel.
 @param [in] channel channel of pwm.
 @param [in] value value of pwm.
*/
  void set_channel_value(uint8_t channel, uint16_t value)
  {
    writeI2c(PCA9685_LED0_ON_L_REG + channel * 4, 0x00);
    writeI2c(PCA9685_LED0_ON_H_REG + channel * 4, 0x00);
    writeI2c(PCA9685_LED0_OFF_L_REG + channel * 4, (value & 0xFF));
    writeI2c(PCA9685_LED0_OFF_H_REG + channel * 4, (value >> 8));
  }

  /**
 @brief Write I2C Data
 @param [in] register_addr Write Register Address
 @param [in] value Write Data
*/
  void writeI2c(uint8_t register_addr, uint8_t value)
  {
    Wire.beginTransmission(i2cAddress);
    Wire.write(register_addr);
    Wire.write(value);
    Wire.endTransmission();
  }

  /**
   @brief Read I2C Data
   @param [in] register_addr register address
   @param [in] num Data Length
   @param [out] *buf Read Data
  */
  void readI2c(uint8_t register_addr, int num, uint8_t *buf)
  {
    Wire.beginTransmission(i2cAddress);
    Wire.write(register_addr);
    Wire.endTransmission();
    Wire.requestFrom(i2cAddress, num);

    int i = 0;
    while (Wire.available())
    {
      buf[i] = Wire.read();
      i++;
    }
  }
};

const char PCA9685::_name[] PROGMEM = "PCA9685";
