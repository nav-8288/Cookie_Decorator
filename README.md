# Cookie_Decorator
CSE453 (SPRING 2026)- Cookie Decorator repository

Current firmware progress

Inputs implemented + debounced (Bounce2): Print button, Kill switch, Refill/Clean extruder button, and Cookie Size switch are read with INPUT_PULLUP and debounced using Bounce2 (15ms). Events (fell/rose/changed) are printing to Serial for verification. 

4-motor pin mapping set up: STEP/DIR pins are defined for X/Y/Z/Extruder and initialized as outputs. STEP pins default LOW and DIR pins default LOW to start in a known state.

Driver enable control added (software-controlled EN): ENABLE pins are defined per driver and currently initialized HIGH (disabled) for safety; later logic will enable them only during motion and disable on Kill.

Design doc requirements reflected in firmware plan

User controls / modes:

Print starts decorating process (Printing mode).

Kill stops all motion immediately and returns to Idle.

Refill/Clean enters a service mode where the extruder releases the piston for removal/refill.

Cookie Size switch selects Small vs Large and changes motion scaling/steps. 



TO IMPLEMENT:
Status LED behavior:

Blue = Idle

Yellow = Printing

Green = Completed

Red = Needs Refill/Clean 

Microstepping configuration: MS1/MS2/MS3 set HIGH for 1/16 microsteps on all four drivers (hardware-wired). 

A4988 wiring assumptions: SLEEP and RESET tied together and pulled to +5V to prevent sleeping/resetting (hardware-wired). 

Note: The design doc mentions ENABLE tied to GND (always enabled). We’re moving toward Arduino-controlled ENABLE pins so KillSWITCH can fully disable drivers (safer). We should make sure wiring matches whichever approach we standardize on. 

Next steps:

Motor  tests (one axis at a time): Implement a small stepper “pulse” helper and confirm X, then Y, Z, and Extruder can move reliably via button-triggered test routines.

Kill behavior: Ensure Kill immediately stops stepping and disables all drivers (EN HIGH).

Limit switches + homing (NEW): Add limit switch pins and logic so the machine can home X/Y/Z and avoid crashing into end stops. (Design doc lists limit switches as required hardware for knowing motor endpoints.) 

State machine: Implement core modes: IDLE, PRINTING, REFILL_CLEAN, COMPLETED, KILLED, and hook in RGB LED state changes accordingly. 

Print path + cookie size scaling: Use cookie size switch (Small/Large) to adjust step counts so the pattern aligns with the selected cookie size.
