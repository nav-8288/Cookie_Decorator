#include <Bounce2.h>
#include "cookie_config.h"
#include "logo_data.h"

// --- prototypes ---
float cookieScale();
void runLogoPath(const PlotPoint *path, int n, int pulseUs);
void moveXY(long dxSteps, long dySteps, int pulseUs);
void setLED(int r, int g, int b);

void enableDriver(int ENABLE_PIN);
void disableAllDrivers();
void stopAllDrivers();
void stepOnce(int step_pin, int pulse);
void liftTool();
void lowerTool();
bool isYLimitPressed();
bool isXLimitPressed();
bool isZLimitPressed();
void homeXYAxes();
void homeZAxis();
void moveToPrintStart();
void returnToHome();
void triggerKill();
void startPrintJob();
void showStateLED();
void initCookieSizeFromSwitches();
void handleIdle();
void enterNeedsRefill();

void actuatorExtend();
void actuatorRetract();
void actuatorStop();
void actuatorRetractBrief();

int readActuatorExtendCurrent();
bool actuatorExtendStalled();

// ----- states -----
typedef enum {
  IDLE,
  SIZE_SMALL_PRINT,
  SIZE_LARGE_PRINT,
  COMPLETED,
  NEEDS_REFILL,
  REFILL_CLEAN,
  KILLED
} State;

// ----- globals -----
State state = IDLE;
State stateBeforeKill = IDLE;
State stateBeforeRefill = IDLE;

volatile bool killActive = false;
volatile bool killPressedEvent = false;
volatile bool killReleasedEvent = false;

bool extruderJogOn = false;

unsigned long lastExtruderPrimeStepUs = 0;
unsigned long sizeFlashUntilMs = 0;

unsigned long actuatorOverCurrentStartMs = 0;

CookieSize cookieSize = SIZE_NONE;
// ----- Bounce -----
Bounce bPrint = Bounce();
Bounce bRefill = Bounce();
Bounce bSize_SMALL = Bounce();
Bounce bSize_LARGE = Bounce();

//-------Helper Functions-----------

int readActuatorExtendCurrent() {
  /* pick the one that matches the extend direction on your driver */
  return analogRead(ACT_R_IS);
}

bool actuatorExtendStalled() {
  int currentReading = readActuatorExtendCurrent();

  if (currentReading >= ACT_EXTEND_CURRENT_THRESHOLD) {
    if (actuatorOverCurrentStartMs == 0) {
      actuatorOverCurrentStartMs = millis();
    }

    if ((millis() - actuatorOverCurrentStartMs) >= ACT_STALL_TIME_MS) {
      return true;
    }
  } else {
    actuatorOverCurrentStartMs = 0;
  }

  return false;
}

void actuatorExtend() {
  analogWrite(ACT_R_PWM, ACT_PWM_SPEED);
  analogWrite(ACT_L_PWM, 0);
}

void actuatorRetract() {
  analogWrite(ACT_R_PWM, 0);
  analogWrite(ACT_L_PWM, ACT_PWM_SPEED);
}

void actuatorStop() {
  analogWrite(ACT_R_PWM, 0);
  analogWrite(ACT_L_PWM, 0);
  actuatorOverCurrentStartMs = 0;
}

void actuatorRetractBrief() {
  actuatorRetract();
  delay(ACT_RETRACT_MS);
  actuatorStop();
}

void enterNeedsRefill() {
  if (state == NEEDS_REFILL || state == KILLED) return;

  stateBeforeRefill = state;
  state = NEEDS_REFILL;

  extruderJogOn = false;

  actuatorStop();
  stopAllDrivers();


  showStateLED();
}
// for the emerganince stop button so used in the interupt 
void triggerKill() {
  if (digitalRead(kill_PB) == LOW) {   // pressed with INPUT_PULLUP
    killActive = true;
    killPressedEvent = true;
  } else {
    killActive = false;
    killReleasedEvent = true;
  }
}

// starts printing based on the size of the cookie mode chosen 
void startPrintJob() {
  extruderJogOn = false;
  showStateLED();

  moveToPrintStart();
  if (killActive ) return;
  
  runLogoPath(logoPoints, NUM_POINTS, 800);
  if (killActive ) return;
  
  returnToHome();
  if (killActive) return;
  
  state = COMPLETED;
  showStateLED();
  delay(1000);
}


void runLogoPath(const PlotPoint *path, int n, int pulseUs) {
  if (n <= 0) return;

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
    // emergency button check
    if (killActive) {
      disableAllDrivers();
      return;
    }
    if (toolIsDown && actuatorExtendStalled()) {
  actuatorStop();
  enterNeedsRefill();
  return;
}
    float tgtXmm = (path[i].x - logoMinX) * scale;
    float tgtYmm = (path[i].y - logoMinY) * scale;

    long dxSteps = lround(-(tgtXmm - curXmm) * X_STEPS_PER_MM);
    long dySteps = lround(-(tgtYmm - curYmm) * Y_STEPS_PER_MM);

    if (path[i].penDown && !toolIsDown) {
  lowerTool();
  if (killActive) {
    disableAllDrivers();
    actuatorStop();
    return;
  }
  if (actuatorExtendStalled()) {
    actuatorStop();
    enterNeedsRefill();
    return;
  }

  actuatorExtend();   /*start pushing frosting*/
  toolIsDown = true;
}

if (!path[i].penDown && toolIsDown) {
  actuatorStop();     /*stop frosting before travel*/
  actuatorRetractBrief();

  liftTool();
  if (killActive) {
    disableAllDrivers();
    actuatorStop();
    return;
  }
  

  toolIsDown = false;
}

    moveXY(dxSteps, dySteps, pulseUs);
    if (killActive) {
      disableAllDrivers();
      return;
    }
    
    curXmm = tgtXmm;
    curYmm = tgtYmm;
  }

  if (toolIsDown) {
    actuatorStop();
    actuatorRetractBrief();
    liftTool();
  }
}

void runMinimapPath(const Pt *path, int n, int pulseUs) {
  if (n <= 0) return;

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

    curXmm = tgtXmm;
    curYmm = tgtYmm;
  }
}


void moveXY(long dxSteps, long dySteps, int pulseUs) {
  enableDriver(X_ENABLE);
  enableDriver(Y_ENABLE);

  digitalWrite(X_DIR, (dxSteps >= 0) ? HIGH : LOW);
  digitalWrite(Y_DIR, (dySteps >= 0) ? HIGH : LOW);

  long ax = labs(dxSteps);
  long ay = labs(dySteps);

  bool xMovingBackward = (dxSteps > 0);
  bool yMovingBackward = (dySteps < 0);

  long err = ax - ay;
  long cx = 0, cy = 0;

  while (cx < ax || cy < ay) {
    if (killActive) {
      disableAllDrivers();
      return;
    }
    
    long e2 = 2 * err;
    bool steppedXY = false;

    if (e2 > -ay && cx < ax) {
      if (xMovingBackward && isXLimitPressed()) {
        stopAllDrivers();
        return;
      }

      err -= ay;
      stepOnce(X_STEP, pulseUs);
      cx++;
      steppedXY = true;
    }

    if (e2 < ax && cy < ay) {
      if (yMovingBackward && isYLimitPressed()) {
        stopAllDrivers();
        return;
      }

      err += ax;
      stepOnce(Y_STEP, pulseUs);
      cy++;
      steppedXY = true;
    }

   
  }

  stopAllDrivers();
}

void showStateLED() {
  if (killActive == true) {
    setLED(255, 0, 0);
  } else if (state == COMPLETED) {
    setLED(0, 255, 0);
  } else if (state == NEEDS_REFILL) {
    setLED(255, 255, 0);   // Yellow = needs refill
  }else if (state == REFILL_CLEAN) {
    setLED(255, 0, 255);
  }else if (cookieSize == SIZE_SMALL ) {
    setLED(255, 80, 0);        // Orange = SMALL
  } else if (cookieSize == SIZE_LARGE ) {
    setLED(255, 255, 255);     // White = LARGE
  }  else {
    setLED(0, 0, 255);
  }
}

float cookieScale() {
  if (cookieSize == SIZE_SMALL) {
    return 2.375 / 4.25;  // ~0.559
  }
  return 1.0; // LARGE or NONE defaults to full scale
}

/*Function used to check which cookie-size switch is currently selected at POWER ON*/
/*Sets the cookiesize var to match and prints it/updates the LED*/
void initCookieSizeFromSwitches() {
  // switches are INPUT_PULLUP, so "selected" position usually reads LOW
  bool largeActive = (digitalRead(cookie_size_LARGE) == LOW);
  bool smallActive = (digitalRead(cookie_size_SMALL) == LOW);

  if (largeActive && !smallActive) {
    cookieSize = SIZE_LARGE;
    showStateLED();
  } else if (smallActive && !largeActive) {
    cookieSize = SIZE_SMALL;
    showStateLED();
  } else {
    cookieSize = SIZE_NONE;
    showStateLED();
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

void stopAllDrivers(){
  digitalWrite(X_STEP, LOW);
  digitalWrite(Y_STEP, LOW);
  digitalWrite(Z_STEP, LOW);
}

//function used to set enable pins 
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
    // emergency button check
    if (killActive) {
      disableAllDrivers();
      return;
    }
    
    /*Prevent Z from driving farther downward into the bottom switch*/
    if (enPin == Z_ENABLE && dir == Z_DOWN_DIR && isZLimitPressed()) {
      stopAllDrivers();
      return;
    }

    stepOnce(stepPin, pulseUs);
  }

  stopAllDrivers();
}

void liftTool() {
  jogAxis(Z_ENABLE, Z_DIR, Z_STEP, Z_UP_DIR, Z_LIFT_STEPS, Z_PULSE_US);
}

void lowerTool() {
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

bool isZLimitPressed() {
  /*NC + INPUT_PULLUP wiring:
    normal state = LOW
    pressed / hit = HIGH
  */
  return digitalRead(Z_LIMIT_SW) == HIGH;
}



void homeZAxis() {
  //check emergency button
  if (killActive) {
    disableAllDrivers();
    return;
  }
  
  enableDriver(Z_ENABLE);
  /*Move Z downward toward the bottom limit switch*/
  digitalWrite(Z_DIR, Z_DOWN_DIR ? HIGH : LOW);
  while (!isZLimitPressed()) {
    //check emergency button
    if (killActive) {
      disableAllDrivers();
      return;
    }
   
    stepOnce(Z_STEP, 1200);
  }
  /*Back off a little upward so it is not sitting on the switch*/
  digitalWrite(Z_DIR, Z_UP_DIR ? HIGH : LOW);
  for (int i = 0; i < 200; i++) {
    if (killActive) {
      disableAllDrivers();
      return;
    }
    
    stepOnce(Z_STEP, 1200);
  }
  stopAllDrivers();
}

void moveToPrintStart() {

  long xSteps = lround(-PRINT_START_X_MM * X_STEPS_PER_MM);
  long ySteps = lround(-PRINT_START_Y_MM * Y_STEPS_PER_MM);

  moveXY(xSteps, ySteps, 800);
}

void returnToHome() {
  homeXYAxes();
  homeZAxis();
}

void homeXYAxes() {
  //check emergency button
  if (killActive) {
  disableAllDrivers();
  return;
  }
  
  setLED(255, 0, 255);   /*Pink during homing*/

  enableDriver(X_ENABLE);
  enableDriver(Y_ENABLE);

  bool xHomed = false;
  bool yHomed = false;

  /*Move both axes toward their limit switches*/
  digitalWrite(X_DIR, HIGH);   // flip if needed
  digitalWrite(Y_DIR, HIGH);   // flip if needed

  while (!xHomed || !yHomed) {
    //check emergency button
    if (killActive) {
      disableAllDrivers();
      return;
    }
    
    if (!xHomed) {
      if (isXLimitPressed()) {
        xHomed = true;
      } else {
        stepOnce(X_STEP, 1000);
      }
    }

    if (!yHomed) {
      if (isYLimitPressed()) {
        yHomed = true;
      } else {
        stepOnce(Y_STEP, 1000);
      }
    }
  }

  /*Back off both axes a little from the switches*/
  digitalWrite(X_DIR, LOW);   // opposite of homing direction
  digitalWrite(Y_DIR, LOW);   // opposite of homing direction

  for (int i = 0; i < 400; i++) {
    //check emergency button
    if (killActive) {
    disableAllDrivers();
    return;
    }
   
    stepOnce(X_STEP, 1000);
    stepOnce(Y_STEP, 1000);
  }

  stopAllDrivers();

}
// th is function is for the action when the swicth is on idle and the print button is pressed 
void handleIdle() {
  // emergency stop
  if (killActive) {
    extruderJogOn = false;
    actuatorStop();
    return;
  }

 

  // check switch portion
  initCookieSizeFromSwitches();

  // enter refill/clean mode only from IDLE
  if (bRefill.fell()) {
    extruderJogOn = false;
    actuatorStop();
    state = REFILL_CLEAN;
    showStateLED();
    return;
  }

  // use print button for either print mode or manual actuator jog
  if (bPrint.fell()) {
    if (cookieSize == SIZE_SMALL) {
      extruderJogOn = false;
      actuatorStop();
      state = SIZE_SMALL_PRINT;
      return;

    } else if (cookieSize == SIZE_LARGE) {
      extruderJogOn = false;
      actuatorStop();
      state = SIZE_LARGE_PRINT;
      return;

    } else {
      // toggle actuator jog mode
      extruderJogOn = !extruderJogOn;

      if (extruderJogOn) {
        actuatorExtend();
      } else {
        actuatorStop();
      }
    }
  }
}

void setup() {
  Serial.begin(115200); /*Initialize serial communication between arduino and monitor*/
  /*115200 describes baud rate (bits per second) */

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  pinMode(print_SW, INPUT_PULLUP); /*Initialize print sw*/
  pinMode(kill_PB, INPUT_PULLUP); /*Initialize kill pb*/
  pinMode(refill_clean_PB, INPUT_PULLUP); /*Initialize refill/clean push button*/
  pinMode(cookie_size_SMALL, INPUT_PULLUP); /*Initialize cookie size switch SMALL */
  pinMode(cookie_size_LARGE, INPUT_PULLUP); /*Initialize cookie size switch SMALL */

  pinMode(Y_LIMIT_SW, INPUT_PULLUP);
  pinMode(X_LIMIT_SW, INPUT_PULLUP);
  pinMode(Z_LIMIT_SW, INPUT_PULLUP);

  pinMode(ACT_R_PWM, OUTPUT);
  pinMode(ACT_L_PWM, OUTPUT);

  actuatorStop();

  attachInterrupt(digitalPinToInterrupt(kill_PB), triggerKill, CHANGE);

  /*Initialize DIR and STEP FOR ALL 4 DRIVERS*/

  initDriver(X_ENABLE, X_DIR, X_STEP);
  initDriver(Y_ENABLE, Y_DIR, Y_STEP);
  initDriver(Z_ENABLE, Z_DIR, Z_STEP);

  /*Each bounce obj should know which Arduino pin to read*/
  bPrint.attach(print_SW); /*tracks the state of pin A9*/
  bRefill.attach(refill_clean_PB); /*tracks the state of pin A0*/
  bSize_LARGE.attach(cookie_size_LARGE); /*tracks the state of pin D2*/
  bSize_SMALL.attach(cookie_size_SMALL); /*tracks the state of pin D3*/


  /*Sets debounce window, buttons and sw's "bounce electrically" meaning single press can rapidly flicker
  Adding .interval() allows signal to stay stable for 15ms before accepting button/SW press  */
  bPrint.interval(15);
  bRefill.interval(15);
  bSize_SMALL.interval(15);
  bSize_LARGE.interval(15);

  returnToHome();

    
  state = IDLE ;
  initCookieSizeFromSwitches();

}

void loop() {
  bPrint.update();
  bRefill.update();
  bSize_LARGE.update();
  bSize_SMALL.update();

  if (killPressedEvent) {
    killPressedEvent = false;

    if (state != KILLED) {
      stateBeforeKill = state;
    }

    state = KILLED;
    extruderJogOn = false;
    disableAllDrivers();
    actuatorStop();
    showStateLED();
  }

  if (killReleasedEvent) {
    killReleasedEvent = false;

    initDriver(X_ENABLE, X_DIR, X_STEP);
    initDriver(Y_ENABLE, Y_DIR, Y_STEP);
    initDriver(Z_ENABLE, Z_DIR, Z_STEP);
    returnToHome();
    // safer to return to IDLE after kill release
    state = IDLE; 
    showStateLED();
  }

  if (killActive) {
    return;
  }
  
  switch (state) {
    case IDLE:
      handleIdle();
      break;

    case SIZE_SMALL_PRINT:
    case SIZE_LARGE_PRINT:
      startPrintJob();
      break;

    case REFILL_CLEAN:
        extruderJogOn = false;
        actuatorStop();
        showStateLED();

       if (bRefill.fell()) {
          extruderJogOn = false;
          actuatorStop();
          state = IDLE;
          showStateLED();
        }
      break;

    case NEEDS_REFILL:
      extruderJogOn = false;
      actuatorStop();
      stopAllDrivers();
      showStateLED();

      if (bRefill.fell()) {
        returnToHome();
        state = stateBeforeRefill;
        showStateLED();
      }
      break;

    case COMPLETED:
      state = IDLE;
      showStateLED();
      break;

    case KILLED:
      break;
  }
}
