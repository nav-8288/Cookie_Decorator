#include <Bounce2.h> /*bounce 2 library used for reading peripherals such as buttons or switches*/
#include "cookie_config.h"
#include "logo_data.h"

// --- prototypes to avoid "not declared" issues ---
float cookieScale();
void runLogoPath(const PlotPoint *path, int n, int pulseUs);
void runMinimapPath(const Pt *path, int n, int pulseUs);
void moveXY(long dxSteps, long dySteps, int pulseUs);
void setLED(int r, int g, int b);

void enableDriver(int ENABLE_PIN);
void disableAllDrivers();
void stepOnce(int step_pin, int pulse);
void liftTool();
void lowerTool();
bool isYLimitPressed();
void homeYAxis();
bool isXLimitPressed();
void homeXAxis();
bool isZLimitPressed();
void homeZAxis();
void moveToPrintStart();
void returnToHome();
void homeXYAxes();
void triggerKill();
bool checkKill();
void startExtruderPrimeMode();
void stopExtruderPrimeMode();
void serviceExtruderPrimeMode();
void startPrintJob();

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

bool extruderPrimeMode = false;
unsigned long lastExtruderPrimeStepUs = 0;

typedef enum {
   SIZE_NONE, SIZE_SMALL, SIZE_LARGE 
   } CookieSize;

CookieSize cookieSize = SIZE_NONE;

/*Bounce objects store state of pin; Last reading (High/Low), last time changed, */
Bounce bPrint = Bounce(); /*For print button*/
Bounce bKill = Bounce(); /*For kill switch */
Bounce bRefill = Bounce(); /*For refill/clean switch */
Bounce bSize_SMALL = Bounce(); /*for cookie size sw (SMALL)*/
Bounce bSize_LARGE = Bounce(); /*for cookie size sw (LARGE)*/

void triggerKill() {
  killRequested = true;
  state = KILLED;
  disableAllDrivers();
  setLED(255, 0, 0);
  Serial.println("!!! KILL TRIGGERED !!!");
}

bool checkKill() {
  if (killRequested) {
    state = KILLED;
    extruderPrimeMode = false;
    disableAllDrivers();
    setLED(255, 0, 0);
    return true;
  }
  return false;
}

void startExtruderPrimeMode() {
  if (checkKill()) return;

  extruderPrimeMode = true;
  lastExtruderPrimeStepUs = micros();
  enableDriver(EX_ENABLE);
  digitalWrite(EX_DIR, EXTRUDER_PRIME_DIR ? HIGH : LOW);
  Serial.println("Extruder prime mode ON");
}

void stopExtruderPrimeMode() {
  extruderPrimeMode = false;
  digitalWrite(EX_STEP, LOW);
  digitalWrite(EX_ENABLE, HIGH);
  Serial.println("Extruder prime mode OFF");
}

void serviceExtruderPrimeMode() {
  if (!extruderPrimeMode) return;
  if (state != IDLE) return;
  if (checkKill()) return;

  unsigned long nowUs = micros();
  if ((unsigned long)(nowUs - lastExtruderPrimeStepUs) >= EXTRUDER_PRIME_INTERVAL_US) {
    enableDriver(EX_ENABLE);
    digitalWrite(EX_DIR, EXTRUDER_PRIME_DIR ? HIGH : LOW);
    stepOnce(EX_STEP, EXTRUDER_PULSE_US);
    lastExtruderPrimeStepUs = nowUs;
  }
}

void startPrintJob() {
  if (checkKill()) return;

  if (extruderPrimeMode) {
    stopExtruderPrimeMode();
  }

  state = PRINTING;
  showStateLED();
  Serial.println("Printing UB logo path...");

  moveToPrintStart();
  if (checkKill()) return;

  runLogoPath(logoPoints, NUM_POINTS, 800);
  if (checkKill()) return;

  returnToHome();
  if (checkKill()) return;

  state = COMPLETED;
  showStateLED();
  delay(1000);

  if (checkKill()) return;

  state = IDLE;
  showStateLED();
}

void runLogoPath(const PlotPoint *path, int n, int pulseUs) {
  if (n <= 0) return;
  if (checkKill()) return;

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
  if (checkKill()) return;

  for (int i = 1; i < n; i++) {
    if (checkKill()) return;

    float tgtXmm = (path[i].x - logoMinX) * scale;
    float tgtYmm = (path[i].y - logoMinY) * scale;

    long dxSteps = lround(-(tgtXmm - curXmm) * X_STEPS_PER_MM);
    long dySteps = lround(-(tgtYmm - curYmm) * Y_STEPS_PER_MM);

    /*If this segment is meant to draw, lower tool first*/
    if (path[i].penDown && !toolIsDown) {
      lowerTool();
      toolIsDown = true;
      if (checkKill()) return;
    }

    /*If this segment is meant to travel, lift tool first*/
    if (!path[i].penDown && toolIsDown) {
      liftTool();
      toolIsDown = false;
      if (checkKill()) return;
    }

    moveXY(dxSteps, dySteps, pulseUs);
    if (checkKill()) return;

    curXmm = tgtXmm;
    curYmm = tgtYmm;
  }

  /*Lift tool at the end so it does not drag after finishing*/
  if (toolIsDown) {
    liftTool();
  }
}

void runMinimapPath(const Pt *path, int n, int pulseUs) {
  if (n <= 0) return;
  if (checkKill()) return;

  float s = cookieScale();

  // large cookie design area (mm) with margins
  const float W = 70.0 * s;
  const float H = 45.0 * s;

  float curXmm = path[0].x * W;
  float curYmm = path[0].y * H;

  for (int i = 1; i < n; i++) {
    if (checkKill()) return;

    float tgtXmm = path[i].x * W;
    float tgtYmm = path[i].y * H;

    long dxSteps = lround((tgtXmm - curXmm) * X_STEPS_PER_MM);
    long dySteps = lround((tgtYmm - curYmm) * Y_STEPS_PER_MM);

    moveXY(dxSteps, dySteps, pulseUs);
    if (checkKill()) return;

    curXmm = tgtXmm;
    curYmm = tgtYmm;
  }
}


void moveXY(long dxSteps, long dySteps, int pulseUs) {
  if (checkKill()) return;

  enableDriver(X_ENABLE);
  enableDriver(Y_ENABLE);

  bool extruderPrinting = (state == PRINTING);
  int motionLoops = 0;

  if (extruderPrinting) {
    enableDriver(EX_ENABLE);
    digitalWrite(EX_DIR, EXTRUDER_PRINT_DIR ? HIGH : LOW);
  }

  digitalWrite(X_DIR, (dxSteps >= 0) ? HIGH : LOW);
  digitalWrite(Y_DIR, (dySteps >= 0) ? HIGH : LOW);

  long ax = labs(dxSteps);
  long ay = labs(dySteps);

  bool xMovingBackward = (dxSteps > 0);   // flip if needed
  bool yMovingBackward = (dySteps < 0);   // flip if needed

  long err = ax - ay;
  long cx = 0, cy = 0;

  while (cx < ax || cy < ay) {
    if (checkKill()) {
      Serial.println("KILL detected during moveXY!");
      return;
    }

    long e2 = 2 * err;
    bool steppedXY = false;

    if (e2 > -ay && cx < ax) {
      if (xMovingBackward && isXLimitPressed()) {
        Serial.println("X limit hit - stopping backward X motion");
        disableAllDrivers();
        return;
      }

      err -= ay;
      stepOnce(X_STEP, pulseUs);
      cx++;
      steppedXY = true;
    }

    if (e2 < ax && cy < ay) {
      if (yMovingBackward && isYLimitPressed()) {
        Serial.println("Y limit hit - stopping backward Y motion");
        disableAllDrivers();
        return;
      }

      err += ax;
      stepOnce(Y_STEP, pulseUs);
      cy++;
      steppedXY = true;
    }

    if (extruderPrinting && steppedXY) {
      motionLoops++;
      if (motionLoops >= EXTRUDER_PRINT_STEP_DIVIDER) {
        stepOnce(EX_STEP, EXTRUDER_PULSE_US);
        motionLoops = 0;
      }
    }
  }

  disableAllDrivers();
}

float cookieScale() {
  if (cookieSize == SIZE_SMALL) {
    return 2.375 / 4.25;  // ~0.559
  }
  return 1.0; // LARGE or NONE defaults to full scale
}




void showCookieSizeLED() {
  if (cookieSize == SIZE_SMALL) {
    setLED(255, 80, 0);        // Orange = SMALL
  } else if (cookieSize == SIZE_LARGE) {
    setLED(255, 255, 255);     // White = LARGE
  } else {
    showStateLED();            // no size selected -> show current state color
  }
}

void flashCookieSizeLED(unsigned long durationMs) {
  if (cookieSize == SIZE_NONE) {
    showStateLED();
    sizeFlashActive = false;
    return;
  }

  showCookieSizeLED();
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

  if (largeActive && !smallActive) {
    cookieSize = SIZE_LARGE;
    Serial.println("Boot Cookie Size = LARGE");
    flashCookieSizeLED(400);
  } else if (smallActive && !largeActive) {
    cookieSize = SIZE_SMALL;
    Serial.println("Boot Cookie Size = SMALL");
    flashCookieSizeLED(400);
  } else {
    cookieSize = SIZE_NONE;
    Serial.println("Boot Cookie Size = NONE");
    sizeFlashActive = false;
  }
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
  if (killRequested) return;

  digitalWrite(step_pin, HIGH);
  delayMicroseconds(pulse);
  digitalWrite(step_pin, LOW);
  delayMicroseconds(pulse);
}

void jogAxis(int enPin, int dirPin, int stepPin, bool dir, int steps, int pulseUs) {
  if (checkKill()) return;

  enableDriver(enPin);
  digitalWrite(dirPin, dir ? HIGH : LOW);

  for (int i = 0; i < steps; i++) {
    if (checkKill()) {
      Serial.println("KILL detected during jog!");
      return;
    }

    /*Prevent Z from driving farther downward into the bottom switch*/
    if (enPin == Z_ENABLE && dir == Z_DOWN_DIR && isZLimitPressed()) {
      Serial.println("Z limit hit - stopping downward Z motion");
      disableAllDrivers();
      return;
    }

    stepOnce(stepPin, pulseUs);
  }

  disableAllDrivers();
}

void liftTool() {
  if (checkKill()) return;
  jogAxis(Z_ENABLE, Z_DIR, Z_STEP, Z_UP_DIR, Z_LIFT_STEPS, Z_PULSE_US);
}

void lowerTool() {
  if (checkKill()) return;
  jogAxis(Z_ENABLE, Z_DIR, Z_STEP, Z_DOWN_DIR, Z_LOWER_STEPS, Z_PULSE_US);
}

bool isYLimitPressed() {
  /*NC + INPUT_PULLUP wiring:
    normal state = LOW
    pressed / hit = HIGH
  */
  return digitalRead(Y_LIMIT_SW) == HIGH;
}

bool isXLimitPressed() {
  /*NC + INPUT_PULLUP wiring:
    normal state = LOW
    pressed / hit = HIGH
  */
  return digitalRead(X_LIMIT_SW) == HIGH;
}

void homeXAxis() {
  Serial.println("Homing X axis...");

  enableDriver(X_ENABLE);

  /*Move X toward the limit switch*/
  digitalWrite(X_DIR, HIGH);   // if wrong direction, change HIGH to LOW

  while (!isXLimitPressed()) {
    if (checkKill()) {
      Serial.println("KILL detected during X homing!");
      return;
    }

    stepOnce(X_STEP, 1000);
  }

  Serial.println("X limit hit.");

  /*Back off a little so it is not sitting on the switch*/
  digitalWrite(X_DIR, LOW);   // opposite of home direction
  for (int i = 0; i < 400; i++) {
    if (checkKill()) return;
    stepOnce(X_STEP, 1000);
  }

  disableAllDrivers();
  Serial.println("X homing complete.");
}

void homeYAxis() {
  Serial.println("Homing Y axis...");

  enableDriver(Y_ENABLE);

  /*Move Y backward toward the limit switch*/
  digitalWrite(Y_DIR, HIGH);   // if wrong direction, change LOW to HIGH

  while (!isYLimitPressed()) {
    if (checkKill()) {
      Serial.println("KILL detected during Y homing!");
      return;
    }

    stepOnce(Y_STEP, 1000);
  }

  Serial.println("Y limit hit.");

  /*Back off a little so it is not sitting on the switch*/
  digitalWrite(Y_DIR, LOW);  // opposite of home direction
  for (int i = 0; i < 400; i++) {
    if (checkKill()) return;
    stepOnce(Y_STEP, 1000);
  }

  disableAllDrivers();
  Serial.println("Y homing complete.");
}

bool isZLimitPressed() {
  /*NC + INPUT_PULLUP wiring:
    normal state = LOW
    pressed / hit = HIGH
  */
  return digitalRead(Z_LIMIT_SW) == HIGH;
}

void homeZAxis() {
  Serial.println("Homing Z axis...");
  setLED(255, 0, 255);   /*Pink during homing*/

  enableDriver(Z_ENABLE);

  /*Move Z downward toward the bottom limit switch*/
  digitalWrite(Z_DIR, Z_DOWN_DIR ? HIGH : LOW);

  while (!isZLimitPressed()) {
    if (checkKill()) {
      Serial.println("KILL detected during Z homing!");
      return;
    }

    stepOnce(Z_STEP, 1200);
  }

  Serial.println("Z limit hit.");

  /*Back off a little upward so it is not sitting on the switch*/
  digitalWrite(Z_DIR, Z_UP_DIR ? HIGH : LOW);
  for (int i = 0; i < 200; i++) {
    if (checkKill()) return;
    stepOnce(Z_STEP, 1200);
  }

  disableAllDrivers();
  Serial.println("Z homing complete.");
}

void moveToPrintStart() {
  if (checkKill()) return;

  long xSteps = lround(-PRINT_START_X_MM * X_STEPS_PER_MM);
  long ySteps = lround(-PRINT_START_Y_MM * Y_STEPS_PER_MM);

  Serial.println("Moving to print start position...");

  moveXY(xSteps, ySteps, 800);
}

void returnToHome() {
  if (checkKill()) return;

  Serial.println("Returning to home position...");

  homeXAxis();
  if (checkKill()) return;
  homeYAxis();
  if (checkKill()) return;
  homeZAxis();
}

void homeXYAxes() {
  Serial.println("Homing X and Y axes together...");
  setLED(255, 0, 255);   /*Pink during homing*/

  enableDriver(X_ENABLE);
  enableDriver(Y_ENABLE);

  bool xHomed = false;
  bool yHomed = false;

  /*Move both axes toward their limit switches*/
  digitalWrite(X_DIR, HIGH);   // flip if needed
  digitalWrite(Y_DIR, HIGH);   // flip if needed

  while (!xHomed || !yHomed) {
    if (checkKill()) {
      Serial.println("KILL detected during XY homing!");
      return;
    }

    if (!xHomed) {
      if (isXLimitPressed()) {
        xHomed = true;
        Serial.println("X limit hit.");
      } else {
        stepOnce(X_STEP, 1000);
      }
    }

    if (!yHomed) {
      if (isYLimitPressed()) {
        yHomed = true;
        Serial.println("Y limit hit.");
      } else {
        stepOnce(Y_STEP, 1000);
      }
    }
  }

  /*Back off both axes a little from the switches*/
  digitalWrite(X_DIR, LOW);   // opposite of homing direction
  digitalWrite(Y_DIR, LOW);   // opposite of homing direction

  for (int i = 0; i < 400; i++) {
    if (checkKill()) return;
    stepOnce(X_STEP, 1000);
    stepOnce(Y_STEP, 1000);
  }

  disableAllDrivers();

  Serial.println("XY homing complete.");
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

  pinMode(Y_LIMIT_SW, INPUT_PULLUP);
  pinMode(X_LIMIT_SW, INPUT_PULLUP);
  pinMode(Z_LIMIT_SW, INPUT_PULLUP);

  initCookieSizeFromSwitches();
  //setLED(0,0, 255); /*Make sure LED still outputs BLUE for IDLE mode*/


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

  homeXYAxes();

 

  homeZAxis();

  state = IDLE ;
  showStateLED();

 // moveToPrintStart();


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

  if (bKill.fell()) {
    Serial.println("Kill switch Button pressed");
    triggerKill();
  }

  if (killRequested) {
    disableAllDrivers();
    state = KILLED;
    setLED(255, 0, 0);
    return;
  }

  serviceExtruderPrimeMode();

  if (sizeFlashActive && (long)(millis() - sizeFlashUntilMs) >= 0) {
    sizeFlashActive = false;
    showStateLED();   // return to current state color
  }

  /*When the button is not pressed, BUTTON = 1; When the button IS PRESSED, BUTTON = 0 (ACTIVE LOW)*/

  /*If the button has been pressed*/
  if (bPrint.fell()) {
    Serial.println("PRINT Button pressed");
  }

  if (bPrint.rose()) {
    if (checkKill()) return;

    if (state == IDLE) {
      if (cookieSize == SIZE_NONE) {
        if (!extruderPrimeMode) {
          startExtruderPrimeMode();
        } else {
          stopExtruderPrimeMode();
        }
      } else {
        startPrintJob();
      }
      return;
    }
  }

  if (bRefill.fell()) {
    if (checkKill()) return;

    if (extruderPrimeMode) {
      stopExtruderPrimeMode();
    }

    if (state != REFILL_CLEAN) {
      state = REFILL_CLEAN;      // enter refill/clean mode and stay there
      setLED(255, 0, 255);       // SET LED TO PURPLE
      Serial.println("REFILL/CLEAN mode ON");
      Serial.println("Releasing extruder piston...");
      jogAxis(EX_ENABLE, EX_DIR, EX_STEP, false, 200 * 16, 800); // reverse a bit
      if (checkKill()) return;
    } else {
      state = IDLE;              // exit refill/clean mode
      setLED(0, 0, 255);         // Blue
      Serial.println("REFILL/CLEAN mode OFF");
    }
  }

  if (bSize_LARGE.changed() || bSize_SMALL.changed()) {
    bool largeActive = (digitalRead(cookie_size_LARGE) == LOW);
    bool smallActive = (digitalRead(cookie_size_SMALL) == LOW);

    if (largeActive && !smallActive) {
      cookieSize = SIZE_LARGE;
      if (extruderPrimeMode) stopExtruderPrimeMode();
      Serial.println("Cookie Size set to LARGE");
      flashCookieSizeLED(1500);
    } 
    else if (smallActive && !largeActive) {
      cookieSize = SIZE_SMALL;
      if (extruderPrimeMode) stopExtruderPrimeMode();
      Serial.println("Cookie Size set to SMALL");
      flashCookieSizeLED(1500);
    }
    else {
      cookieSize = SIZE_NONE;
      if (extruderPrimeMode) stopExtruderPrimeMode();
      sizeFlashActive = false;
      Serial.println("Cookie size cleared / no size selected");
      showStateLED();
    }
  }
}
