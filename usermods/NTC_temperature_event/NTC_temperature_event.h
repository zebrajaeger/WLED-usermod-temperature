#pragma once

#include "wled.h"
#include "NTC_temperature_lut.h"

#ifdef ARDUINO_ARCH_ESP32
  #error This NTC_temperature_event usermod supports the ESP32.
#endif

class UsermodNtcTemperatureEvent : public Usermod
{
private:
  static const char _name[];
  static const PinOwner pinOwner;

  static const float ntcBeta; // Beta value
  static const float tempInKelvinAt25degCelsius;
  static const float adcMax; // ADC resolution
  static const int adcSampleCount;

  bool enabled = false;

  // configurable stuff
  float supplyVoltage = 3.3;
  float resistorValue = 100000;
  float ntcValue = 100000;
  unsigned long readingIntervalMs = 1000;
  float thresholdTemperature = 25.0;
  float hysteresis = 1.0;
  int ntcPin = 34;
  int onPreset = 1;
  int offPreset = 2;

  // internal state
  bool initDone = false;
  bool currentState = false;
  unsigned long lastMillis = 0;
  float currentTemperature = -300;
  float mqttLastPublishedTemperature = -300;

  float readTemperature()
  {
    // this part -->
    float raw = 0;
    for (int i = 0; i < adcSampleCount; ++i)
    {
      raw += analogRead(ntcPin);
    }
    float adcRawValue = ADC_LUT[(int)raw / adcSampleCount];
    // <-- takes around 1ms with 16 samples

    float Vout = adcRawValue * supplyVoltage / adcMax;
    float Rt = resistorValue * Vout / (supplyVoltage - Vout);
    float T = 1 / (1 / tempInKelvinAt25degCelsius + log(Rt / ntcValue) / ntcBeta); // Temperature in Kelvin

    return T - 273.15; // Temperature in °C
  }

public:
  void setup()
  {
    if ((ntcPin >= 0) && (digitalPinToAnalogChannel(ntcPin) >= 0))
    {
      if (!pinManager.allocatePin(ntcPin, false, pinOwner))
      {
        enabled = false; // pin already in use -> disable usermod
      }
      else
      {
        pinMode(ntcPin, INPUT); // alloc success -> configure pin for input
      }
    }
    else
    {
      enabled = false; // invalid pin -> disable usermod
    }
    initDone = true;
  };

  void loop()
  {
    if (millis() - lastMillis > readingIntervalMs)
    {
      bool stateChanged = false;
      lastMillis = millis();
      if ((enabled == true) && (ntcPin >= 0) && (digitalPinToAnalogChannel(ntcPin) >= 0))
      { // make sure that pin is valid for analogread()
        currentTemperature = readTemperature();
        if (!currentState)
        {
          // current state: off -> check for change state
          if (currentTemperature >= (thresholdTemperature + hysteresis))
          {
            DEBUG_PRINTLN(F("[NTC_T_E] change state to ON"));
            currentState = true;
            applyPreset(offPreset);
            stateChanged = true;
          }
        }
        else
        {
          // current state: on -> check for change state
          if (currentTemperature <= (thresholdTemperature - hysteresis))
          {
            DEBUG_PRINTLN(F("[NTC_T_E] change state to OFF"));
            currentState = false;
            applyPreset(onPreset);
            stateChanged = true;
          }
        }

        DEBUG_PRINT(F("[NTC_T_E] temp: "));
        DEBUG_PRINT(currentTemperature);

#ifndef WLED_DISABLE_MQTT
        if (WLED_MQTT_CONNECTED)
        {
          char buf[128];
          if (currentTemperature != mqttLastPublishedTemperature)
          {
            DEBUG_PRINTLN("[NTC_T_E] Publish temperature to MQTT");

            snprintf(buf, 63, PSTR("%s/ntc/temperature"), mqttDeviceTopic);
            mqtt->publish(buf, 0, false, String(currentTemperature).c_str());

            mqttLastPublishedTemperature = currentTemperature;
          }

          if (stateChanged)
          {
            DEBUG_PRINT("[NTC_T_E] Publish statechange to MQTT");

            snprintf(buf, 63, PSTR("%s/ntc/state"), mqttDeviceTopic);
            mqtt->publish(buf, 0, false, String(currentState).c_str());

            snprintf(buf, 63, PSTR("%s/ntc/preset"), mqttDeviceTopic);
            mqtt->publish(buf, 0, false, String(currentState ? onPreset : offPreset).c_str());
          }
        }
#endif
      }
    }
  };

  void addToJsonInfo(JsonObject &root)
  {
    if (!enabled)
    {
      return;
    }

    JsonObject user = root[F("u")];
    if (user.isNull())
      user = root.createNestedObject(F("u"));

    JsonArray t = user.createNestedArray(F("NTC Temperature"));
    char temp[10];
    snprintf((char *)&temp, 10, "%.2f °C", currentTemperature);
    t.add(temp);

    JsonArray s = user.createNestedArray(F("NTC Temperature State"));
    s.add(currentState ? F("On") : F("Off"));

    JsonArray p = user.createNestedArray(F("NTC Temperature Preset"));
    p.add(currentState ? onPreset : offPreset);
  }

  void addToConfig(JsonObject &root)
  {
    JsonObject top = root.createNestedObject(FPSTR(_name));
    top["Enabled"] = enabled;
    top["Supply voltage"] = supplyVoltage;
    top["Resistor value in Ohm"] = resistorValue;
    top["NTC value in Ohm"] = ntcValue;
    top["Reading interval in ms"] = readingIntervalMs;
    top["Threshold Temperature in &deg;C"] = thresholdTemperature;
    top["Hysteresis in &deg;C"] = hysteresis;
    top["NTC pin"] = ntcPin;
    top["On preset"] = onPreset;
    top["Off Preset"] = offPreset;
  };

  bool readFromConfig(JsonObject &root)
  {
    int8_t oldNtcPin = ntcPin;
    JsonObject top = root[FPSTR(_name)];
    bool configComplete = !top.isNull();
    configComplete &= getJsonValue(top["Enabled"], enabled);
    configComplete &= getJsonValue(top["Supply voltage"], supplyVoltage);
    configComplete &= getJsonValue(top["Resistor value in Ohm"], resistorValue);
    configComplete &= getJsonValue(top["NTC value in Ohm"], ntcValue);
    configComplete &= getJsonValue(top["NTC pin"], ntcPin);
    configComplete &= getJsonValue(top["Reading interval in ms"], readingIntervalMs);
    configComplete &= getJsonValue(top["Threshold Temperature in &deg;C"], hysteresis);
    configComplete &= getJsonValue(top["Hysteresis in &deg;C"], hysteresis);
    configComplete &= getJsonValue(top["On Preset"], onPreset);
    configComplete &= getJsonValue(top["Off Preset"], offPreset);

    if (initDone && (ntcPin != oldNtcPin))
    {
      // pin changed - un-register previous pin, register new pin
      if (oldNtcPin >= 0)
        pinManager.deallocatePin(oldNtcPin, PinOwner::UM_LDR_DUSK_DAWN);
      setup(); // setup new pin
    }
    return configComplete;
  }
};

const PinOwner UsermodNtcTemperatureEvent::pinOwner = PinOwner::None; // TODO I want to be an real owner!

const char UsermodNtcTemperatureEvent::_name[] PROGMEM = "NTC Temperature Event";
const float UsermodNtcTemperatureEvent::ntcBeta = 3950.0;
const float UsermodNtcTemperatureEvent::tempInKelvinAt25degCelsius = 298.15;
const float UsermodNtcTemperatureEvent::adcMax = 4095.0; // ADC resolution 12-bit (0-4095)
const int UsermodNtcTemperatureEvent::adcSampleCount = 16;
