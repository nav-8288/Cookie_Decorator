CSE 453 (Spring 2026) – Cookie Decorator Firmware Progress Update
Overview

The current firmware establishes the early control framework for the Cookie Decorator system. It now includes debounced user inputs, RGB LED state feedback, software-controlled A4988 driver enabling, cookie size selection and scaling, a basic state machine, and a first coordinated XY motion prototype for tracing a placeholder UB logo path.

At this stage, the firmware is mainly focused on validating control flow, safety behavior, and XY motion. Extrusion during printing, homing, and limit-switch logic are still upcoming.

Current Implemented Features
1. Debounced user inputs using Bounce2

The firmware uses the Bounce2 library with INPUT_PULLUP inputs and a 15 ms debounce interval for the following controls:

Print switch: A9

Kill pushbutton: A5

Refill/Clean pushbutton: A0

Cookie size LARGE switch: D2

Cookie size SMALL switch: D3

The code uses Bounce2 event methods such as:

fell() for button press detection

rose() for button release detection

changed() for switch changes

Serial messages are printed for verification during testing.

2. 4-motor pin mapping configured

STEP and DIR control pins are currently assigned for all four motion channels:

X axis: DIR D6, STEP D7

Y axis: DIR D8, STEP D9

Z axis: DIR D10, STEP D11

Extruder: DIR D12, STEP D13

Driver enable pins are assigned as:

X_ENABLE = D4

Y_ENABLE = D5

Z_ENABLE = D22

EX_ENABLE = D23

This sets up the firmware-side pin structure for the full 4-motor system.

3. A4988 enable control implemented

The firmware treats A4988 ENABLE as active LOW.

Current behavior:

drivers are initialized disabled for safety

enableDriver() sets an enable pin LOW

disableAllDrivers() sets all enable pins HIGH

drivers are only enabled during motion

kill immediately disables all drivers

This provides a safer default behavior than leaving the stepper drivers always enabled.

4. Basic state machine added

The firmware currently uses the following system states:

IDLE

PRINTING

COMPLETED

NEEDS_REFILL

REFILL_CLEAN

KILLED

The core active states in the current code are:

IDLE

PRINTING

COMPLETED

REFILL_CLEAN

KILLED

NEEDS_REFILL has been defined for future expansion, but it is not yet fully integrated into the behavior logic.

5. RGB LED state feedback working

The RGB LED reflects the current machine state:

Blue = IDLE

Yellow = PRINTING

Green = COMPLETED

Red = KILLED

Purple = REFILL_CLEAN

The firmware also provides temporary cookie-size feedback through the LED:

SMALL = orange flash

LARGE = white flash

After the size flash expires, the LED automatically returns to the current state color.

6. Cookie size selection and scaling implemented

The firmware supports two cookie size selections:

SIZE_SMALL

SIZE_LARGE

On startup, initCookieSizeFromSwitches() reads the physical switch positions and selects the initial cookie size. The firmware also watches for runtime changes and updates the active size accordingly.

The print path uses a uniform scale factor through:

float cookieScale()

Current scaling behavior:

LARGE = full scale (1.0)

SMALL = scaled down using the ratio 2.375 / 4.25

This allows one path definition to be reused for multiple cookie sizes.

7. UB logo minimap path prototype added

A first point-based motion prototype is now implemented for the UB logo using:

struct Pt { float x; float y; bool extrude; };

The current ub_path[] contains a rough placeholder outline of the UB logo using normalized coordinates from 0.0 to 1.0.

The function:

runMinimapPath(const Pt *path, int n, int pulseUs)

does the following:

reads the cookie size scale

converts normalized path coordinates into mm dimensions

converts mm dimensions into step counts

calls moveXY() for each segment

The current design area is:

width = 30.0 mm * scale

height = 20.0 mm * scale

This is still an early motion test path and not the final traced UB design.

8. Coordinated XY motion implemented

The moveXY() function performs coordinated two-axis stepping using Bresenham-style logic.

This allows the X and Y motors to move together along straight segments instead of moving one axis completely before the other. That gives better path behavior for future printing.

Current moveXY() behavior includes:

enabling X and Y drivers during motion

setting axis direction from the sign of the step count

stepping both axes based on accumulated error

stopping immediately if kill is triggered

This is the first real coordinated XY drawing routine in the firmware.

9. Kill logic integrated into motion

Kill behavior is active both in the main control loop and inside movement routines.

When kill is triggered:

killRequested becomes true

system state changes to KILLED

all drivers are disabled

LED changes to red

current motion exits immediately

This kill protection is already built into:

moveXY()

jogAxis()

10. Refill/Clean mode prototype added

The refill/clean button toggles a service mode using REFILL_CLEAN.

Current behavior:

entering refill/clean mode sets the LED purple

the extruder motor performs a short reverse jog

pressing the refill/clean button again exits the mode and returns the system to idle

This currently serves as a simple extruder service test and will be expanded later.

Current Limitations

The following features are not finished yet:

limit switch logic

homing for X, Y, and Z

end-stop protection

active extrusion during print motion

final UB logo geometry

stricter mode restrictions during printing/refill/kill

automatic refill detection

full NEEDS_REFILL behavior

calibration of X_STEPS_PER_MM and Y_STEPS_PER_MM

The extrude field already exists in the point list, but it is not currently used to drive the extruder during path execution.

Next Steps
Motion validation

verify correct X and Y motor direction

confirm expected physical scale of movement

calibrate X_STEPS_PER_MM

calibrate Y_STEPS_PER_MM

Path refinement

replace the placeholder UB path with a more accurate traced outline

refine dimensions and margins

validate scaling on both cookie sizes

Extrusion integration

begin using the extrude flag during print segments

add synchronized extruder motion with XY travel

test frosting flow behavior

Safety and machine control

add limit switch pins and logic

implement homing for X/Y/Z

prevent motion into end stops

enforce stronger restrictions between PRINTING, REFILL_CLEAN, and KILLED
