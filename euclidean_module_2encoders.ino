/*Step Sequencer with Euclidean Rythm Generator 
 
Creative Commons License

Step Sequencer with Euclidean Rhythm Generator by Pantala Labs 
is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
Based on a work at https://github.com/PantalaLabs/Euclidean.

Gibran Curtiss Salomão. FEB/2017 - CC-BY-NC-SA
*/

#include <TimerOne.h>
#include "LedControl.h"
#include <Encoder.h>

LedControl lc = LedControl(12, 11, 10, 2); //
// pin 12 - DATA (data-DIN)
// pin 11 - CLK  (CLOCK)
// pin 10 - LOAD (CS)

boolean debug = false;            //very useful to enable / disable some codes from executing without change the entire code

volatile unsigned long  tickInterval = 600;                         //time interval in ms between each interruption, would be cool if saved on EEPROM
unsigned long           timerOneInterval = tickInterval * 1000 ;    //time interval in us for the TimerOne interruption
boolean                 tickScreenState =  true;

//digital pins
#define trigger0Pin          36   //trigger out pins to synth
#define trigger1Pin          38
#define trigger2Pin          40
#define trigger3Pin          42
#define trigger4Pin          44
#define trigger5Pin          46
#define trigger6Pin          48
#define trigger7Pin          50
#define clockInPin           3    //clock in signal
#define clockInButtonPin     6    //clock in button
#define triggerClockPin      52   //clock out
#define pausePin             25   //play/pause
#define resetPin             27   //restart
#define removeEuclidPin      24   //remove entire row from matrix
#define addDotPin            26   //add/remove a dot
#define addEuclidPin         31   //add an euclidean sequence

//analogic pins
#define xAxis                A0  // joystick X axis
#define yAxis                A1  // joystick Y axis

volatile unsigned long  thisLoop;               //millis for each loop()
volatile unsigned long  tickLast;               //last valid millis reading for the loop
volatile int            sweepColumn     = 0;    //column counter for sweep 0-15
volatile int            sweepOldColumn  = -1;   //old column counter for make it easier to restore the changed column
#define                 triggerTime        5   //value in ms to trigger lenght

int                     bleenkDelay = 300;    //led cursor on/off time lenght
unsigned long           bleenkTime;           //last valid millis() for led bleenk event
boolean                 bleenkState = false;  //on/off led state


#define                 matrixDim0 8                    //leds rows
#define                 matrixDim1 16                   //leds columns
boolean                 matrix[matrixDim0][matrixDim1]; //leds matrix (this could be improved to a byte array for faster performance)

#define                 joyAxisRange 12          //joystick range 0 to 12
int                     joyCursor[] = {7, 5};    //actual cursor coordinates (column/row)
int                     joyOldX = joyCursor[0];  //old cursor position
int                     joyOldY = joyCursor[1];  //old cursor position

volatile boolean        euclidParmTimedOut = true;  //indicates if it is time to exit from the euclidean parameters mode state

unsigned long           interfaceEvent;             //last valid millis to any interface input event
int                     interfaceDebounce = 50;    //debounce to avoid many interface inputs at same time or in a row
#define                 interfaceNormalDebounce 50    //debounce to avoid many interface inputs at same time or in a row
#define                 interfaceSuperDebounce  250    //super debounce to avoid 2x interface inputs

int                     euclidParm1 = 1;        //last valid euclidean parameter K (no reason chosen 1)
int                     euclidParm2 = 4;        //last valid euclidean parameter N (no reason chosen 1)

long                    oldEncoderKPosition      = 0;    //old K encoder value
long                    oldEncoderNPosition      = 0;    //old N encoder value
long                    oldEncoderShiftPosition  = 0;    //old shift encoder value
long                    oldEncoderSpeedPosition  = 0;    //old speed encoder value

int                     encoderShiftCounter;              //stores the encoder value for comparison
int                     lastEncoderShiftCounter;          //stores the encoder value for comparison

unsigned long           encoderSpeedValue =   50;         //start value to speed -> tick ->
int                     encoderSpeedCenter =  50;         //start value to speed -> tick ->
#define                 encoderSpeedMinValue  15          //min value in ms for speed encoder 100ms
#define                 encoderSpeedMaxValue  100         //max value in ms for speed encoder 1400ms
#define                 encoderSpeedStepValue 10          //step value for speed encoder

boolean                 shiftEuclidean = false;
boolean                 shiftEuclideanDirection = false;

boolean                 triggersOpenState = true;  //triggers pin high/low state
volatile boolean        externalClock     = true;  //internal / external clock
volatile int            oldTickInterval   = 600;
String                  E[17];                      //store euclidean calculated binary number


Encoder encoderA(8, 9);       //shift interface encoder
Encoder encoderB(22, 23);     //speed interface encoder

volatile long benchTry = 1;
volatile long benchMicros;

void setup()
{
  if (debug) {
    Serial.begin(9600);
  }
  pinMode(trigger0Pin, OUTPUT);
  pinMode(trigger1Pin, OUTPUT);
  pinMode(trigger2Pin, OUTPUT);
  pinMode(trigger3Pin, OUTPUT);
  pinMode(trigger4Pin, OUTPUT);
  pinMode(trigger5Pin, OUTPUT);
  pinMode(trigger6Pin, OUTPUT);
  pinMode(trigger7Pin, OUTPUT);
  pinMode(triggerClockPin, OUTPUT);

  pinMode(clockInPin, INPUT);
  pinMode(clockInButtonPin, INPUT);

  pinMode(pausePin, INPUT);
  pinMode(resetPin, INPUT);
  pinMode(removeEuclidPin, INPUT);
  pinMode(addDotPin, INPUT);
  pinMode(addEuclidPin, INPUT);

  for (int i = 0; i < matrixDim0; i++) {      //clear the led matrix
    for (int j = 0; j < matrixDim1; j++) {
      matrix[i][j] = false;
    }
  }

  for (int i = 0; i < 2; i++) {               //setup and clears the screens
    lc.shutdown(i, false);
    lc.setIntensity(i, 1);
    lc.clearDisplay(i);
  }

  //load some euclidean test values -----------
  calculateESequence(1, 4);
  copyEtoMatrix(0, E[0]);
  calculateESequence(5, 13);
  copyEtoMatrix(1, E[0]);
  calculateESequence(2, 4);
  copyEtoMatrix(2, E[0]);
  calculateESequence(1, 4);
  copyEtoMatrix(3, E[0]);
  calculateESequence(2, 5);
  copyEtoMatrix(4, E[0]);
  calculateESequence(7, 12);
  copyEtoMatrix(5, E[0]);
  calculateESequence(9, 13);
  copyEtoMatrix(6, E[0]);
  calculateESequence(4, 14);
  copyEtoMatrix(7, E[0]);
  //end of euclidean test values -----------

  refreshLeds();                      //refresh screens
  
  externalClock = digitalRead(clockInButtonPin);

  unsigned long now = millis();       //start the timers
  tickLast = now;
  bleenkTime = now;
  Timer1.initialize(timerOneInterval);
  Timer1.attachInterrupt(tickInterrupt);
}


//this is the only critical code to maintain the master clock stable
//any other operation should be done into the interval between two triggers
//you should read ahead that I stop any activity before the next trigger event
//because I had a lot of overhead into refreshing the led matrix
//sometimes, delaying up to 30ms to fire the triggers pins :-(
void tickInterrupt() {
  if (externalClock) {
    tickInterval = millis() - tickLast;
    tickInterval = (tickInterval < (encoderSpeedMinValue*encoderSpeedStepValue)) ? (encoderSpeedMinValue*encoderSpeedStepValue) : tickInterval;   //minimum value of SpeedMinValue
    tickLast = millis();
    firetriggers();
    tickScreenState = true;
  }
  else {
    tickLast = millis();
    timerOneInterval = 1000 * tickInterval;
    firetriggers();
    tickScreenState = true;
  }
}
void tickScreen() {               //refresh displays , update stepBar
  if (euclidParmTimedOut) {       //if not in euclidean state
    //switch off before BAR counter led
    if (sweepOldColumn >= 0) {
      lc.setLed(sweepOldColumn / 8, sweepOldColumn % 8, 7 , matrix[0][sweepOldColumn]);
    }
    //switch on actual BAR counter led
    lc.setLed(sweepColumn / 8, sweepColumn % 8, 7 , !matrix[0][sweepColumn]);
  }
  sweepOldColumn = sweepColumn;   //update stepBar counter
  sweepColumn++;
  if (sweepColumn > 15) {
    sweepColumn = 0;
  }
  tickScreenState = false;        //finish sctreen update state
}

void loop()
{
  thisLoop = millis();
  if (triggersOpenState) {                                   //if triggers are open, calculates the close condition
    if (thisLoop >= (tickLast + triggerTime)) {
      closetriggers();
      if (!externalClock) {
        Timer1.setPeriod(timerOneInterval);                 //update the main clock time
      }
    }
  }
  if (tickScreenState) {                                  //if it is time to refresh the stepBar and screen
    tickScreen();                                         //refresh displays , update stepBar
  }
  //  if ( (shiftEuclidean) && (thisLoop < (tickLast + (tickInterval / 2) ) ) ) {
  //    shiftEuclidean = false;
  //    shiftMatrixRow(joyCursor[1], shiftEuclideanDirection);
  //  }
  //now we calculate if we will have enough time to deal with all interface routines
  //this is to avoid to cause any clock delay or prevent any task could crash led matrix overhead

  //if now is after than last user interface input + own task delay (soft interface debounce to void 2 or more inputs at the same time)
  if (thisLoop > interfaceEvent + interfaceDebounce) {
    interfaceDebounce = interfaceNormalDebounce;
    //if now is after the trigger are closed (this case 5ms)
    if ( (thisLoop > (tickLast + triggerTime + 1))  &&
         //if now is before the end of the full trigger interval les 10ms (these 10ms are to prevent
         //that some task takes too much time to complete and ruins the next trigger rising
         //this time could be changed , and i concluded that it was a good safety margin
         (thisLoop < (tickLast + tickInterval - 30) ) ) { //if NOW is after triggers off and NOW if before the start of next pulse
      //if we are in the main screen (not in euclidean parameters input)
      if (euclidParmTimedOut) {
        //so we can ALMOST certify that we´ll have time to read all interface an do the changes
        //there is a priority of the tasks
        bleenkCursor();
        readEncoderShift();
        readJoystick();
        if (!externalClock) {
          readEncoderSpeed();
          readPauseButton();
        }
        readResetButton();
        readKillButton();
        readAddDotButton();
        readNewEuclidButton();
        readClockSourceButton();
      } else {
        //if we are on euclidean parameters input screen
        readEncoderEuclidKParm();
        readEncoderEuclidNParm();
        readAddEuclidButton();
        //this is a time out , so  , if we forgot to change any parameter in the euclid screen
        //4 seconds after we come back to mais state
        if ( (thisLoop > (interfaceEvent + 4000) ) && (!euclidParmTimedOut) ) {
          giveUpAddEuclid();
        }
      }
    }
  }
}


//changes the main led matrix interface, prepares to display the euclidean parameters
void changeEuclidScreen(int k , int n) {
  markThisEvent();
  interfaceDebounce = interfaceSuperDebounce;
  euclidParmTimedOut = false;
  lc.setColumn(0, 7, false);                      //delete the 1st euclidean sequence of 1st led matrix
  lc.setColumn(1, 7, false);                      //delete the 1st euclidean sequence of 2nd led matrix
  lc.setColumn(0, 6, false);                      //delete the 2nd euclidean sequence of 1st led matrix
  lc.setColumn(1, 6, false);                      //delete the 2nd euclidean sequence of 2nd led matrix
  lc.setLed((k - 1) / 8, (k - 1) % 8, 7, true);   //update the 0 and 1 rows with euclidean parameters
  lc.setLed((n - 1) / 8, (n - 1) % 8, 6, true);
}


//blink the cursor
void bleenkCursor() {
  if (thisLoop > (bleenkTime + bleenkDelay)) {    //if now if after the blink start time + blink lenght delay
    lc.setLed(joyCursor[0] / 8, joyCursor[0] % 8, 7 - joyCursor[1], bleenkState);
    bleenkState = !bleenkState;
    bleenkTime = thisLoop;
  }
}

void closetriggers() {
  digitalWrite(trigger0Pin, LOW);
  digitalWrite(trigger1Pin, LOW);
  digitalWrite(trigger2Pin, LOW);
  digitalWrite(trigger3Pin, LOW);
  digitalWrite(trigger4Pin, LOW);
  digitalWrite(trigger5Pin, LOW);
  digitalWrite(trigger6Pin, LOW);
  digitalWrite(trigger7Pin, LOW);
  digitalWrite(triggerClockPin, LOW);
  triggersOpenState = false;
}

void firetriggers() {
  triggersOpenState = true;
  digitalWrite(triggerClockPin, true);

  if (matrix[0][sweepColumn]) {
    digitalWrite(trigger0Pin, true);
  }
  if (matrix[1][sweepColumn]) {
    digitalWrite(trigger1Pin, true);
  }
  if (matrix[2][sweepColumn]) {
    digitalWrite(trigger2Pin, true);
  }
  if (matrix[3][sweepColumn]) {
    digitalWrite(trigger3Pin, true);
  }
  if (matrix[4][sweepColumn]) {
    digitalWrite(trigger4Pin, true);
  }
  if (matrix[5][sweepColumn]) {
    digitalWrite(trigger5Pin, true);
  }
  if (matrix[6][sweepColumn]) {
    digitalWrite(trigger6Pin, true);
  }
  if (matrix[7][sweepColumn]) {
    digitalWrite(trigger7Pin, true);
  }
}

//refresh all the 2 led matrix
//this function could be improved changing the setLed to a faster setColumn
void refreshLeds() {
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 16; col++) {
      lc.setLed(col / 8, col % 8, 7 - row, matrix[row][col]);
    }
  }
  lc.setLed(joyCursor[0] / 8, joyCursor[0] % 8, 7 - joyCursor[1], true);
}

void benchMe() {
  if (debug) {
    if (benchTry <= 1000) {
      if ((benchTry % 2) == 0) {
        Serial.print("Elapsed (us): ");
        Serial.println(micros() - benchMicros);
      }
      else {
        benchMicros = micros();
      }
      benchTry++;
    }
  }
}

