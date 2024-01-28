# AutoPower

Automatically switch on/off a pin after a delay (may zero), when all leds are off / at least one LED is on

# Config

- Enabled
- Relay pin (-1 to disable)
- On preset (-1 to disable)
- Off preset (-1 to disable)
- On delay in ms
- Off delay in ms

# Logic

| state# | ledState | wasOn | isOn | Function|
|---|---|---|---|---|
|0|0|0|0| stable, nothing to do
|1|0|0|1| wasOn = true; start delay: onThreshold = now() + offDelayMs
|2|0|1|0| wasOn = false; stop on-delay: onThreshold = 0
|3|0|1|1| check on-delay:  onThreshjold >= now() ? ledState = true
|4|1|0|0| check off-delay: offThreshjold >= now() ? ledState = false
|5|1|0|1| wasOn = true; stop off-delay: offThreshold = 0
|6|1|1|0| wasOn = false; start delay: offThresholld = now() + offDelayMs
|7|1|1|1| stable, nothing to do