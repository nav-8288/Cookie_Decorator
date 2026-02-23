#include <Bounce2.h> /*bounce 2 library used for reading peripherals such as buttons or switches*/

/*INCLUDE LOGIC FOR LIMIT SWITCHES */

#define print_SW A9 /* For dig Pin A9*/
#define kill_PB A5 /*For dig pin A5 */
#define refill_clean_PB A0 /*For dig pin A0*/
#define cookie_size_SW 2 /*For dig pin D2  */

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

/*Bounce objects store state of pin; Last reading (High/Low), last time changed, */
Bounce bPrint = Bounce(); /*For print button*/
Bounce bKill = Bounce(); /*For kill switch */
Bounce bRefill = Bounce(); /*For refill/clean switch */
Bounce bSize = Bounce(); /*for cookie size sw (SMALL/LARGE)*/


void setup() {
  Serial.begin(115200); /*Initialize serial communication between arduino and monitor*/
  /*115200 describes baud rate (bits per second) */
  pinMode(print_SW, INPUT_PULLUP); /*Initialize print sw*/
  pinMode(kill_PB, INPUT_PULLUP); /*Initialize kill pb*/
  pinMode(refill_clean_PB, INPUT_PULLUP); /*Initialize refill/clean push button*/
  pinMode(cookie_size_SW, INPUT_PULLUP); /*Initialize cookie size switch */

  /*Initialize DIR and STEP FOR ALL 4 DRIVERS*/
  pinMode(X_DIR, OUTPUT); /*Initialize DIR pin on a4988 driver ; tells driver which dir to step (forw/back) */
  pinMode(X_STEP, OUTPUT); /*Initialize STEP pin on a4988 driver ; step pulses to make motor move */
  /*step and dir are control signals sent to a4988 driver*/

  pinMode(Y_DIR, OUTPUT); 
  pinMode(Y_STEP, OUTPUT);

  pinMode(Z_DIR, OUTPUT); 
  pinMode(Z_STEP, OUTPUT);

  pinMode(EX_DIR, OUTPUT); 
  pinMode(EX_STEP, OUTPUT);

  /*Initialize pins for the ENABLE pins on A4988 DRIVER*/
  pinMode(X_ENABLE, OUTPUT); 
  pinMode(Y_ENABLE, OUTPUT);

  pinMode(Z_ENABLE, OUTPUT); 
  pinMode(EX_ENABLE, OUTPUT);
  
  
  digitalWrite(X_STEP, LOW); /*set step to low so its starts with clean state; prevents random step from unknown state*/

  digitalWrite(Y_STEP, LOW); /*set step to low so its starts with clean state; prevents random step from unknown state*/

  digitalWrite(Z_STEP, LOW); /*set step to low so its starts with clean state; prevents random step from unknown state*/

  digitalWrite(EX_STEP, LOW); /*set step to low so its starts with clean state; prevents random step from unknown state*/

  /*SET ENABLE PINS TO HIGH TO DISABLE ; ENABLE PINS ARE ACTIVE LOW*/
  digitalWrite(X_ENABLE, HIGH); 

  digitalWrite(Y_ENABLE, HIGH);

  digitalWrite(Z_ENABLE, HIGH);

  digitalWrite(EX_ENABLE, HIGH);


  /*SET DIR PINS TO lOW SO IT STARTS WITH A CLEAN STATE, PREVENTS FROM GOING IN RANDOM DIRECION FROM UNKNOWN STATE*/

  digitalWrite(X_DIR, LOW);

  digitalWrite(Y_DIR, LOW)

  digitalWrite(Z_DIR, LOW);

  digitalWrite(EX_DIR, LOW);


/*Each bounce obj should know which Arduino pin to read*/
  bPrint.attach(print_SW); /*tracks the state of pin A9*/
  bKill.attach(kill_PB); /*tracks the state of pin A5*/
  bRefill.attach(refill_clean_PB); /*tracks the state of pin A0*/
  bSize.attach(cookie_size_SW); /*tracks the state of pin D2*/

/*Sets debounce window, buttons and sw's "bounce electrically" meaning single press can rapidly flicker
Adding .interval() allows signal to stay stable for 15ms before accepting button/SW press  */
  bPrint.interval(15);
  bKill.interval(15);
  bRefill.interval(15);
  bSize.interval(15);

  /*IF THE BUTTON HAS BEEN HELD FOR 15ms, accept the new state*/

  
  // put your setup code here, to run once:

}

void loop() {
/*update must be called repeatedly so the pin is read in present time and decide if real change happened
functions fell(), rose() and changed() WILL NOT work without update()*/
  bPrint.update();
  bKill.update();
  bRefill.update();
  bSize.update();

  /*When the button is not pressed, BUTTON = 1; When the button IS PRESSED, BUTTON = 0 (ACTIVE LOW)*/

/*If the button has been pressed*/
  if(bPrint.fell()){
    Serial.println("PRINT Button pressed");
    }

/*If the button has been released*/
 if(bPrint.rose()){
    Serial.println("PRINT Button released");
    }

    /*If the button has been pressed*/
  if(bKill.fell()){
    Serial.println("Kill switch Button pressed");
    }

/*If the button has been released*/
 if(bKill.rose()){
    Serial.println("Kill switch Button released");
    }

    /*If the button has been pressed*/
  if(bRefill.fell()){
    Serial.println("Refill/Clean Button pressed");
    }

/*If the button has been released*/
 if(bRefill.rose()){
    Serial.println("Refill/Clean Button released");
    }

/*If the cookie size switch has flipped positions (small/large)*/
 if(bSize.changed()){
    if(bSize.read() == LOW){
      Serial.println("Cookie Size set to LARGE");
      }else{
        Serial.println("Cookie Size set to SMALL");
      }
  }

  // put your main code here, to run repeatedly:

}
