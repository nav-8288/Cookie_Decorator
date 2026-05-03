#ifndef COOKIE_CONFIG_H
#define COOKIE_CONFIG_H
#include <math.h>

/*INCLUDE LOGIC FOR LIMIT SWITCHES */
// pins 2, 3, 18, 19, 20 and 21 can only be used for interrupts

/*
  LED PINS — must be PWM-capable on the Mega.
  Mega PWM pins: 2,3,4,5,6,7,8,9,10,11,12,13,44,45,46
  OLD: R=43 (no PWM), G=45 (PWM ok), B=47 (no PWM)  <- analogWrite silently failed
  FIX: R=44, G=45, B=46  <- all three are hardware PWM on the Mega
  ACTION REQUIRED: move your LED R wire to pin 44, B wire to pin 46.
*/
#define LED_R 44  /* PWM pin — Mega OC5C */
#define LED_G 45  /* PWM pin — Mega OC5B */
#define LED_B 46  /* PWM pin — Mega OC5A */

#define print_SW          A9  /* dig pin A9  */
#define kill_PB            2  /* dig pin 2 — interrupt capable */
#define refill_clean_PB   A0  /* dig pin A0  */
#define cookie_size_LARGE 30  /* dig pin D30 */
#define cookie_size_SMALL 32  /* dig pin D32 */

/* FOR X AXIS */
#define X_DIR  6
#define X_STEP 7

/* FOR Y AXIS */
#define Y_DIR  8
#define Y_STEP 9

/* FOR Z AXIS */
#define Z_DIR  10
#define Z_STEP 11

/* ENABLE PINS FOR DRIVERS (active LOW) */
#define X_ENABLE  4
#define Y_ENABLE  5
#define Z_ENABLE 22

#define Y_LIMIT_SW 24  /* NC: normal=LOW, triggered=HIGH */
#define X_LIMIT_SW 25
#define Z_LIMIT_SW 26

#define ACT_R_PWM 12  /* extend  side PWM */
#define ACT_L_PWM 13  /* retract side PWM */
#define ACT_L_IS  A6  /* current sense — retract side */
#define ACT_R_IS  A7  /* current sense — extend  side */

#define MAX_PWM    50
#define RAMP_STEP   5
#define RAMP_DELAY 20

/*
  BTS7960 IS pin: voltage RISES with motor load.
  Always check  val >= threshold  (not <=).

  HOW TO TUNE:
    1. Open Serial Monitor at 115200.
    2. Run actuator freely — note idle ADC reading (usually 0-30).
    3. Let it stall at end of travel — note stall ADC reading.
    4. Set CURRENT_THRESHOLD halfway between those two numbers.
    5. CURRENT_SPIKE_THRESHOLD is for the fast in-jog load check; set a bit higher.
*/
#define CURRENT_THRESHOLD       120  /* end-of-travel stall — tune on bench */
#define CURRENT_SPIKE_THRESHOLD 250  /* hard in-motion load — tune on bench  */

struct PlotPoint {
  float x;
  float y;
  bool  penDown;
};

struct Pt {
  float x;
  float y;
  bool  extrude;
};

const float X_STEPS_PER_MM = 160.0;  // TODO: calibrate
const float Y_STEPS_PER_MM = 160.0;  // TODO: calibrate

const int  Z_PULSE_US    = 800;
const int  Z_LIFT_STEPS  = 1600;
const int  Z_LOWER_STEPS = 1600;
const bool Z_UP_DIR      = false;
const bool Z_DOWN_DIR    = true;

const float PRINT_START_X_MM = 15.0;
const float PRINT_START_Y_MM = 15.0;

typedef enum {
  SIZE_NONE,
  SIZE_SMALL,
  SIZE_LARGE
} CookieSize;

extern CookieSize cookieSize;

#endif
