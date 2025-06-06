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

### Decide interface, bluetooth, tcp client or something else.
Using UDP for faster respons of motor controll, tried TCP, but was to slow. sending rpm to server is not tested.
Motor is currently controlled on a separate core. Pico has 2 cores.

### Simple Protocol
Sends messages every second to IP-adress, this has to be manualy on pico. Messages from pico report rpm.
Messages to the pico can also be sent, this will set the resistans on the trainer from 0 to 100. Look at  https://github.com/davidQva/VibeCodingApicoTestServer for example.

### Finished?
This can be improved on, but will probably not be given mutch more love for now. Converting rpm to watt will likely not be done. 
 But this serves as a good starting point for someone ho wants to look into retrofit components
to biketrainers. 
## TODO
- Connect sensors and code. - done
- Implement initialization sequence for motor calibration. -done
- Set maximum allowed rotations. -done
- Convert rpm to watt - nope

![IMG_4867](https://github.com/user-attachments/assets/980a814e-d6e8-485e-804d-288753f059bc)


Here you can see the debugger attached to pico when its mounted on the trainer, this allows for continous coding and/or testing.
![IMG_4982](https://github.com/user-attachments/assets/efb69b0e-600d-400a-a885-2748cd6f651e)
