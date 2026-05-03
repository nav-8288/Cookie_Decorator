#ifndef COOKIE_CONFIG_H
#define COOKIE_CONFIG_H

#include <math.h>

/*INCLUDE LOGIC FOR LIMIT SWITCHES */

//pins 2, 3, 18, 19, 20 and 21 can only be used for interupts 

#define LED_R 44 /* dig pin 44 */
#define LED_G 45 /*dig pin 45*/
#define LED_B 47 /*dig pin 46*/

#define print_SW A9 /* For dig Pin A9*/
#define kill_PB 2 /*For dig pin A1 */
#define refill_clean_PB A0 /*For dig pin A0*/
#define cookie_size_LARGE 30 /*For dig pin D2  */
#define cookie_size_SMALL 32 /* For dig pin D3 */

/*FOR X AXIS*/
#define X_DIR 6 /*For dig pin D6*/
#define X_STEP 7 /*For dig pin D7*/

/*FOR Y AXIS*/
#define Y_DIR 8 /*For dig pin D8*/
#define Y_STEP 9 /*For dig pin D9*/

/*FOR Z AXIS*/
#define Z_DIR 10 /*For dig pin D10*/
#define Z_STEP 11 /*For dig pin D11*/



/*Initialize ENABLE PINS FOR DRIVERS*/
#define X_ENABLE 4 /*For dig pin D4*/
#define Y_ENABLE 5 /*For dig pin D5*/
#define Z_ENABLE 22 /*For dig pin D22*/

#define Y_LIMIT_SW 24 /*Y back/home limit switch ; Black(COM)->GND, Gray(NC)->A3*/
#define X_LIMIT_SW 25 /*X home limit switch ; Black(COM)->GND, Gray(NC)->25*/
#define Z_LIMIT_SW 26 /*Z bottom/home limit switch ; Black(COM)->GND, Gray(NC)->26*/

#define ACT_R_PWM 12    /* extend side PWM */
#define ACT_L_PWM 13    /* retract side PWM */

#define ACT_L_IS A6   /* current sense for one direction */
#define ACT_R_IS A7   /* current sense for other direction */

#define MAX_PWM 50
#define RAMP_STEP 5
#define RAMP_DELAY 20
#define CURRENT_THRESHOLD 0
#define CURRENT_SPIKE_THRESHOLD 250

// Generated Arduino point data
struct PlotPoint {
  float x;
  float y;
  bool penDown;
};

struct Pt {
  float x;
  float y;
  bool extrude;
};

const float X_STEPS_PER_MM = 160.0; // TODO: calibrate
const float Y_STEPS_PER_MM = 160.0; // TODO: calibrate

const int Z_PULSE_US = 800;

/*change these after testing*/
const int Z_LIFT_STEPS = 1600;     // how many microsteps to raise tool
const int Z_LOWER_STEPS = 1600;    // how many microsteps to lower tool

/*adjust these booleans if Z moves opposite of what you want*/
const bool Z_UP_DIR = false;
const bool Z_DOWN_DIR = true;

/*OFFSET FOR THE PRINT TO START AWAY FROM HOMED CORNER */
const float PRINT_START_X_MM = 15.0; /*start 15mm away from X home */
const float PRINT_START_Y_MM = 15.0; /*start 15mm away from Y home */



typedef enum {
  SIZE_NONE,
  SIZE_SMALL,
  SIZE_LARGE
} CookieSize;

extern CookieSize cookieSize;

#endif
