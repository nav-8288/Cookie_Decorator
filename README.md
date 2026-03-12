Firmware Progress Summary

Current firmware includes debounced input handling using the Bounce2 library.

Implemented user inputs:

Print switch: A9

Kill pushbutton: A5

Refill/Clean pushbutton: A0

Cookie size LARGE switch: D2

Cookie size SMALL switch: D3

STEP and DIR pins are mapped for all four motion channels:

X axis: DIR D6, STEP D7

Y axis: DIR D8, STEP D9

Z axis: DIR D10, STEP D11

Extruder: DIR D12, STEP D13

Driver enable pins are assigned as:

X_ENABLE = D4

Y_ENABLE = D5

Z_ENABLE = D22

EX_ENABLE = D23

A4988 driver ENABLE is treated as active LOW:

drivers start disabled for safety

drivers are enabled only during motion

kill disables all drivers immediately

Basic firmware states are implemented:

IDLE

PRINTING

COMPLETED

REFILL_CLEAN

KILLED

NEEDS_REFILL is defined for future use but not fully implemented yet

RGB LED state feedback is working:

Blue = IDLE

Yellow = PRINTING

Green = COMPLETED

Red = KILLED

Purple = REFILL_CLEAN

Cookie size feedback is also implemented:

SMALL = orange flash

LARGE = white flash

LED returns to the active system-state color after the flash ends

Cookie size scaling is implemented:

LARGE uses full scale

SMALL uses a reduced scale based on the ratio 2.375 / 4.25

A first UB logo motion prototype is implemented using a normalized point list:

current path is a rough placeholder, not the final UB outline

points are scaled into mm dimensions

mm motion is converted into step counts using placeholder X_STEPS_PER_MM and Y_STEPS_PER_MM values

Coordinated XY motion is implemented using a Bresenham-style stepping routine:

X and Y move together along straight path segments

current motion testing is focused on shape, scaling, and control flow

Kill behavior is integrated into the motion routines:

active motion stops immediately when kill is triggered

all drivers are disabled

LED changes to red

system enters KILLED

Refill/Clean mode is partially implemented:

entering refill mode sets the LED to purple

extruder performs a short reverse jog

pressing the refill/clean button again exits the mode

Current limitations:

extrusion during print motion is not implemented yet

limit switch logic is not added yet

homing is not implemented yet

end-stop protection is not implemented yet

final UB logo path is not implemented yet

X_STEPS_PER_MM and Y_STEPS_PER_MM still need calibration

Next steps:

verify X/Y direction and motion scale

calibrate steps/mm

replace the placeholder UB path with a more accurate design

integrate extrusion during print segments

add limit switches and homing logic

strengthen state restrictions between print, refill, and kill modes
