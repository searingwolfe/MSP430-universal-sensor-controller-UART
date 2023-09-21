## PCB Board Designs to Improve Universality:

These two PCB board designs allow for much more convenient and practical integration of the sensor controller within any larger enveloping device/system. A very specific amplifier circuit is required for a wide range of sensors that output a voltage range outside the 0V to +3V requirement of the MSP430G2553 (see the sensor’s datasheet, or measure the outputs with a multi-meter). 

### MSP430G2553 LED Interface PCB Board

This PCB board serves to provide the user with easy access to LEDs and other GPIO outputs for the MSP430G2553 IC chip (without board). To improve the universality, ease of installation, and speed of testing the various pins, this PCB board converts a typical +5V USB cable into a +3.3V power supply for the MSP430G2553, using an LM3940 voltage regulator. 

>Note: In order for the MSP430G2553 to function properly while disconnected from its red programming board, the “RST” pin must be wired to power, but must have an RC circuit before the power supply pin to provide enough time-delay for the “RST” pin to trigger off and stay off. 

- All GPIO outputs used for the outputs, Timers, Bounds, and status bits, are wired directly to LEDs. 
- The PWM outputs are fed straight to the Headers without going through any more components (the PWM signal amplifier PCB connects to these two PWM outputs for controlling stronger DC motors). 

#### Schematic Diagram & Header Labels

![alt text](https://github.com/searingwolfe/MSP430-universal-sensor-controller-UART/blob/master/MSP430G2553_LED_Board/Board/MSP430G2553_LED_Board.jpg)

Schematic of the MSP430G2553 LED interface PCB board:
- The labels of the 7 lower headers are provided on the left. 
- The +5V power supply and GND supply come from a male USB plug, connecting to an appropriate +5V or +5.1V (1A) wall-outlet power-adapter.

#### PCB Design (Front/Back)

![alt text](https://github.com/searingwolfe/MSP430-universal-sensor-controller-UART/blob/master/MSP430G2553_LED_Board/Board/MSP430G2553_LED_BoardPCB3.jpg)
![alt text](https://github.com/searingwolfe/MSP430-universal-sensor-controller-UART/blob/master/MSP430G2553_LED_Board/Board/MSP430G2553_LED_BoardPCB4.jpg)

Front and back faces of the MSP430G2553 LED interface PCB board; realistic images. 
- Dimensions are, width: 74.93 mm, height: 57.15 mm. 
  - The PCB board has through-hole pins before all resistors and LEDs, for attaching other devices to the MSP430G2553’s GPIO pins (outputting 1/0 signals). 
  - An extra through-hole pin, other than the Headers, is also provided for the two PWM outputs. 
  - Three through-hole pins are also above the LM340 (5V-3.3V regulator) in case a heat sink, thermistor, or other temperature sensor must be placed next the heat-emitting IC regulator.