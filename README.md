# PicoTrainerHardwareHack
This is a hobby project aimed at learning and experimenting with new ideas. I'm replacing the old, malfunctioning hardware with new components. To control a stepper motor and read sensor data from a bike trainer, I'm using a Raspberry Pi Pico W. This allows me to connect to a server and receive RPM data, opening up the possibility of creating an app controlled by the trainer. Communication is handled via UDP, which works over the local network—however, the IP address must be manually set on the Pico to point to the server. The repo https://github.com/davidQva/VibeCodingApicoTestServer was develop along with this, to create a simple server with send and receive function.

Use vscode and clone repo. Download Raspberry pi pico project addon and use to compile and flash.

## TODO
- Connect sensors and code. - done
- Implement initialization sequence for motor calibration. -done
- Set maximum allowed rotations. -done
- Convert rpm to watt - nope

## Thoughts
Currently the steppermotor from the trainer works using a drv8226 steppermotor driver with a 12v supply https://www.electrokit.com/upload/product/41016/41016255/drv8825.pdf.

On start-up, motor homes itselfe by using the breaker-switch. Tried using internal pullup in pico for hallsensor but hysteris(dont fluctuate when magnet is close to sensor) was to low, and readings was bad.
Used 330ohm instead, info on how to choose correct resistance was taken from datasheet for similar sensor, in Section 3 the formula below is described, https://www.electrokit.com/upload/product/41015/41015964/2343219.pdf. output should not be above 25mA e.g.: VS = 12 V; IQ = 12 V/1200 Ω = 10 mA
but we have 3.3V so 3.3/330 = 10 mA. Also added temporerary buttons to test steppermotor and will remove when interface is implemented, but can be found in older commits.

### Decide interface, bluetooth, tcp client or UDP.
Using UDP for faster respons of pico, tried TCP but was to slow.
Pico has 2 cores, using first core to receiving and sending data, motor is controlled on second core. 

### Simple Protocol
Connecting to local server is done by manually setting ip adress on pico. Sends messages every second to IP-adress from pico report rpm.
Look at  https://github.com/davidQva/VibeCodingApicoTestServer for example.

### Finished?
This can be improved on, but will probably not be given mutch more love for now. Converting rpm to watt will not be done for now, set a 0-100 scale for resistans on biketrainer. 
But this serves as a good starting point for someone ho wants to look into retrofit components to biketrainers.


![IMG_4867](https://github.com/user-attachments/assets/980a814e-d6e8-485e-804d-288753f059bc)


Here you can see the debugger attached to pico when its mounted on the trainer, this allows for continous coding and testing.
![IMG_4982](https://github.com/user-attachments/assets/efb69b0e-600d-400a-a885-2748cd6f651e)
