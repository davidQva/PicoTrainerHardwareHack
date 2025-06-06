# PicoTrainerHardwareHack
This is a hobby project to learn and test things. Im replaceing the old hardware that is malfunctioning, with new. To be able to control steppermotor and read sensors on a biketrainer im using a raspberry pi pico w. This will alow me to connect to a server and receive rpm. This will allow me to potetially create app that is controllord by the trainer. Communication is done by UDP this works on local network, but ip has to be set on the pico and point to server adress. The repo https://github.com/davidQva/VibeCodingApicoTestServer was develop along with this, to create a simple server with send and receive function.

Use vscode and clone repo. Download Raspberry pi pico project addon and use to compile and flash.

## TODO
- Connect sensors and code. - done
- Implement initialization sequence for motor calibration. -done
- Set maximum allowed rotations. -done
- Convert rpm to watt - nope

## Thoughts
Currently the steppermotor from the trainer works using a drv8226 steppermotor driver with a 12v supply https://www.electrokit.com/upload/product/41016/41016255/drv8825.pdf.

On startup motor goes to start pos. Breaker switch for motor position is implemented ans HAll-sensor for rpm. 
Tried using internal pullup for Hall but hysteris(dont fluctuate when magnet close) was to low, and readings was bad.
Used 330ohm instead, info taken from datasheet for similar sensor, in Section 3 the formula below is described, https://www.electrokit.com/upload/product/41015/41015964/2343219.pdf. output should not be above 25mA e.g.: VS = 12 V; IQ = 12 V/1200 Ω = 10 mA
but we have 3.3V so 3.3/330 = 10 mA. Also added buttons to test steppermotor, will remove when interface is implemented.

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


![IMG_4867](https://github.com/user-attachments/assets/980a814e-d6e8-485e-804d-288753f059bc)


Here you can see the debugger attached to pico when its mounted on the trainer, this allows for continous coding and/or testing.
![IMG_4982](https://github.com/user-attachments/assets/efb69b0e-600d-400a-a885-2748cd6f651e)
