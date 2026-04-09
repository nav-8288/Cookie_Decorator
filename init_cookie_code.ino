#include <math.h>
#include <Bounce2.h> /*bounce 2 library used for reading peripherals such as buttons or switches*/

/*INCLUDE LOGIC FOR LIMIT SWITCHES */

#define LED_R 44 /* dig pin 44 */
#define LED_G 45 /*dig pin 45*/
#define LED_B 46 /*dig pin 46*/

#define print_SW A9 /* For dig Pin A9*/
#define kill_PB A5 /*For dig pin A5 */
#define refill_clean_PB A0 /*For dig pin A0*/
#define cookie_size_LARGE 2 /*For dig pin D2  */
#define cookie_size_SMALL 3 /* For dig pin D3 */


/*FOR X AXIS*/
#define X_DIR 6 /*For dig pin D6*/ 
#define X_STEP 7 /*For dig pin D7*/

/*FOR Y AXIS*/
#define Y_DIR 8 /*For dig pin D8*/
#define Y_STEP 9 /*For dig pin D9*/

/*FOR Z AXIS*/
#define Z_DIR 10 /*For dig pin D10*/
#define Z_STEP 11 /*For dig pin D11*/

/*FOR EXTRUDER*/
#define EX_DIR 12 /*For dig pin D12*/
#define EX_STEP 13 /*For dig pin D13*/

/*Initialize ENABLE PINS FOR DRIVERS*/
#define X_ENABLE 4 /*For dig pin D4*/
#define Y_ENABLE 5 /*For dig pin D5*/
#define Z_ENABLE 22 /*For dig pin D22*/
#define EX_ENABLE A1 /*For dig pin A1*/


// Generated Arduino point data
struct PlotPoint {
  float x;
  float y;
  bool penDown;
};



struct Pt { float x; float y; bool extrude; };


// --- prototypes to avoid "not declared" issues ---
float cookieScale();
void runLogoPath(const PlotPoint *path, int n, int pulseUs);
void runMinimapPath(const Pt *path, int n, int pulseUs);
void moveXY(long dxSteps, long dySteps, int pulseUs);

void enableDriver(int ENABLE_PIN);
void disableAllDrivers();
void stepOnce(int step_pin, int pulse);
void liftTool();
void lowerTool();

const float X_STEPS_PER_MM = 160.0; // TODO: calibrate
const float Y_STEPS_PER_MM = 160.0; // TODO: calibrate

const int Z_PULSE_US = 800;

/*change these after testing*/
const int Z_LIFT_STEPS = 1600;     // how many microsteps to raise tool
const int Z_LOWER_STEPS = 1600;    // how many microsteps to lower tool

/*adjust these booleans if Z moves opposite of what you want*/
const bool Z_UP_DIR = true;
const bool Z_DOWN_DIR = false;

void setLED(int r, int g, int b);





const int NUM_POINTS = 582;
const PlotPoint logoPoints[NUM_POINTS] = {
  {0.00, 0.00, false},
  {0.00, 1.44, true},
  {0.00, 2.88, true},
  {1.93, 2.88, true},
  {2.31, 2.88, true},
  {2.65, 2.88, true},
  {2.95, 2.88, true},
  {3.21, 2.89, true},
  {3.43, 2.89, true},
  {3.70, 2.89, true},
  {3.92, 2.90, true},
  {4.13, 2.94, true},
  {4.34, 3.03, true},
  {4.51, 3.16, true},
  {4.64, 3.33, true},
  {4.75, 3.55, true},
  {4.81, 3.76, true},
  {4.83, 4.05, true},
  {4.83, 4.32, true},
  {4.84, 4.69, true},
  {4.84, 4.92, true},
  {4.84, 5.18, true},
  {4.84, 5.49, true},
  {4.85, 5.83, true},
  {4.85, 6.21, true},
  {4.85, 6.64, true},
  {4.85, 7.12, true},
  {4.85, 7.65, true},
  {4.85, 8.23, true},
  {4.86, 8.79, true},
  {4.86, 9.30, true},
  {4.86, 9.76, true},
  {4.86, 10.19, true},
  {4.86, 10.57, true},
  {4.87, 10.91, true},
  {4.87, 11.21, true},
  {4.87, 11.49, true},
  {4.87, 11.72, true},
  {4.88, 11.93, true},
  {4.88, 12.27, true},
  {4.89, 12.52, true},
  {4.90, 12.76, true},
  {4.94, 13.03, true},
  {4.98, 13.28, true},
  {5.03, 13.52, true},
  {5.09, 13.75, true},
  {5.15, 13.97, true},
  {5.22, 14.19, true},
  {5.29, 14.40, true},
  {5.37, 14.60, true},
  {5.46, 14.81, true},
  {5.56, 15.01, true},
  {5.70, 15.28, true},
  {5.88, 15.60, true},
  {6.09, 15.91, true},
  {6.32, 16.21, true},
  {6.56, 16.51, true},
  {6.83, 16.79, true},
  {7.11, 17.07, true},
  {7.26, 17.21, true},
  {7.42, 17.34, true},
  {7.58, 17.47, true},
  {7.74, 17.60, true},
  {7.91, 17.72, true},
  {8.08, 17.84, true},
  {8.25, 17.96, true},
  {8.69, 18.24, true},
  {9.16, 18.50, true},
  {9.65, 18.75, true},
  {10.17, 18.99, true},
  {10.71, 19.21, true},
  {11.29, 19.41, true},
  {11.88, 19.61, true},
  {12.51, 19.79, true},
  {13.16, 19.96, true},
  {13.84, 20.11, true},
  {14.54, 20.25, true},
  {15.27, 20.38, true},
  {16.03, 20.49, true},
  {16.81, 20.59, true},
  {17.62, 20.67, true},
  {18.45, 20.74, true},
  {19.31, 20.80, true},
  {20.20, 20.84, true},
  {21.11, 20.87, true},
  {22.04, 20.89, true},
  {22.60, 20.90, true},
  {22.61, 23.46, true},
  {22.61, 23.71, true},
  {22.61, 23.94, true},
  {22.61, 24.17, true},
  {22.61, 24.38, true},
  {22.61, 24.59, true},
  {22.61, 24.96, true},
  {22.61, 25.28, true},
  {22.61, 25.55, true},
  {22.61, 25.78, true},
  {22.60, 26.03, true},
  {22.58, 26.26, true},
  {22.52, 26.47, true},
  {22.42, 26.67, true},
  {22.29, 26.84, true},
  {22.14, 26.98, true},
  {21.96, 27.09, true},
  {21.75, 27.17, true},
  {21.47, 27.21, true},
  {21.26, 27.22, true},
  {20.99, 27.23, true},
  {20.63, 27.23, true},
  {20.42, 27.23, true},
  {20.19, 27.23, true},
  {19.93, 27.23, true},
  {19.64, 27.23, true},
  {17.79, 27.23, true},
  {17.79, 28.67, true},
  {17.79, 30.11, true},
  {36.26, 30.11, true},
  {39.11, 30.11, true},
  {41.65, 30.11, true},
  {43.91, 30.11, true},
  {45.90, 30.11, true},
  {47.64, 30.11, true},
  {49.15, 30.11, true},
  {50.44, 30.11, true},
  {51.53, 30.11, true},
  {52.45, 30.10, true},
  {53.20, 30.10, true},
  {53.80, 30.09, true},
  {54.28, 30.09, true},
  {54.64, 30.08, true},
  {54.92, 30.07, true},
  {55.26, 30.04, true},
  {55.52, 29.99, true},
  {55.81, 29.93, true},
  {56.01, 29.87, true},
  {56.23, 29.80, true},
  {56.44, 29.72, true},
  {56.66, 29.63, true},
  {56.87, 29.54, true},
  {57.08, 29.44, true},
  {57.27, 29.34, true},
  {57.46, 29.23, true},
  {57.73, 29.05, true},
  {57.94, 28.90, true},
  {58.15, 28.73, true},
  {58.34, 28.56, true},
  {58.53, 28.38, true},
  {58.71, 28.19, true},
  {58.88, 27.99, true},
  {59.03, 27.79, true},
  {59.18, 27.58, true},
  {59.31, 27.37, true},
  {59.46, 27.10, true},
  {59.61, 26.77, true},
  {59.73, 26.43, true},
  {59.84, 26.08, true},
  {59.91, 25.71, true},
  {59.97, 25.34, true},
  {60.00, 24.97, true},
  {60.00, 24.59, true},
  {59.97, 24.21, true},
  {59.92, 23.84, true},
  {59.81, 23.33, true},
  {59.72, 23.02, true},
  {59.61, 22.72, true},
  {59.48, 22.42, true},
  {59.33, 22.14, true},
  {59.16, 21.86, true},
  {58.98, 21.60, true},
  {58.79, 21.35, true},
  {58.58, 21.10, true},
  {58.36, 20.88, true},
  {58.13, 20.66, true},
  {57.88, 20.46, true},
  {57.62, 20.28, true},
  {57.35, 20.11, true},
  {57.07, 19.95, true},
  {56.78, 19.82, true},
  {56.49, 19.70, true},
  {56.18, 19.59, true},
  {55.87, 19.51, true},
  {55.55, 19.45, true},
  {55.34, 19.40, true},
  {55.46, 19.23, true},
  {55.74, 19.16, true},
  {55.98, 19.10, true},
  {56.22, 19.03, true},
  {56.45, 18.95, true},
  {56.67, 18.86, true},
  {56.89, 18.75, true},
  {57.11, 18.64, true},
  {57.32, 18.52, true},
  {57.52, 18.38, true},
  {57.71, 18.24, true},
  {57.89, 18.09, true},
  {58.08, 17.92, true},
  {58.22, 17.77, true},
  {58.37, 17.62, true},
  {58.50, 17.47, true},
  {58.65, 17.29, true},
  {58.81, 17.07, true},
  {58.93, 16.88, true},
  {59.04, 16.69, true},
  {59.15, 16.49, true},
  {59.24, 16.28, true},
  {59.33, 16.08, true},
  {59.41, 15.86, true},
  {59.48, 15.65, true},
  {59.54, 15.42, true},
  {59.59, 15.20, true},
  {59.62, 14.98, true},
  {59.64, 14.75, true},
  {59.65, 14.54, true},
  {59.65, 14.33, true},
  {59.65, 14.12, true},
  {59.64, 13.89, true},
  {59.63, 13.69, true},
  {59.61, 13.41, true},
  {59.57, 13.16, true},
  {59.50, 12.91, true},
  {59.42, 12.65, true},
  {59.31, 12.36, true},
  {59.22, 12.14, true},
  {59.11, 11.93, true},
  {58.99, 11.72, true},
  {58.86, 11.51, true},
  {58.72, 11.32, true},
  {58.56, 11.12, true},
  {58.40, 10.94, true},
  {58.22, 10.76, true},
  {58.04, 10.59, true},
  {57.85, 10.43, true},
  {57.65, 10.28, true},
  {57.44, 10.14, true},
  {57.22, 10.00, true},
  {56.99, 9.87, true},
  {56.76, 9.76, true},
  {56.53, 9.65, true},
  {56.28, 9.56, true},
  {56.03, 9.47, true},
  {55.78, 9.40, true},
  {55.52, 9.34, true},
  {55.26, 9.28, true},
  {54.96, 9.25, true},
  {54.66, 9.23, true},
  {54.23, 9.22, true},
  {53.95, 9.21, true},
  {53.62, 9.21, true},
  {53.24, 9.21, true},
  {52.80, 9.20, true},
  {52.29, 9.20, true},
  {51.71, 9.20, true},
  {51.05, 9.20, true},
  {50.31, 9.20, true},
  {49.48, 9.20, true},
  {48.56, 9.20, true},
  {42.54, 9.20, true},
  {42.55, 6.49, true},
  {42.55, 3.77, true},
  {42.63, 3.55, true},
  {42.73, 3.33, true},
  {42.86, 3.17, true},
  {43.03, 3.04, true},
  {43.35, 2.89, true},
  {45.36, 2.89, true},
  {47.37, 2.88, true},
  {47.37, 1.44, true},
  {47.37, 0.00, true},
  {39.70, 0.00, true},
  {32.03, 0.00, true},
  {32.03, 1.44, true},
  {32.03, 2.88, true},
  {33.96, 2.88, true},
  {34.34, 2.88, true},
  {34.68, 2.88, true},
  {34.98, 2.88, true},
  {35.24, 2.89, true},
  {35.46, 2.89, true},
  {35.73, 2.89, true},
  {35.96, 2.90, true},
  {36.16, 2.94, true},
  {36.37, 3.03, true},
  {36.54, 3.16, true},
  {36.68, 3.33, true},
  {36.78, 3.55, true},
  {36.84, 3.75, true},
  {36.86, 3.95, true},
  {36.86, 4.23, true},
  {36.87, 4.50, true},
  {36.87, 4.85, true},
  {36.87, 5.06, true},
  {36.87, 5.30, true},
  {36.88, 5.56, true},
  {36.88, 5.85, true},
  {36.88, 6.16, true},
  {36.88, 6.51, true},
  {36.89, 9.20, true},
  {27.35, 9.20, true},
  {17.82, 9.20, true},
  {17.82, 10.65, true},
  {17.82, 12.11, true},
  {19.66, 12.11, true},
  {19.95, 12.11, true},
  {20.21, 12.11, true},
  {20.44, 12.11, true},
  {20.65, 12.11, true},
  {21.00, 12.11, true},
  {21.27, 12.12, true},
  {21.47, 12.13, true},
  {21.69, 12.15, true},
  {21.91, 12.22, true},
  {22.13, 12.35, true},
  {22.28, 12.49, true},
  {22.41, 12.66, true},
  {22.51, 12.85, true},
  {22.58, 13.07, true},
  {22.60, 13.28, true},
  {22.60, 13.53, true},
  {22.61, 13.74, true},
  {22.61, 14.00, true},
  {22.61, 14.31, true},
  {22.61, 14.65, true},
  {22.62, 15.03, true},
  {22.62, 15.23, true},
  {22.62, 15.44, true},
  {22.62, 15.66, true},
  {22.62, 18.03, true},
  {22.26, 18.03, true},
  {22.04, 18.03, true},
  {21.83, 18.02, true},
  {21.61, 18.01, true},
  {21.38, 18.00, true},
  {21.16, 17.99, true},
  {20.94, 17.97, true},
  {20.72, 17.95, true},
  {20.50, 17.93, true},
  {20.28, 17.90, true},
  {20.07, 17.87, true},
  {19.85, 17.84, true},
  {19.64, 17.80, true},
  {19.43, 17.77, true},
  {19.23, 17.73, true},
  {19.03, 17.68, true},
  {18.66, 17.59, true},
  {18.30, 17.49, true},
  {17.84, 17.33, true},
  {17.55, 17.22, true},
  {17.28, 17.10, true},
  {17.01, 16.96, true},
  {16.76, 16.82, true},
  {16.53, 16.68, true},
  {16.30, 16.52, true},
  {16.09, 16.35, true},
  {15.89, 16.18, true},
  {15.71, 15.99, true},
  {15.53, 15.80, true},
  {15.37, 15.60, true},
  {15.22, 15.39, true},
  {15.08, 15.17, true},
  {14.96, 14.94, true},
  {14.84, 14.71, true},
  {14.74, 14.46, true},
  {14.65, 14.21, true},
  {14.58, 13.94, true},
  {14.51, 13.67, true},
  {14.45, 13.39, true},
  {14.41, 13.13, true},
  {14.39, 12.90, true},
  {14.38, 12.59, true},
  {14.38, 12.39, true},
  {14.37, 12.15, true},
  {14.37, 11.87, true},
  {14.37, 11.55, true},
  {14.36, 11.17, true},
  {14.36, 10.74, true},
  {14.36, 10.25, true},
  {14.36, 9.70, true},
  {14.36, 9.08, true},
  {14.36, 8.38, true},
  {14.36, 7.69, true},
  {14.35, 7.07, true},
  {14.35, 6.51, true},
  {14.35, 6.02, true},
  {14.35, 5.59, true},
  {14.35, 5.21, true},
  {14.35, 4.89, true},
  {14.36, 4.61, true},
  {14.36, 4.37, true},
  {14.37, 4.01, true},
  {14.38, 3.77, true},
  {14.41, 3.57, true},
  {14.50, 3.37, true},
  {14.63, 3.20, true},
  {14.81, 3.06, true},
  {15.01, 2.96, true},
  {15.21, 2.91, true},
  {15.44, 2.89, true},
  {15.69, 2.89, true},
  {15.90, 2.89, true},
  {16.14, 2.88, true},
  {16.42, 2.88, true},
  {16.73, 2.88, true},
  {17.08, 2.88, true},
  {19.18, 2.88, true},
  {19.18, 1.44, true},
  {19.18, 0.00, true},
  {9.59, 0.00, true},
  {0.00, 0.00, true},
  {44.77, 12.10, false},
  {45.47, 12.10, true},
  {46.10, 12.10, true},
  {46.65, 12.10, true},
  {47.15, 12.10, true},
  {47.58, 12.09, true},
  {47.96, 12.09, true},
  {48.28, 12.10, true},
  {48.56, 12.10, true},
  {48.80, 12.10, true},
  {49.00, 12.10, true},
  {49.31, 12.12, true},
  {49.51, 12.13, true},
  {49.72, 12.17, true},
  {49.98, 12.25, true},
  {50.17, 12.33, true},
  {50.35, 12.41, true},
  {50.53, 12.51, true},
  {50.70, 12.62, true},
  {50.87, 12.74, true},
  {51.03, 12.88, true},
  {51.24, 13.09, true},
  {51.38, 13.27, true},
  {51.51, 13.45, true},
  {51.62, 13.63, true},
  {51.72, 13.83, true},
  {51.80, 14.03, true},
  {51.87, 14.24, true},
  {51.92, 14.45, true},
  {51.96, 14.67, true},
  {51.97, 14.90, true},
  {51.98, 15.13, true},
  {51.97, 15.34, true},
  {51.94, 15.59, true},
  {51.89, 15.78, true},
  {51.79, 16.09, true},
  {51.67, 16.37, true},
  {51.52, 16.64, true},
  {51.34, 16.89, true},
  {51.14, 17.11, true},
  {50.92, 17.32, true},
  {50.67, 17.50, true},
  {50.40, 17.65, true},
  {50.11, 17.79, true},
  {49.80, 17.90, true},
  {49.57, 17.95, true},
  {49.32, 17.97, true},
  {49.08, 17.98, true},
  {48.74, 17.98, true},
  {48.53, 17.98, true},
  {48.28, 17.98, true},
  {48.00, 17.99, true},
  {47.67, 17.99, true},
  {47.31, 17.99, true},
  {46.91, 17.99, true},
  {46.45, 17.99, true},
  {45.95, 17.99, true},
  {45.40, 17.99, true},
  {44.79, 18.00, true},
  {44.23, 18.00, true},
  {43.72, 18.00, true},
  {43.24, 18.00, true},
  {42.81, 18.00, true},
  {42.42, 18.00, true},
  {42.07, 18.00, true},
  {41.75, 18.00, true},
  {41.47, 18.00, true},
  {41.22, 18.00, true},
  {41.01, 18.00, true},
  {40.66, 17.99, true},
  {40.41, 17.99, true},
  {40.21, 17.98, true},
  {40.35, 17.81, true},
  {40.65, 17.55, true},
  {40.81, 17.39, true},
  {40.97, 17.23, true},
  {41.11, 17.06, true},
  {41.26, 16.89, true},
  {41.39, 16.71, true},
  {41.52, 16.52, true},
  {41.64, 16.34, true},
  {41.76, 16.14, true},
  {41.87, 15.95, true},
  {41.97, 15.74, true},
  {42.06, 15.54, true},
  {42.14, 15.33, true},
  {42.22, 15.12, true},
  {42.29, 14.91, true},
  {42.35, 14.69, true},
  {42.41, 14.48, true},
  {42.45, 14.26, true},
  {42.49, 14.04, true},
  {42.51, 13.82, true},
  {42.52, 13.61, true},
  {42.53, 13.36, true},
  {42.54, 13.13, true},
  {42.54, 12.89, true},
  {42.54, 12.11, true},
  {44.77, 12.10, true},
  {32.03, 12.11, false},
  {34.46, 12.11, true},
  {36.89, 12.11, true},
  {36.89, 12.92, true},
  {36.88, 13.12, true},
  {36.88, 13.32, true},
  {36.88, 13.58, true},
  {36.87, 13.80, true},
  {36.86, 14.01, true},
  {36.84, 14.23, true},
  {36.80, 14.49, true},
  {36.74, 14.76, true},
  {36.67, 15.01, true},
  {36.58, 15.27, true},
  {36.48, 15.51, true},
  {36.36, 15.76, true},
  {36.24, 15.97, true},
  {36.12, 16.17, true},
  {35.98, 16.36, true},
  {35.82, 16.55, true},
  {35.65, 16.74, true},
  {35.44, 16.92, true},
  {35.23, 17.10, true},
  {35.00, 17.26, true},
  {34.76, 17.40, true},
  {34.50, 17.53, true},
  {34.23, 17.65, true},
  {33.95, 17.75, true},
  {33.65, 17.84, true},
  {33.35, 17.91, true},
  {33.03, 17.96, true},
  {32.82, 17.99, true},
  {32.61, 18.01, true},
  {32.41, 18.02, true},
  {32.03, 18.04, true},
  {32.03, 15.07, true},
  {32.03, 12.11, true},
  {32.03, 20.88, false},
  {40.63, 20.89, true},
  {49.22, 20.90, true},
  {49.46, 20.95, true},
  {49.82, 21.06, true},
  {50.16, 21.19, true},
  {50.47, 21.36, true},
  {50.76, 21.56, true},
  {51.02, 21.78, true},
  {51.26, 22.03, true},
  {51.46, 22.31, true},
  {51.64, 22.61, true},
  {51.78, 22.93, true},
  {51.89, 23.28, true},
  {51.94, 23.53, true},
  {51.97, 23.74, true},
  {51.98, 23.97, true},
  {51.98, 24.19, true},
  {51.97, 24.41, true},
  {51.94, 24.61, true},
  {51.89, 24.83, true},
  {51.79, 25.14, true},
  {51.67, 25.43, true},
  {51.51, 25.71, true},
  {51.33, 25.98, true},
  {51.13, 26.22, true},
  {50.90, 26.44, true},
  {50.65, 26.64, true},
  {50.38, 26.81, true},
  {50.09, 26.95, true},
  {49.90, 27.03, true},
  {49.70, 27.09, true},
  {49.50, 27.15, true},
  {49.22, 27.22, true},
  {40.63, 27.22, true},
  {32.03, 27.23, true},
  {32.03, 24.06, true},
  {32.03, 20.88, true},
};


/*List of modes (states) machine can be in */
typedef enum{
  IDLE, /*Idle state, nothing happens*/
  PRINTING, /*If printing, LED should be yellow and buttons do diff things*/
  COMPLETED, /*Print has been completed*/
  NEEDS_REFILL, /*Show Red and block printing from happening */
  REFILL_CLEAN, /**/
  KILLED /*If KILLED, dont allow motors to move*/
} SystemState;

//set the state to IDLE
SystemState state = IDLE;

volatile bool killRequested = false;

unsigned long sizeFlashUntilMs = 0;
bool sizeFlashActive = false;



typedef enum { SIZE_SMALL, SIZE_LARGE } CookieSize;
CookieSize cookieSize = SIZE_SMALL;

/*Bounce objects store state of pin; Last reading (High/Low), last time changed, */
Bounce bPrint = Bounce(); /*For print button*/
Bounce bKill = Bounce(); /*For kill switch */
Bounce bRefill = Bounce(); /*For refill/clean switch */
Bounce bSize_SMALL = Bounce(); /*for cookie size sw (SMALL)*/
Bounce bSize_LARGE = Bounce(); /*for cookie size sw (LARGE)*/



void runLogoPath(const PlotPoint *path, int n, int pulseUs) {
  float s = cookieScale();

  /*design area on cookie in mm*/
  const float W = 70.0 * s;
  const float H = 45.0 * s;

  /*Bounds of imported UB logo coordinates*/
  const float logoMinX = 0.0;
  const float logoMaxX = 60.0;
  const float logoMinY = 0.0;
  const float logoMaxY = 30.11;

  /*Scale imported logo so it fits the cookie area*/
  float scaleX = W / (logoMaxX - logoMinX);
  float scaleY = H / (logoMaxY - logoMinY);

  /*Use uniform scaling so the logo shape stays the same*/
  float scale = scaleX;
  if (scaleY < scaleX) {
    scale = scaleY;
  }

  float curXmm = (path[0].x - logoMinX) * scale;
  float curYmm = (path[0].y - logoMinY) * scale;

  bool toolIsDown = false;

  /*Make sure tool starts lifted before any travel*/
  liftTool();

  for (int i = 1; i < n; i++) {
    float tgtXmm = (path[i].x - logoMinX) * scale;
    float tgtYmm = (path[i].y - logoMinY) * scale;

    long dxSteps = lround((tgtXmm - curXmm) * X_STEPS_PER_MM);
    long dySteps = lround((tgtYmm - curYmm) * Y_STEPS_PER_MM);

    /*If this segment is meant to draw, lower tool first*/
    if (path[i].penDown && !toolIsDown) {
      lowerTool();
      toolIsDown = true;
      if (state == KILLED) return;
    }

    /*If this segment is meant to travel, lift tool first*/
    if (!path[i].penDown && toolIsDown) {
      liftTool();
      toolIsDown = false;
      if (state == KILLED) return;
    }

    moveXY(dxSteps, dySteps, pulseUs);
    if (state == KILLED) return;

    curXmm = tgtXmm;
    curYmm = tgtYmm;
  }

  /*Lift tool at the end so it does not drag after finishing*/
  if (toolIsDown) {
    liftTool();
  }
}

void runMinimapPath(const Pt *path, int n, int pulseUs) {
  float s = cookieScale();

  // large cookie design area (mm) with margins
 const float W = 70.0 * s;
const float H = 45.0 * s;

  float curXmm = path[0].x * W;
  float curYmm = path[0].y * H;

  for (int i = 1; i < n; i++) {
    float tgtXmm = path[i].x * W;
    float tgtYmm = path[i].y * H;

    long dxSteps = lround((tgtXmm - curXmm) * X_STEPS_PER_MM);
    long dySteps = lround((tgtYmm - curYmm) * Y_STEPS_PER_MM);

    moveXY(dxSteps, dySteps, pulseUs);
    if (state == KILLED) return;

    // (later) if path[i].extrude == true -> step extruder while moving

    curXmm = tgtXmm;
    curYmm = tgtYmm;
  }
}


// Move X/Y together to draw a straight line
void moveXY(long dxSteps, long dySteps, int pulseUs) {
  enableDriver(X_ENABLE);
  enableDriver(Y_ENABLE);

  digitalWrite(X_DIR, (dxSteps >= 0) ? HIGH : LOW);
  digitalWrite(Y_DIR, (dySteps >= 0) ? HIGH : LOW);

  long ax = labs(dxSteps);
  long ay = labs(dySteps);

  long err = ax - ay;
  long cx = 0, cy = 0;

  while (cx < ax || cy < ay) {
    if (killRequested) {
      Serial.println("KILL detected during moveXY!");
      state = KILLED;
      disableAllDrivers();
      setLED(255, 0, 0); // Red
      return;
    }

    long e2 = 2 * err;

    if (e2 > -ay && cx < ax) {
      err -= ay;
      stepOnce(X_STEP, pulseUs);
      cx++;
    }
    if (e2 < ax && cy < ay) {
      err += ax;
      stepOnce(Y_STEP, pulseUs);
      cy++;
    }
  }

  disableAllDrivers();
}

float cookieScale() {
  if (cookieSize == SIZE_LARGE) return 1.0;
  // scale small relative to large (use width ratio as a simple uniform scale)
  return 2.375 / 4.25;  // ~0.559
}




void showCookieSizeLED() {
  if (cookieSize == SIZE_SMALL) {
    setLED(255, 80, 0);        // Orange = SMALL
  } else {
    setLED(255, 255, 255);     // White = LARGE
  }
}

void flashCookieSizeLED(unsigned long durationMs) {
  showCookieSizeLED();                 // orange/white based on cookieSize
  sizeFlashActive = true;
  sizeFlashUntilMs = millis() + durationMs;
}

void showStateLED() {
  if (state == KILLED) {
    setLED(255, 0, 0);         // Red
  } else if (state == REFILL_CLEAN) {
    setLED(255, 0, 255);       // Purple
  } else if (state == PRINTING) {
    setLED(255, 255, 0);       // Yellow
  } else if (state == COMPLETED) {
    setLED(0, 255, 0);         // Green
  } else {
    setLED(0, 0, 255);         // Blue (IDLE)
  }
}

/*Function used to check which cookie-size switch is currently selected at POWER ON*/
/*Sets the cookiesize var to match and prints it/updates the LED*/
void initCookieSizeFromSwitches() {
  // switches are INPUT_PULLUP, so "selected" position usually reads LOW
  bool largeActive = (digitalRead(cookie_size_LARGE) == LOW);
  bool smallActive = (digitalRead(cookie_size_SMALL) == LOW);

  /*LargeActive set to true if Large switch pin is LOW*/
  /* SmallActive set to true if Small switch pin is LOW*/

  if (largeActive && !smallActive) {
    cookieSize = SIZE_LARGE;
  } else if (smallActive && !largeActive) {
    cookieSize = SIZE_SMALL;
  } 
  // If both are HIGH (or both LOW), leave default SIZE_SMALL (safe fallback)

  Serial.print("Boot Cookie Size = ");
  if (cookieSize == SIZE_LARGE) {
  Serial.println("LARGE");
} else {
  Serial.println("SMALL");
}

flashCookieSizeLED(400); /*set the LED to match the cookie size  */
}

void setLED(int r, int g, int b){
  analogWrite(LED_R, r);
  analogWrite(LED_G, g);
  analogWrite(LED_B, b);
}

void initDriver(int enPin, int dirPin, int stepPin) {
  pinMode(enPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);

  digitalWrite(enPin, HIGH);            // disable outputs first (active LOW)
  delay(2);                             // settle time like your working test

  digitalWrite(stepPin, LOW);           // clean STEP state
  digitalWrite(dirPin, LOW);            // clean DIR state
}

void disableAllDrivers() {
  /*Function for kill_switch setting all enable pins on driver to HIGH (ENABLE IS ACTIVE LOW)*/
  digitalWrite(X_ENABLE, HIGH);
  digitalWrite(Y_ENABLE, HIGH);
  digitalWrite(Z_ENABLE, HIGH);
  digitalWrite(EX_ENABLE, HIGH);
}

/*function used to set enable pins on the A4988 DRIVER*/
void enableDriver(int ENABLE_PIN){
  digitalWrite(ENABLE_PIN, LOW);
}

/*function used to generate one clean STEP pulse on druvers step pin. Each pulse = one microstep*/
void stepOnce(int step_pin, int pulse){
  digitalWrite(step_pin, HIGH);
  delayMicroseconds(pulse);
  digitalWrite(step_pin, LOW);
  delayMicroseconds(pulse);
  
}

void jogAxis(int enPin, int dirPin, int stepPin, bool dir, int steps, int pulseUs) {
  enableDriver(enPin);
  digitalWrite(dirPin, dir ? HIGH : LOW);

  for (int i = 0; i < steps; i++) {
    // hard kill check (active-low)
    if (killRequested) {
  Serial.println("KILL detected during jog!");
  state = KILLED;
  disableAllDrivers();
  setLED(255, 0, 0); // SET TO RED
  return;
}
    stepOnce(stepPin, pulseUs);
  }

  // done stepping this axis
disableAllDrivers();}

void liftTool() {
  jogAxis(Z_ENABLE, Z_DIR, Z_STEP, Z_UP_DIR, Z_LIFT_STEPS, Z_PULSE_US);
}

void lowerTool() {
  jogAxis(Z_ENABLE, Z_DIR, Z_STEP, Z_DOWN_DIR, Z_LOWER_STEPS, Z_PULSE_US);
}


void setup() {
  Serial.begin(115200); /*Initialize serial communication between arduino and monitor*/
  /*115200 describes baud rate (bits per second) */

  



  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  setLED(0,0,255); /*Setting blue LED ON - IDLE MODE*/

  pinMode(print_SW, INPUT_PULLUP); /*Initialize print sw*/
  pinMode(kill_PB, INPUT_PULLUP); /*Initialize kill pb*/
  pinMode(refill_clean_PB, INPUT_PULLUP); /*Initialize refill/clean push button*/
  pinMode(cookie_size_SMALL, INPUT_PULLUP); /*Initialize cookie size switch SMALL */
  pinMode(cookie_size_LARGE, INPUT_PULLUP); /*Initialize cookie size switch SMALL */

  initCookieSizeFromSwitches();
  setLED(0,0, 255); /*Make sure LED still outputs BLUE for IDLE mode*/


  /*Initialize DIR and STEP FOR ALL 4 DRIVERS*/

  initDriver(X_ENABLE, X_DIR, X_STEP);
  initDriver(Y_ENABLE, Y_DIR, Y_STEP);
  initDriver(Z_ENABLE, Z_DIR, Z_STEP);
  initDriver(EX_ENABLE, EX_DIR, EX_STEP);




   /*Initialize DIR pin on a4988 driver ; tells driver which dir to step (forw/back) */
   /*Initialize STEP pin on a4988 driver ; step pulses to make motor move */
  /*step and dir are control signals sent to a4988 driver*/

  /*Initialize pins for the ENABLE pins on A4988 DRIVER*/
 
  
  
 /*set step to low so its starts with clean state; prevents random step from unknown state*/

  /*SET ENABLE PINS TO HIGH TO DISABLE ; ENABLE PINS ARE ACTIVE LOW*/


  /*SET DIR PINS TO lOW SO IT STARTS WITH A CLEAN STATE, PREVENTS FROM GOING IN RANDOM DIRECION FROM UNKNOWN STATE*/


/*Each bounce obj should know which Arduino pin to read*/
  bPrint.attach(print_SW); /*tracks the state of pin A9*/
  bKill.attach(kill_PB); /*tracks the state of pin A5*/
  bRefill.attach(refill_clean_PB); /*tracks the state of pin A0*/
  bSize_LARGE.attach(cookie_size_LARGE); /*tracks the state of pin D2*/
  bSize_SMALL.attach(cookie_size_SMALL); /*tracks the state of pin D3*/


/*Sets debounce window, buttons and sw's "bounce electrically" meaning single press can rapidly flicker
Adding .interval() allows signal to stay stable for 15ms before accepting button/SW press  */
  bPrint.interval(15);
  bKill.interval(15);
  bRefill.interval(15);
  bSize_SMALL.interval(15);
  bSize_LARGE.interval(15);


  /*IF THE BUTTON HAS BEEN HELD FOR 15ms, accept the new state*/

  
  // put your setup code here, to run once:

}

void loop() {
/*update must be called repeatedly so the pin is read in present time and decide if real change happened
functions fell(), rose() and changed() WILL NOT work without update()*/
  bPrint.update();
  bKill.update();
  bRefill.update();
  bSize_LARGE.update();
  bSize_SMALL.update();

  if (sizeFlashActive && (long)(millis() - sizeFlashUntilMs) >= 0) {
  sizeFlashActive = false;
  showStateLED();   // return to current state color
}


  /*When the button is not pressed, BUTTON = 1; When the button IS PRESSED, BUTTON = 0 (ACTIVE LOW)*/

/*If the button has been pressed*/
  if(bPrint.fell()){
    Serial.println("PRINT Button pressed");
    }


if (bPrint.rose()) {
  state = PRINTING;
  showStateLED();
  Serial.println("Printing UB logo path...");

  runLogoPath(logoPoints, NUM_POINTS, 800);

  if (state != KILLED) {
    state = COMPLETED;
    showStateLED();
    delay(1000);
    state = IDLE;
    showStateLED();
  }
}
  

  

    /*If the button has been pressed*/
  if(bKill.fell()){
    Serial.println("Kill switch Button pressed");
    killRequested = true;
    state = KILLED;
    disableAllDrivers(); /*disable all the */
    setLED(255, 0, 0); /*set the LED to RED*/
    }

/*If the button has been released*/
 if(bKill.rose()){
    Serial.println("Kill switch Button released");
    killRequested = false;
    state = IDLE;
    setLED(0, 0, 255); /*Set the LED to blue for IDLE state */
    }

   


  if (bRefill.fell()) {
    if (state != REFILL_CLEAN) {
     state = REFILL_CLEAN;      // enter refill/clean mode and stay there
     setLED(255, 0, 255);         // SET LED TO PURPLE
     Serial.println("REFILL/CLEAN mode ON");
     Serial.println("Releasing extruder piston...");
     jogAxis(EX_ENABLE, EX_DIR, EX_STEP, false, 200 * 16, 800); // reverse a bit
  } else {
    state = IDLE;              // exit refill/clean mode
    setLED(0, 0, 255);         // Blue
    Serial.println("REFILL/CLEAN mode OFF");
  }
}




if (bSize_LARGE.changed() || bSize_SMALL.changed()) {
  // re-evaluate based on actual pin levels
  bool largeActive = (digitalRead(cookie_size_LARGE) == LOW);
  bool smallActive = (digitalRead(cookie_size_SMALL) == LOW);

  if (largeActive && !smallActive) {
    cookieSize = SIZE_LARGE;
    Serial.println("Cookie Size set to LARGE");
  } else if (smallActive && !largeActive) {
    cookieSize = SIZE_SMALL;
    Serial.println("Cookie Size set to SMALL");
  }

  flashCookieSizeLED(400); /*set the LED to match the cookie size  */

}



}
