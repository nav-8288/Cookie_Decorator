CSE453 (SPRING 2026) – Cookie Decorator Firmware (Progress Update)
Current firmware progress
Inputs implemented + debounced (Bounce2)

Print button (A9), Kill switch (A5), Refill/Clean button (A0) are read using INPUT_PULLUP and debounced with Bounce2 (15ms).

Cookie size selection uses two separate inputs:

LARGE switch = D2

SMALL switch = D3

Button events are used to trigger behavior (fell, rose, changed), and key actions print to Serial for verification.

4-motor pin mapping set up (X/Y/Z/Extruder)

STEP/DIR pins are defined for X, Y, Z, and Extruder:

X: DIR D6, STEP D7

Y: DIR D8, STEP D9

Z: DIR D10, STEP D11

Extruder: DIR D12, STEP D13

ENABLE pins are defined per driver:

X_EN D4, Y_EN D5, Z_EN D22, EX_EN D23

Driver SLEEP/RESET pins are wired to +5V physically

Driver enable control added (software-controlled EN)

ENABLE is treated as active LOW (A4988 style):

Drivers start disabled (EN HIGH) for safety.

Drivers are enabled only during movement (enableDriver() sets EN LOW).

Kill stops motion and disables all drivers immediately (disableAllDrivers() sets all EN HIGH).

Design doc requirements reflected in firmware behavior
User controls / modes

Print (A9): starts the “printing” motion routine (PRINTING mode).

Kill (A5): stops all motion immediately and disables all drivers (KILLED mode).

Refill/Clean (A0): enters a service mode (REFILL_CLEAN) to support extruder piston removal/refill (extruder motion will be expanded later once the extruder driver is connected).

Cookie Size (D2/D3): selects SMALL vs LARGE and scales the print path accordingly.

Status LED behavior (implemented)

RGB LED is now tied into the firmware states:

Blue = IDLE

Yellow = PRINTING

Green = COMPLETED

Red = KILLED

Purple = REFILL/CLEAN

Cookie size feedback:

SMALL = Orange flash

LARGE = White flash

The cookie-size LED flashes briefly, then returns to the current state color automatically.

Motion / “Minimap” path prototype (NEW)
UB logo motion test (XY only)

A first “minimap-style” path is implemented using a Pt {x, y, extrude} point list.

The code currently contains a placeholder UB path (rough U + B outline) in normalized 0–1 coordinates.

Firmware converts the normalized path into mm and then into step counts using:

X_STEPS_PER_MM and Y_STEPS_PER_MM (currently placeholder values; calibration TBD)

A custom moveXY() routine steps X and Y together (Bresenham-style stepping), allowing straight-line XY motion rather than “stair-step” movement.

Note: The extrude flag exists in the path points, but extrusion is not enabled yet. Current testing is focused on validating XY path shape, scaling, and control flow.

Microstepping configuration

MS1/MS2/MS3 configured for 1/16 microstepping on all A4988 drivers (hardware wiring).

X_STEPS_PER_MM / Y_STEPS_PER_MM are currently set as placeholders and will be calibrated once consistent XY motion is confirmed.

Wiring assumptions / notes

A4988 ENABLE is active LOW and is being controlled by the Arduino (safer than always-enabled wiring).

SLEEP and RESET:

Firmware supports driving them HIGH for “awake/not reset” if wired to Arduino pins,

but final wiring may also use a fixed +5V tie depending on the team’s hardware standard.

Next steps
Hardware validation

Confirm reliable stepping on X and Y with consistent direction and expected scale.

Calibrate steps/mm for X and Y based on pulley/leadscrew configuration.

Limit switches + homing (upcoming)

Add limit switch pins and logic so the machine can:

home X/Y/Z

prevent crashing into end stops

Use the same “kill-style” checks to stop motion safely when a limit switch triggers.

Printing path improvements

Replace the rough point list with a more accurate UB outline (likely SVG-traced points).

Implement extrusion logic:

start with simple “extrude-per-segment”

then integrate extrusion while moving for smoother lines.

Full state machine integration

Enforce mode restrictions more strictly:

prevent PRINT during REFILL_CLEAN

prevent cookie-size changes from affecting an active print job

require kill release before re-entering motion states
