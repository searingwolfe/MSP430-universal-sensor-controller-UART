## PCB Board Designs to Improve Universality:

This PCB board design allows for much more convenient and practical integration of the sensor controller within any larger enveloping device/system. A very specific amplifier circuit is required for a wide range of sensors that output a voltage range outside the 0V to +3V requirement of the MSP430G2553 (see the sensor’s datasheet, or measure the outputs with a multi-meter). 

### PWM Signal Amplifier PCB Board

This PCB board serves as a miniature amplifier circuit for the two PWM signals outputting from the sensor controller program within the MSP430G2553. 
- Originally designed for two simple +12V DC motors (200-300mA each); MSP430G2553 outputs a PWM amplitude of +3.3V:
  - Powerful N-channel MOSFET switches (STP16NF06) amplify the two PWM outputs. 
  - The minimum VGS (threshold voltage at gate pin) rating of the STP16NF06 is +2V. 
  - A preliminary transistor (2N2222 NPN BJT) first amplifies the current +3.3V PWM amplitude to a +5V PWM amplitude. 
  - The 2N2222 transistors enforce the saturation mode for the STP16NF06 switches, allowing them to trigger on/off properly. 

> Note: STP16NF06 MOSFET switches are rated for up to 60V and 16A. Use proper resistor values, and high-power rated components (P=IV; 60*16 = 960W maximum) should allow for almost any square-wave controlled device, such as the +12V DC motor peristaltic pumps, to connect and react to the sensor controller.

#### Schematic Diagram & Header Labels

![alt text](https://github.com/RU09342/final-project-ramfinalproject/blob/master/PWM_Signal_Amplifier_PCB/PWM_Motor_Interface.jpg)

Schematic of the dual-PWM signal amplifier PCB board: 
- Includes 2N2222 BJTs and STP16NF06 MOSFETs. 
- The labels of the 5 lower headers are provided on the left. 
- The two upper sets of headers are for +5V / GND and +12V / GND.

#### PCB Design (Front/Back)

![alt text](https://github.com/searingwolfe/MSP430-universal-sensor-controller-UART/blob/master/PWM_Motor_Interface/PWM_Motor_InterfacePCB3.jpg)
![alt text](https://github.com/searingwolfe/MSP430-universal-sensor-controller-UART/blob/master/PWM_Motor_Interface/PWM_Motor_InterfacePCB4.jpg)

Front and back faces of the dual-PWM signal amplifier PCB board; realistic images. Dimensions are, width: 48.26 mm, height: 39.37 mm. 
- An extra two through-hole pins are provided next to +12V for connecting the +12V pins of two different DC motors to the PCB board. 
- The other pins on the two motors connect to the Headers, or the outputs of the PCB.