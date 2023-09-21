## The UART Text-Printed Program

Prints to the PC via a command window all of the following, but not limited to: 
instructions, 
- lists of commands and descriptions, 
- real-time values of Bound settings and Timer durations while adjusting, 
- real-time ADC measurements and Timer increments, 
- statuses of the controller (when the Timers have begun counting). 

### Initial Startup Instructions:

1. Upon booting up the sensor controller program (“main.c” file; 1362 lines), text begins printing to the PC via a command window. 
2. After pressing the ‘r’ key on the PC keyboard, many more lines of text illustrate a command-prompt like user-friendly interface (see Figure 8). 
..2. There are initial instructions printed that assist the user in setting up the controller. 
3. Two lists of commands are printed along with the instructions: Main commands, and Other commands. 

> Note: Before any Timers begin counting or Bounds begin comparing ADC values, the OFF-Timer duration and ON-Timer duration must first be given a specified time-length in seconds, minutes, and/or hours (maximum 8 hour OFF-Timer; maximum 5 minute ON-Timer). 

### Single-Character Keyboard Commands

The image below is a useful keyboard-template showing all the available keyboard commands that are currently coded into the sensor controller program. 

> Note: the program DOES include uppercase and lowercase commands, such as incrementing and decrementing the 4 adjustable timer/bound values. The cases of the letters in the image below do not pertain to only uppercase commands.

![alt text](https://github.com/searingwolfe/MSP430-universal-sensor-controller-UART/blob/master/Universal_Sensor_Controller_UART_Program/availablecommandsController.jpg "")

- The red commands can be inputted at any time. 
- The green commands serve as enter/exit mode keys for setting the Low Bound, High Bound, OFF Timer, and ON Timer. While in one of these set modes, another mode cannot be inputted without first exiting the current one. 
..- While in a Timer set mode, both Timers will stop counting and outputs will be disabled. 
..- The Timers will restart counting once the mode is exited (a new time is set), but only if both Timers have both been initialized. 
..- A white LED in the demo video, or P1.7 on the MSP430G2553, indicates when both Timers are set and have begun counting. 
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
..- The user must pause the printing by pressing ‘E’ to immediately fix the error
..- Outputs and Timers will then work properly.

Minor bugs can occur when inputting keyboard commands too quickly, and when trying to perform some other command while in a set mode. Other, more serious bugs/flaws in the user-interface portion of the code include but are not limited to:

> While in a Bound set mode (sometimes called Bound adjust mode), the Timers will continue counting, and the outputs can be turned on if a Bound is accidentally set too high or too low. Thus, the Bounds should be set before the two Timers are initialized. 

> Typically the program will be observable as nonfunctioning when the OFF-Timer and a Bound are on, but the ON-Timer keeps turning on and off and neither output will respond.

> A quick fix to the error above involves reinitializing the two Timers by pressing the key-combination: “z” and “Z”, then “a” and “A” (the two set mode commands be entered in either order).

> If the outputs still do not respond, the sensor controller program must hard-reset by pressing ‘R’. 

## Example Images of Controller Program in CoolTerm

![alt text](https://github.com/searingwolfe/MSP430-universal-sensor-controller-UART/blob/master/Universal_Sensor_Controller_UART_Program/CoolTerm_ControllerTextProgram1.jpg "")
![alt text](https://github.com/searingwolfe/MSP430-universal-sensor-controller-UART/blob/master/Universal_Sensor_Controller_UART_Program/CoolTerm_ControllerTextProgram2.jpg "")
![alt text](https://github.com/searingwolfe/MSP430-universal-sensor-controller-UART/blob/master/Universal_Sensor_Controller_UART_Program/CoolTerm_ControllerTextProgram3.jpg "")