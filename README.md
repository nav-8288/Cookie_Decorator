Functional behavior

1. Startup

On setup, the code:

initializes pins
attaches debounce handlers
attaches kill interrupt
initializes all drivers
homes the machine using returnToHome()
enters IDLE
reads cookie size switches




2. State machine

The sketch uses these states:
IDLE
SIZE_SMALL_PRINT
SIZE_LARGE_PRINT
COMPLETED
NEEDS_REFILL
REFILL_CLEAN
KILLED

STATE: IDLE

Machine waits for user input.

Behavior:

reads cookie size switches
print button:
if small selected → SIZE_SMALL_PRINT
if large selected → SIZE_LARGE_PRINT
if no size selected → toggles extruder jog/prime mode
refill button:
enters REFILL_CLEAN
SIZE_SMALL_PRINT / SIZE_LARGE_PRINT

Runs the print sequence:

move to print start offset
print logo path
return home
go to COMPLETED

Scaling depends on cookie size. Small uses a reduced scale factor; large uses full scale.

STATE: COMPLETED

LED shows complete state, then returns to IDLE.

STATE: NEEDS_REFILL

Entered when EX_LIMIT_SW is pressed during motion.

Behavior:

stops motion
disables extruder
waits for refill button
on refill button press, returns to previous state

Note: current implementation returns to the previous state, not the previous position in the job.

STATE: REFILL_CLEAN

Can only be entered from IDLE.

Behavior:

extruder disabled
special LED color shown
pressing refill button again exits back to IDLE

STATE: KILLED

Entered on kill interrupt.

Behavior:

disables all drivers
stops jog mode
LED shows killed state

On kill release:

drivers reinitialized
machine attempts to return home
state goes to IDLE



Motion system
XY movement

moveXY() uses a Bresenham-style synchronized stepping approach to move X and Y together for straight-line segments.

Features:

direction pins set from sign of target delta
X and Y steps interleaved for coordinated motion
checks kill and refill limit during motion
optionally steps extruder during print states
Z movement

liftTool() and lowerTool() call jogAxis() with configured Z directions and step counts. Z parameters are defined in the config header.

Extruder

Two modes:

Prime/Jog mode in IDLE
toggled by print button when no cookie size is selected
extruder steps continuously using timed intervals
Print mode
extruder steps are tied to XY motion
extrusion frequency controlled by EXTRUDER_PRINT_STEP_DIVIDER in config
Homing behavior
XY homing

homeXYAxes():

drives both axes toward home switches
stops each axis when its switch is hit
backs both axes off by 400 steps
leaves step outputs stopped
Z homing

homeZAxis():

drives Z downward until bottom switch is pressed
backs upward 200 steps
Return home

returnToHome() calls:

homeXYAxes()
homeZAxis()
LED status behavior

Current LED mapping:

KILLED → red
COMPLETED → green
NEEDS_REFILL → yellow
REFILL_CLEAN → magenta
SIZE_SMALL selected in idle → orange
SIZE_LARGE selected in idle → white
no size selected / default idle → blue
Configurable parameters

From the config header:

Motion calibration
X_STEPS_PER_MM = 160.0
Y_STEPS_PER_MM = 160.0
Z tuning
Z_PULSE_US = 800
Z_LIFT_STEPS = 1600
Z_LOWER_STEPS = 1600
Z_UP_DIR = false
Z_DOWN_DIR = true
Print start offset
PRINT_START_X_MM = 15.0
PRINT_START_Y_MM = 15.0
Extruder tuning
EXTRUDER_PRIME_DIR = true
EXTRUDER_PRINT_DIR = true
EXTRUDER_PULSE_US = 800
EXTRUDER_PRIME_INTERVAL_US = 2500
EXTRUDER_PRINT_STEP_DIVIDER = 3
Logo path spec

The print graphic is stored as an array of PlotPoint items, each containing:

x
y
penDown

The uploaded logo data contains NUM_POINTS = 582.

The code scales the imported logo bounds from:

X: 0.0 to 60.0
Y: 0.0 to 30.11

into a print area based on cookie size.
