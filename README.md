# Universal Sensor Controller (UART Program)

- Main.c Program Code for MSP430G2553. No other files needed for program code. 
- Schematics, PCB designs, and PCB GERBER files for an MSP430G2553 LED interface board and a PWM signal amplifier board. 
- Similar PCB files for a pH probe amplifier board (for ADC input pin), a typical circuit for a sensor amplifier. 
- CoolTerm program for viewing the outputting printed text in a program other than CCS IDE. 
- Example images of CoolTerm and universal sensor controller program in action.

Hover over images to see more information.

![alt text](https://github.com/searingwolfe/MSP430-universal-sensor-controller-UART/blob/master/Universal_Sensor_Controller_UART_Program/LowOutput_testdark.JPG "Low Output (purple LED) turns on when the ADC value goes below the Low Bound (red LED), when the OFF-Timer duration has passed (green LED), and when the ON-Timer duration has passed (yellow LED). The low output stays on for the same length of time that the ON-Timer takes to count until it turns on/passes (length of time that yellow LED is off).")

![alt text](https://github.com/searingwolfe/MSP430-universal-sensor-controller-UART/blob/master/Universal_Sensor_Controller_UART_Program/HighOutput_testbright.JPG "High Output (blue LED) turns on when the ADC value goes above the High Bound (orange LED), when the OFF-Timer duration has passed (green LED), and when the ON-Timer duration has passed (yellow LED). The low output stays on for the same length of time that the ON-Timer takes to count until it turns on/passes (length of time that yellow LED is off).")

```
A breadboard-simulated demo of the pH Pump Controller over UART (now named the Universal Sensor Controller),
is shown above after wiring a photoresistor to the ADC input P1.3 pin on the MSP430G2553.
After setting both timers, the controller began to measure the intensity of light in the room and determine 
if its intensity value is within the two Bounds.
```

## Description & Functions

This universal and fully-customizable controller program allows virtually any sensor to connect to the ADC pin on the MSP430G2553, as long as the sensor outputs a voltage range somewhere in between 0V and +3V.  The controller, once loaded into CCS IDE, allows single-character inputs across UART from the PC keyboard (a program such as CoolTerm or similar is required to view printed text) to communicate with the MSP430 board. Four primary functions are currently fully-customizable: Low Bound, High Bound, ON-Timer, and OFF-Timer. 

### Four Functions: Low Bound, High Bound, OFF-Timer, ON-Timer

- The range of the MSP430G2553’s ADC values goes from 0 to about 1020 (board has ADC10, so 2^10 = 1024 total values, hence cutting the maximum number in half creates the: 
  - Low Bound from 0 to 510 in ADC values
  - High Bound from 510 to 1020 in ADC values. 

- The OFF-Timer’s duration disables the outputs, from the controller program, until the duration has finished (10 minutes minimum). Once the duration passes, the OFF-Timer does not restart counting again until an output is finished outputting (only triggered after the next ON-Timer duration comes around and after a Bound is detected). 

- The ON-Timer holds the output on for the specified duration, and restarts its counting endlessly until an output is turned on (1 second minimum). Any ADC value, and any time duration can be selected via single-character keyboard inputs (full customization). More commands are listed below. 

### Dual-PWM Output Conditional Statement

The final outputs are controlled by a master if-statement stating logically: when Low Bound is on, and when OFF-Timer has passed, and when ON-Timer has passed, turn on the Low Output (see dark image of LEDs above); when High Bound is on, and when OFF-Timer has passed, and when ON-Timer has passed, turn on the High Output (see bright image of LEDs above). 

There are two types of outputs that are controlled by the two master if-statements: a logic 1/0 control signal, and a PWM signal with 90% duty cycle. The control signal allows the controller program to actuate any device requiring an on-off signal (lights, humidifiers, heaters, switches, etc.). The PWM signal allows the controller program to actuate any device requiring a continuous square-wave signal (motors, fans, pumps, gears, etc.).

## Installing on MSP430G2553

1. Plug the mini-USB plug between the MSP430G2553 and a PC. 
2. Open CoolTerm, and assign the serial port to COM4 (or whatever port # the PC reads the board from; look in Device Manager). 
3. Wire the various GPIO outputs to LEDs or other devices, using a breadboard or the provided MSP430G2553 LED interface PCB design. 
4. If using the LED Interface PCB, connect the PCB board to a +5V USB power supply (see below for more details on the two PCB boards). 
5. Wire the two PWM output signals to devices that operate on +3.3V maximum, the amplitiude of the PWM signals. 
6. If power-intensive square-wave devices, such as +12V DC motors, are wired to the Interface PCB board (or breadboard), then attach the PWM signal amplifier PCB design to the PWM signals being directly outputted from the MSP430G2553, for converting the +3.3V PWM amplitude to a +12V PWM amplitude. 
7. For the input to the ADC pin, the only input of the universal sensor controller, any sensor outputting a voltage range of or somewhere within 0V to +3V can be wired to the P1.3 pin on the MSP430G2553.

Pins used:

```
P1.0 = GPIO output pin: LED turns on when program is currently writing text to PC, after a command input. (UV LED)
P1.1 = RX UART pin.
P1.2 = TX UART pin.
P1.3 = ADC input pin, for connect to any sensor with 0V to +3V voltage output.
P1.4 = GPIO output pin: LED indicates when low output PWM is on. (Purple LED)
P1.5 = GPIO output pin: LED indicates when high output PWM is on. (Blue LED)
P1.6 = GPIO output pin: LED blinks any time a new ADC value is measured, and/or printed to the PC. (White LED)
P1.7 = GPIO output pin: status bit LED indicates when both Timers are ready and have begun counting. (white LED)
P2.0 = GPIO output pin: LED indicates when Low Bound detects an ADC range lower than its specified value. (Red LED)
P2.1 = GPIO output pin: LED indicates when High Bound detects an ADC range higher than its specified value. (Orange LED)
P2.2 = PWM output pin: Low Output.
P2.3 = GPIO output pin: LED indicates when ON-Timer duration has passed. (Yellow LED)
P2.4 = PWM output pin: High Output.
P2.5 = GPIO output pin: LED indicates when OFF-Timer duration has passed. (Green LED)
```

## The UART Text-Printed Program

Prints to the PC via a command window all of the following, but not limited to: 
instructions, 
- lists of commands and descriptions, 
- real-time values of Bound settings and Timer durations while adjusting, 
- real-time ADC measurements and Timer increments, 
- statuses of the controller (when the Timers have begun counting). 

### Initial Startup Instructions:

1. Upon booting up the sensor controller program (“main.c” file; 1362 lines), text begins printing to the PC via a command window. 
2. After pressing the ‘r’ key on the PC keyboard, many more lines of text illustrate a command-prompt like user-friendly interface. 
3. There are initial instructions printed that assist the user in setting up the controller. 
4. Two lists of commands are printed along with the instructions: Main commands, and Other commands. 

> Note: Before any Timers begin counting or Bounds begin comparing ADC values, the OFF-Timer duration and ON-Timer duration must first be given a specified time-length in seconds, minutes, and/or hours (maximum 8 hour OFF-Timer; maximum 5 minute ON-Timer). 

### Single-Character Keyboard Commands

The image below is a useful keyboard-template showing all the available keyboard commands that are currently coded into the sensor controller program. 

> Note: the program DOES include uppercase and lowercase commands, such as incrementing and decrementing the 4 adjustable timer/bound values. The cases of the letters in the image below do not pertain to only uppercase commands.

![alt text](https://github.com/searingwolfe/MSP430-universal-sensor-controller-UART/blob/master/Universal_Sensor_Controller_UART_Program/availablecommandsController.jpg)

- The red commands can be inputted at any time. 
- The green commands serve as enter/exit mode keys for setting the Low Bound, High Bound, OFF Timer, and ON Timer. While in one of these set modes, another mode cannot be inputted without first exiting the current one. 
  - While in a Timer set mode, both Timers will stop counting and outputs will be disabled. 
  - The Timers will restart counting once the mode is exited (a new time is set), but only if both Timers have both been initialized. 
  - A white LED in the demo video, or P1.7 on the MSP430G2553, indicates when both Timers are set and have begun counting. 
- The blue commands can only be inputted while within a corresponding green/main command’s set mode.

The table below lists all available single-character keyboard commands used for the universal sensor controller program, including descriptions, and which modes they are limited to.

| Single-Character Command:        | Mode(s) Each is Limited to:           | Descrption of Command  |
|:-------------:|:-------------:|:-----:|
| r      | After startup or after a hard-reset. | Turns ON the controller, enables all other commands. |
| R      | After controller is turned ON. | Forces a hard-reset, resets all bound settings and timer durations. |
| e      | After controller is turned ON. | Un-pause the constant printing of ADC values, or Timer increments. |
| E      | After controller is turned ON. | Pause the constant printing of ADC values, or Timer increments. |
| w      | After controller is turned ON. | Print values of measured ADC in real-time (once per second). |
| W      | After controller is turned ON. | Print values of current increments within OFF-Timer and ON-Timer. |
| l      | While in Low Bound set mode. | Exit Low Bound set mode. |
| L      | After controller is ON. Must not be in any set modes. | Enter Low Bound set mode. Adjustable range:   0 – 510. |
| i      | While in Low Bound set mode. | Decrement Low Bound’s ADC value by 1. |
| I      | While in Low Bound set mode. | Increment Low Bound’s ADC value by 1. |
| o      | While in Low Bound set mode. | Decrement Low Bound’s ADC value by 10. |
| O      | While in Low Bound set mode. | Increment Low Bound’s ADC value by 10. |
| p      | While in Low Bound set mode. | Decrement Low Bound’s ADC value by 50. |
| P      | While in Low Bound set mode. | Increment Low Bound’s ADC value by 50. |
| h      | While in High Bound set mode. | Exit High Bound set mode. |
| H      | After controller is ON. Must not be in any set modes. | Enter High Bound set mode. Adjustable range:   510 – 1020. |
| t      | While in High Bound set mode. | Decrement High Bound’s ADC value by 1. |
| T      | While in High Bound set mode. | Increment High Bound’s ADC value by 1. |
| y      | While in High Bound set mode. | Decrement High Bound’s ADC value by 10. |
| Y      | While in High Bound set mode. | Increment High Bound’s ADC value by 10. |
| u      | While in High Bound set mode. | Decrement High Bound’s ADC value by 50. |
| U      | While in High Bound set mode. | Increment High Bound’s ADC value by 50. |
| z      | After controller is ON. Must not be in any set modes. | Enter OFF-Timer set mode. Adjustable range:   10 seconds – 8 hours. |
| Z      | While in OFF-Timer set mode. | Exit OFF-Timer set mode. |
| x      | While in OFF-Timer set mode. | Decrement OFF-Timer’s duration value by 1 second. |
| X      | While in OFF-Timer set mode. | Increment OFF-Timer’s duration value by 1 second. |
| c      | While in OFF-Timer set mode. | Decrement OFF-Timer’s duration value by 1 minute. |
| C      | While in OFF-Timer set mode. | Increment OFF-Timer’s duration value by 1 minute. |
| v      | While in OFF-Timer set mode. | Decrement OFF-Timer’s duration value by 1 hour. |
| V      | While in OFF-Timer set mode. | Increment OFF-Timer’s duration value by 1 hour. |
| a      | After controller is ON. Must not be in any set modes. | Enter ON-Timer set mode. Adjustable range:   1 second – 5 minutes. |
| A      | While in ON-Timer set mode. | Exit ON-Timer set mode. |
| s      | While in ON-Timer set mode. | Decrement ON-Timer’s duration value by 1 second. |
| S      | While in ON-Timer set mode. | Increment OFF-Timer’s duration value by 1 second. |
| d      | While in ON-Timer set mode. | Decrement OFF-Timer’s duration value by 1 minute. |
| D      | While in ON-Timer set mode. | Increment OFF-Timer’s duration value by 1 minute. |
| /      | After controller is turned ON. | A help menu for seeing more info on each command (unfinished). |

### Known Bugs, Errors, and Quick-Fixes:

Depending on which version of the controller’s C program code the user has (fixing more bugs before deadline), a critical error may occur that stops the outputs from activating. 

- In older versions, the Timers and Bounds never trigger an output if ADC values are currently being printed. 
  - The user must pause the printing by pressing ‘E’ to immediately fix the error
  - Outputs and Timers will then work properly.

Minor bugs can occur when inputting keyboard commands too quickly, and when trying to perform some other command while in a set mode. Other, more serious bugs/flaws in the user-interface portion of the code include but are not limited to:

> While in a Bound set mode (sometimes called Bound adjust mode), the Timers will continue counting, and the outputs can be turned on if a Bound is accidentally set too high or too low. Thus, the Bounds should be set before the two Timers are initialized. 

> Typically the program will be observable as nonfunctioning when the OFF-Timer and a Bound are on, but the ON-Timer keeps turning on and off and neither output will respond.

> A quick fix to the error above involves reinitializing the two Timers by pressing the key-combination: “z” and “Z”, then “a” and “A” (the two set mode commands be entered in either order).

> If the outputs still do not respond, the sensor controller program must hard-reset by pressing ‘R’. 


## PCB Board Designs to Improve Universality:

These two PCB board designs allow for much more convenient and practical integration of the sensor controller within any larger enveloping device/system. A very specific amplifier circuit is required for a wide range of sensors that output a voltage range outside the 0V to +3V requirement of the MSP430G2553 (see the sensor’s datasheet, or measure the outputs with a multi-meter). 

### MSP430G2553 LED Interface PCB Board

This PCB board serves to provide the user with easy access to LEDs and other GPIO outputs for the MSP430G2553 IC chip (without board). To improve the universality, ease of installation, and speed of testing the various pins, this PCB board converts a typical +5V USB cable into a +3.3V power supply for the MSP430G2553, using an LM3940 voltage regulator. 

>Note: In order for the MSP430G2553 to function properly while disconnected from its red programming board, the “RST” pin must be wired to power, but must have an RC circuit before the power supply pin to provide enough time-delay for the “RST” pin to trigger off and stay off. 

- All GPIO outputs used for the outputs, Timers, Bounds, and status bits, are wired directly to LEDs. 
- The PWM outputs are fed straight to the Headers without going through any more components (the PWM signal amplifier PCB connects to these two PWM outputs for controlling stronger DC motors). 

#### Schematic Diagram & Header Labels

![alt text](https://github.com/searingwolfe/MSP430-universal-sensor-controller-UART/blob/master/MSP430G2553_LED_Board/MSP430G2553_LED_Board.jpg)

Schematic of the MSP430G2553 LED interface PCB board:
- The labels of the 7 lower headers are provided on the left. 
- The +5V power supply and GND supply come from a male USB plug, connecting to an appropriate +5V or +5.1V (1A) wall-outlet power-adapter.

#### PCB Design (Front/Back)

![alt text](https://github.com/searingwolfe/MSP430-universal-sensor-controller-UART/blob/master/MSP430G2553_LED_Board/MSP430G2553_LED_BoardPCB3.jpg)
![alt text](https://github.com/searingwolfe/MSP430-universal-sensor-controller-UART/blob/master/MSP430G2553_LED_Board/MSP430G2553_LED_BoardPCB4.jpg)

Front and back faces of the MSP430G2553 LED interface PCB board; realistic images. 
- Dimensions are, width: 74.93 mm, height: 57.15 mm. 
  - The PCB board has through-hole pins before all resistors and LEDs, for attaching other devices to the MSP430G2553’s GPIO pins (outputting 1/0 signals). 
  - An extra through-hole pin, other than the Headers, is also provided for the two PWM outputs. 
  - Three through-hole pins are also above the LM340 (5V-3.3V regulator) in case a heat sink, thermistor, or other temperature sensor must be placed next the heat-emitting IC regulator.

### PWM Signal Amplifier PCB Board

This PCB board serves as a miniature amplifier circuit for the two PWM signals outputting from the sensor controller program within the MSP430G2553. 
- Originally designed for two simple +12V DC motors (200-300mA each); MSP430G2553 outputs a PWM amplitude of +3.3V:
  - Powerful N-channel MOSFET switches (STP16NF06) amplify the two PWM outputs. 
  - The minimum VGS (threshold voltage at gate pin) rating of the STP16NF06 is +2V. 
  - A preliminary transistor (2N2222 NPN BJT) first amplifies the current +3.3V PWM amplitude to a +5V PWM amplitude. 
  - The 2N2222 transistors enforce the saturation mode for the STP16NF06 switches, allowing them to trigger on/off properly. 

> Note: STP16NF06 MOSFET switches are rated for up to 60V and 16A. Use proper resistor values, and high-power rated components (P=IV; 60*16 = 960W maximum) should allow for almost any square-wave controlled device, such as the +12V DC motor peristaltic pumps, to connect and react to the sensor controller.

#### Schematic Diagram & Header Labels

![alt text](https://github.com/searingwolfe/MSP430-universal-sensor-controller-UART/blob/master/PWM_Motor_Interface/PWM_Motor_Interface.jpg)

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

## Software Tools Used

* [Code Composer Studio](https://dev.ti.com/) - Code compiling program (CCS). 
* [CoolTerm](http://freeware.the-meiers.org/) - Used to print characters from TX/RX UART to PC

## Example Images of Controller Program in CoolTerm

![alt text](https://github.com/searingwolfe/MSP430-universal-sensor-controller-UART/blob/master/Universal_Sensor_Controller_UART_Program/CoolTerm_ControllerTextProgram1.jpg)
![alt text](https://github.com/searingwolfe/MSP430-universal-sensor-controller-UART/blob/master/Universal_Sensor_Controller_UART_Program/CoolTerm_ControllerTextProgram2.jpg)
![alt text](https://github.com/searingwolfe/MSP430-universal-sensor-controller-UART/blob/master/Universal_Sensor_Controller_UART_Program/CoolTerm_ControllerTextProgram3.jpg)

Also stated previously, there are various lines of text that relate to a pH pump controller. This was the original function of the non-UART version of the universal sensor controller (had 8 possible options for each Bound and each Timer, selected by buttons). The constantly printed value above shows "Current pH: #", but in the code this line was changed to "Current ADC: #" before being pushed to the repository, to reduce a bit of confusion before submitting. Changing the rest of the "pH" printed lines throughout the program takes a bit of time. 

- Remember: In older versions, the Timers and Bounds never trigger an output control 1/0 signal or an output PWM signal if ADC values are currently being printed. 
  - The user must pause the printing by pressing ‘E’ to immediately fix the error
  - Outputs and Timers will then work properly.
  
 > This critical error will be fixed as soon as possible. Other bugs will be uncovered and deleted.
