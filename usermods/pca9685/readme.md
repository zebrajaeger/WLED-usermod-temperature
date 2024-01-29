# PCA9685 Usermod

Adds a single PCA9685 16 Channel Output.   

## Compile / change Code

### 1) Edit: /wled00/usermods_list.cpp

Add these parts:
```C
#ifdef USERMOD_PCA9685
  #include "../usermods/pca9685/usermod_pca9685.h"
#endif
```

```C
  #ifdef USERMOD_PCA9685
  usermods.add(new PCA9685());
  #endif
```

### 2) Edit: /platformio.ini

Add parameter 

    -D USERMOD_PCA9685

to the required environment.

### 3) Compile firmware.

https://kno.wled.ge/advanced/compiling-wled/

# Configuration

## Enable I2c

In 

    Config -> Usermods -> Usermod Setup -> Global I2C GPIOs

set SDA and SCL GPIO pins.

## Add RGBW Leds

In

    Config -> LED Preferences -> Hardware setup -> LED outputs

set "SK6812/WS2814 RGBW" as type and a length of 16. Choose an unused gpio port. The white channel is important. We will take the brightness from this channel!

Also set "Auto-calculate white channel from RGB" to "Brighter" or "Max". With this setting, the white value is calculated from the red, green and blue channels.   
It it important that we have rgb channels because FXLed, for example, needs them.

## Configure Usermod

In 

     Config -> Usermods -> PCA9685

check "Enabled" and change the I2C address if needed (default is 0x40 = 64).

## To use with realtime protocol (i.e. E1.31)

In
 
     Config -> Sync interfaces -> Realtime

set "Use main segment only" to true.