#pragma once
#include "wled.h"

class AutoPower : public Usermod
{
private:
    static const char _name[];

    // config
    bool enabled = false;
    int relayPin = -1;
    int onPreset = -1;
    int offPreset = -1;
    unsigned long onDelayMs = 0;
    unsigned long offDelayMs = 1000 * 60;

    // internal variables
    bool initDone = false;
    bool ledState = false;
    bool wasOn = false;
    unsigned long threshold = 0;

    // TODO I want to be a real owner. None is unsatisfying.
    PinOwner pinOwner = PinOwner::None;

public:
    void setup()
    {
        // register pin
        if ((relayPin >= 0) && (digitalPinToAnalogChannel(relayPin) >= 0))
        {
            if (!pinManager.allocatePin(relayPin, false, pinOwner))
            {
                enabled = false;
            }
            else
            {
                pinMode(relayPin, OUTPUT);
                digitalWrite(relayPin, ledState);
            }
        }
        else
        {
            enabled = false;
        }

        initDone = true;
    }

    void loop()
    {
        // calculate state
        bool isOn = isLedOn();
        uint8_t state = ledState << 2 | wasOn << 1 || isOn;

        switch (state)
        {
        case 0:
            // stable, nothing to do
            break;
        case 1:
            startDelay(onDelayMs);
            break;
        case 2:
            // wasOn = isOn;
            break;
        case 3:
            checkDelayAndSetLedStateTo(true);
            break;
        case 4:
            checkDelayAndSetLedStateTo(false);
            break;
        case 5:
            // wasOn = isOn;
            break;
        case 6:
            startDelay(offDelayMs);
            // wasOn = isOn;
            break;
        case 7:
            // stable, nothing to do
            break;
        }

        wasOn = isOn;
    }

    /**
     * @brief Set delay threshold
     *
     * @param delayMs ms to wait to reach threshold
     */
    void startDelay(unsigned long delayMs)
    {
        threshold = millis() + delayMs;
    }

    /**
     * @brief checks if delay threshold is reached, if yes, set new state
     *
     * @param newRelayState the state to set
     */
    void checkDelayAndSetLedStateTo(bool newRelayState)
    {
        if (threshold >= millis())
        {
            // threshold reached
            ledState = newRelayState;
            if (ledState)
            {
                // switch ON
                if (relayPin >= 0)
                {
                    digitalWrite(relayPin, ledState);
                }

                if (onPreset >= 0)
                {
                    applyPreset(onPreset);
                }
            }
            else
            {
                // switch OFF
                if (relayPin >= 0)
                {
                    digitalWrite(relayPin, ledState);
                }

                if (offPreset >= 0)
                {
                    applyPreset(onPreset);
                }
            }
        }
    }

    /**
     * @brief check that at least one LED in on
     *
     * @return true if at least one LED is on
     * @return false  all LEDs are off
     */
    bool isLedOn()
    {
        uint16_t l = strip.getLength();
        for (uint16_t i = 0; i < l; ++i)
        {
            if (strip.getPixelColor(i))
            {
                return true;
            }
        }
        return false;
    }

    void addToConfig(JsonObject &root)
    {
        JsonObject top = root.createNestedObject(FPSTR(_name));
        top["Enabled"] = enabled;
        top["Relay pin"] = relayPin;
        top["On preset"] = onPreset;
        top["Off preset"] = offPreset;
        top["On delay in ms"] = onDelayMs;
        top["Off delay in ms"] = offDelayMs;
    }

    bool readFromConfig(JsonObject &root)
    {
        int8_t oldRelayPin = relayPin;
        JsonObject top = root[FPSTR(_name)];
        bool configComplete = !top.isNull();
        configComplete &= getJsonValue(top["Enabled"], enabled);
        configComplete &= getJsonValue(top["Relay pin"], relayPin);
        configComplete &= getJsonValue(top["On preset"], onPreset);
        configComplete &= getJsonValue(top["Off preset"], offPreset);
        configComplete &= getJsonValue(top["On delay in ms"], onDelayMs);
        configComplete &= getJsonValue(top["Off delay in ms"], offDelayMs);

        if (initDone && (relayPin != oldRelayPin))
        {
            // pin changed - un-register previous pin, register new pin
            if (oldRelayPin >= 0)
                pinManager.deallocatePin(oldRelayPin, pinOwner);
            setup(); // setup new pin
        }
        return configComplete;
    }

    void addToJsonInfo(JsonObject &root)
    {
        // If "u" object does not exist yet we need to create it
        JsonObject user = root["u"];
        if (user.isNull())
            user = root.createNestedObject("u");

        JsonArray LDR_Enabled = user.createNestedArray("Led state");
        LDR_Enabled.add(ledState);
    }
};

const char AutoPower::_name[] PROGMEM = "AutoPower";