# PicoTrainerHardwareHack
Replace hardware to control steppermotor and read sensors on a trainer. 
Using a raspberry pi pico w to replace the electronic controlling device on a bicycle trainer.

Use vscode and clone repo. Download Raspberry pi pico project addon and use to compile and flash.

## Done
Currently the steppermotor from the trainer works using a drv8226 steppermotor driver with a 12v supply.

On startup motor goes to start pos. Breaker switch for motor position is implemented ans HAll-sensor for rpm. 
Tried using internal pullup for Hall but hysteris(dont fluctuate when magnet close) was to low, and readings was bad.
Used 330ohm instead, info taken from datasheet for similar sensor. output should not be above 25mA e.g.: VS = 12 V; IQ = 12 V/1200 Ω = 10 mA
but we have 3.3V so 3.3/330 = 10 mA. ALso added buttons but will remove when interface is implemented

## Next
calculate max rotations.
Decide interface, bluetooth, tcp client or something else.
Sensors is wokring so create a simple circuitboard that fits inside the trainer.
## TODO
- Connect sensors and code. - done
- Implement initialization sequence for motor calibration. -done
- Set maximum allowed rotations.
- Web Interface
- Add electrical schematic later.
