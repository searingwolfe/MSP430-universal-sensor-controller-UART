## PCB Board Designs to Improve Universality:

This PCB board design is a simplified version of the PWM Signal Amplifier PCB Board, minimizing space and components to output a voltage range outside the 0V to +3V requirement of the MSP430G2553 (see the sensor’s datasheet, or measure the outputs with a multi-meter).

### PWM Generic Amplifier PCB Board

This PCB board serves as a miniature amplifier circuit for up to three PWM signals outputting from the sensor controller program within the MSP430G2553. 
- Originally designed for two simple +12V DC motors (200-300mA each); MSP430G2553 outputs a PWM amplitude of +3.3V:
  - Powerful N-channel MOSFET switches (IRLB8721) amplify the three PWM outputs.
  - One rectifier flywheel diode (1N4001) per MOSFET to prevent back EMF from +12V DC motors from going down into the MOSFETs.

> Note: IRLB8721 MOSFET switches are rated for up to 30V and 62A. The 1N4001 diodes are rated for only 1A.

#### Real Circuit Image & Header Labels

![alt text](https://github.com/searingwolfe/MSP430-universal-sensor-controller-UART/blob/master/PWM_Generic_Amplifier/PWM_Amplifier_realimage.jpg)

Image of the Generic triple-PWM signal amplifier PCB board: 
- Includes 1N4001 diodes and IRLB8721 MOSFETs. 
- The headers on the left from top to bottom: GND and +12V input (separated), and PWM_1 input, PWM_2 input, PWM_3 input, PWM_3 output, PWM_2 output, PWM_1 output, GND.
- Additional drill holes allow extra pins to be soldered on for testing voltages across the cirucit.

#### PCB Design (Front/Back)

![alt text](https://github.com/searingwolfe/MSP430-universal-sensor-controller-UART/blob/master/PWM_Generic_Amplifier/PWM_Amplifier_PCB_front.jpg)
![alt text](https://github.com/searingwolfe/MSP430-universal-sensor-controller-UART/blob/master/PWM_Generic_Amplifier/PWM_Amplifier_PCB_back.jpg)

Front and back faces of the triple-PWM signal amplifier PCB board; realistic images. Dimensions are, width: 44.45mm, height: 34.29mm.