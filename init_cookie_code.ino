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
#define EX_ENABLE 23 /*For dig pin D23*/

struct Pt { float x; float y; bool extrude; };


// --- prototypes to avoid "not declared" issues ---
float cookieScale();
void runMinimapPath(const Pt *path, int n, int pulseUs);
void moveXY(long dxSteps, long dySteps, int pulseUs);

void enableDriver(int ENABLE_PIN);
void disableAllDrivers();
void stepOnce(int step_pin, int pulse);

const float X_STEPS_PER_MM = 80.0; // TODO: calibrate
const float Y_STEPS_PER_MM = 80.0; // TODO: calibrate

void setLED(int r, int g, int b);





// normalized 0..1 points (quick placeholder, not the full UB yet)
const Pt ub_path[] = {
  // Move to start (extrude = false = “travel move”)
  {0.15, 0.20, false},

  // Trace “U” (extrude on)
  {0.15, 0.80, true},
  {0.25, 0.80, true},
  {0.25, 0.35, true},
  {0.45, 0.35, true},
  {0.45, 0.80, true},
  {0.55, 0.80, true},
  {0.55, 0.20, true},
  {0.15, 0.20, true},

  // Pen up / travel to “B”
  {0.65, 0.20, false},

  // Trace “B” rough outline
  {0.65, 0.80, true},
  {0.85, 0.80, true},
  {0.90, 0.70, true},
  {0.85, 0.60, true},
  {0.90, 0.50, true},
  {0.85, 0.40, true},
  {0.65, 0.40, true},
  {0.65, 0.20, true},
};
const int UB_N = sizeof(ub_path)/sizeof(ub_path[0]);


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



void runMinimapPath(const Pt *path, int n, int pulseUs) {
  float s = cookieScale();

  // large cookie design area (mm) with margins
 const float W = 30.0 * s;
const float H = 20.0 * s;

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

void initDriver(int enPin, int dirPin, int stepPin, int sleepResetPin) {
  pinMode(enPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(sleepResetPin, OUTPUT);

  digitalWrite(enPin, HIGH);            // disable outputs first (active LOW)
  digitalWrite(sleepResetPin, HIGH);    // wake + not in reset

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

  initDriver(X_ENABLE, X_DIR, X_STEP, X_SLEEP_RESET);
  initDriver(Y_ENABLE, Y_DIR, Y_STEP, Y_SLEEP_RESET);
  initDriver(Z_ENABLE, Z_DIR, Z_STEP, Z_SLEEP_RESET);
  initDriver(EX_ENABLE, EX_DIR, EX_STEP, EX_SLEEP_RESET);




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
  Serial.println("Printing minimap path...");

  runMinimapPath(ub_path, UB_N, 800);

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
