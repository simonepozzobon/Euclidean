/*  This is the code for a Step Sequencer with Euclidean Rythm Generator 
 
Creative Commons License

Step Sequencer with Euclidean Rhythm Generator by Gibran Curtiss Salomão is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License. Based on a work at https://github.com/PantalaLabs/Euclidean. Permissions beyond the scope of this license may be available at http://creativecommons.org/licenses/by-sa/3.0/.

    Gibran Curtiss Salomão. CC-BY-SA
*/


//clock source button
void readClockSourceButton() {
  if ( (digitalRead(clockInButtonPin) == HIGH) && (!externalClock) ) {
    //EXTERNAL clock
    externalClock = true;
    oldTickInterval = tickInterval; //save the interval timer to restore if come back to internal clock
    Timer1.stop();
    Timer1.detachInterrupt();
    attachInterrupt(digitalPinToInterrupt(clockInPin), tickInterrupt, RISING);
    tickLast = millis();
    markThisEvent();
    interfaceDebounce = interfaceSuperDebounce;
  }
  else if ( (digitalRead(clockInButtonPin) == LOW) && (externalClock) ) {
    //INTERNAL clock
    externalClock = false;
    detachInterrupt(digitalPinToInterrupt(clockInPin));
    tickInterval = oldTickInterval;
    timerOneInterval = oldTickInterval * 1000;
    Timer1.initialize(timerOneInterval);
    Timer1.attachInterrupt(tickInterrupt);
    Timer1.restart();
    markThisEvent();
    interfaceDebounce = interfaceSuperDebounce;
  }
}

//I was having some problems to update the 2 led matrix when it was necessary to do in the end of the gate time,
//even interrupting all operations long before of the beginning of the nex trigger (ctrl+f "note1")
//sometimes the screens and the arduino crash
//so, I just created the "shiftEuclidean" flag indicating that matrix should be modified as soon as I had time
//then, I can check the flag in the start of next loop or anytime that I know that I will have enough time to do the task.
//
//this same method could be applied to all interface responses
//any interface input flag a variable indicating that some operation must be executed soon
//in the next time gap , we can run the task with safety
//
//this could be improved slicing the time and distributing the tasks
//in this module it wasnt necessary with other tasks but is a good and fun practice
//
void readEncoderShift() {
  long newEncoderShiftPosition = encoderShift.read();
  if (newEncoderShiftPosition != oldEncoderShiftPosition) {   //if there was a change in encoder position
    if ( (newEncoderShiftPosition % 4) == 0 ) {                   //mod 4 changes
      if (oldEncoderShiftPosition < newEncoderShiftPosition) {
        shiftEuclidean = true;
        shiftEuclideanDirection = true;
        shiftMatrixRow(joyCursor[1], true);
      }
      else {
        shiftEuclidean = true;
        shiftEuclideanDirection = false;
        shiftMatrixRow(joyCursor[1], false);
      }
      oldEncoderShiftPosition = newEncoderShiftPosition;        //update old position with  new position
      interfaceDebounce = interfaceSuperDebounce;
      markThisEvent();
    }
  }
}

//read the speed encoder 
void readEncoderSpeed() {
  long newEncoderSpeedPosition = encoderSpeed.read();
  if (newEncoderSpeedPosition != oldEncoderSpeedPosition) {           //if there was a change in encoder position
    encoderSpeedValue = encoderSpeedCenter + newEncoderSpeedPosition; //update old position with  new position
    if (encoderSpeedValue < encoderSpeedMinValue) {
      encoderSpeedValue = encoderSpeedMinValue;
    }
    if (encoderSpeedValue > encoderSpeedMaxValue) {
      encoderSpeedValue = encoderSpeedMaxValue;
    }
    oldEncoderSpeedPosition = newEncoderSpeedPosition;
    tickInterval = encoderSpeedValue;                                 //sets the tickInterval variable with the new calculated interval
    interfaceDebounce = interfaceNormalDebounce;
    markThisEvent();
  }
}

//read the K euclidean encoder
void readEncoderEuclidKParm() {
  long newEncoderKPosition = encoderK.read();
  if (newEncoderKPosition != oldEncoderKPosition) {       //if there was a change in encoder position
    if ( (newEncoderKPosition % 4) == 0 ) {               //mod 4 changes (debounce like to make the interface less sensitive)
      if (newEncoderKPosition < oldEncoderKPosition) {
        if (euclidParm1 - 1 >= 0) {
          euclidParm1--;
        }
      }
      else {
        if (euclidParm1 + 1 <= euclidParm2) {
          euclidParm1++;
        }
      }
      changeEuclidScreen(euclidParm1 , euclidParm2);
      oldEncoderKPosition = newEncoderKPosition;            //update old position with  new position
      interfaceDebounce = interfaceNormalDebounce;
      markThisEvent();
    }
  }
}

void readEncoderEuclidNParm() {
  long newEncoderNPosition = encoderN.read();
  if (newEncoderNPosition != oldEncoderNPosition) {         //if there was a change in encoder position
    if ( (newEncoderNPosition % 4) == 0 ) {                 //mod 4 changes (debounce like to make the interface less sensitive)
      if (newEncoderNPosition < oldEncoderNPosition) {
        if (euclidParm2 + 1 <= 16) {
          euclidParm2++;
        }
      }
      else {
        if (euclidParm2 - 1 >= euclidParm1) {
          euclidParm2--;
        }
      }
      changeEuclidScreen(euclidParm1 , euclidParm2);
      oldEncoderNPosition = newEncoderNPosition;              //update old position with  new position
      interfaceDebounce = interfaceNormalDebounce;
      markThisEvent();
    }
  }
}

//read the joystick and update its coords values
void readJoystick() {
  if (thisLoop > interfaceEvent + 100) { //100 is a special debounce for joystick
    joyOldY = joyCursor[0];
    joyOldX = joyCursor[1];

    int yReading = readAxis(A0);
    int xReading = readAxis(A1);

    if ( (xReading != 0) || (yReading != 0) )   {
      if (xReading != 0) {
        joyCursor[0] = joyCursor[0] + xReading;
        if (joyCursor[0] < 0) {
          joyCursor[0] = 15;
        } else {
          joyCursor[0] = abs(joyCursor[0]) % 16;
        }
      }
      if (yReading != 0)   {
        joyCursor[1] = joyCursor[1] - yReading;
        if (joyCursor[1] < 0) {
          joyCursor[1] = 7;
        } else {
          joyCursor[1] = abs(joyCursor[1]) % 8;
        }
      }
      lc.setLed(joyCursor[0] / 8, joyCursor[0] % 8, 7 - joyCursor[1], true);
      lc.setLed(joyOldY / 8, joyOldY % 8, 7 - joyOldX , matrix[joyOldX][joyOldY]);
      interfaceDebounce = interfaceSuperDebounce;
      markThisEvent();
    }
  }
}

// map the reading from the analog input range to the output range , used on joystick
int readAxis(int thisAxis) {
  int reading = analogRead(thisAxis);
  reading = map(reading, 0, 1023, 0, joyAxisRange);
  if (reading < 3) {
    return -1;
  }
  if (reading > 9) {
    return 1;
  }
  return 0;
}

//play/pause button
void readPauseButton() {
  if (digitalRead(pausePin) == HIGH) {
    noInterrupts();
    while ( digitalRead(pausePin) == HIGH) {}
    markThisEvent();
    interrupts();
  }
}

//kill entire euclidean sequence
void readKillButton() {
  if (digitalRead(removeEuclidPin) == HIGH) {
    killMatrixRow(joyCursor[1]);
    interfaceDebounce = interfaceSuperDebounce;
    markThisEvent();
  }
}

//reset master clock
void readResetButton() {
  if (digitalRead(resetPin) == HIGH) {
    sweepColumn = 0;
    interfaceDebounce = interfaceSuperDebounce;
    markThisEvent();
  }
}

//add/remove a single step
void readAddDotButton() {
  //add justa a DOT on matrix and screen
  if (digitalRead(addDotPin) == HIGH) {
    matrix[joyCursor[1]][joyCursor[0]] = !matrix[joyCursor[1]][joyCursor[0]];
    lc.setLed(joyCursor[0] / 8, joyCursor[0] % 8, 7 - joyCursor[1], matrix[joyCursor[1]][joyCursor[0]]);
    interfaceDebounce = interfaceSuperDebounce;
    markThisEvent();
  }
}

//read the E button
void readNewEuclidButton() {
  if (digitalRead(addEuclidPin) == HIGH) {
    changeEuclidScreen(euclidParm1, euclidParm2);
    interfaceDebounce = interfaceSuperDebounce;
    markThisEvent();
  }
}

void readAddEuclidButton() {
  //add a full euclidean sequence to matrix and screen
  if (digitalRead(addEuclidPin) == HIGH) {
    euclidParmTimedOut = true;                      //euclidean state is off
    calculateESequence(euclidParm1, euclidParm2);   //calculates the euclidean sequence
    copyEtoMatrix(joyCursor[1], E[0]);              //copies to matrix
    for (int row = 0; row < 2; row++) {             //restores just the rows 0 and 1 cleared before
      for (int col = 0; col < 16; col++) {
        lc.setLed(col / 8, col % 8, 7 - row, matrix[row][col]);
      }
    }
    for (int col = 0; col < 16; col++) {            //restores just the row selected to the new euclidean sequence
      lc.setLed(col / 8, col % 8, 7 - joyCursor[1], matrix[joyCursor[1]][col]);
    }
    lc.setLed(joyCursor[0] / 8, joyCursor[0] % 8, 7 - joyCursor[1], true);    //restores the blink led
    interfaceDebounce = interfaceSuperDebounce;
    markThisEvent();
  }
}

//if there was a time out on euclidean state , come back to mais state - restores the full led matrix
void giveUpAddEuclid() {
  markThisEvent();
  euclidParmTimedOut = true;                      //euclidean state is off
  for (int row = 0; row < 2; row++) {             //restores just the rows 0 and 1 cleared before
    for (int col = 0; col < 16; col++) {
      lc.setLed(col / 8, col % 8, 7 - row, matrix[row][col]);
    }
  }
  lc.setLed(joyCursor[0] / 8, joyCursor[0] % 8, 7 - joyCursor[1], true);    //restores the blink led
  interfaceDebounce = interfaceSuperDebounce;
  markThisEvent();
}

//shift the euclidean sequence backward or forward
void shiftMatrixRow(int row, boolean way) {
  boolean spare;
  if (way) {
    spare = matrix[row][15];
    for (int col = 15; col > 0 ; col--) {
      matrix[row][col] = matrix[row][col - 1];
    }
    matrix[row][0] = spare;
  }
  else {
    spare = matrix[row][0];
    for (int col = 0; col < 15; col++) {
      matrix[row][col] = matrix[row][col + 1];
    }
    matrix[row][15] = spare;
  }
  for (int col = 0; col < 16; col++) {
    lc.setLed(col / 8, col % 8, 7 - row, matrix[row][col]);
  }
}

//delete the euclidean sequence from internal array and from led screen
void killMatrixRow(int row) {
  for (int col = 0; col < 16; col++) {
    matrix[row][col] = false;           //delete the euclidean sequence of the matrix
  }
  lc.setColumn(0, 7 - row, false);      //delete the euclidean sequence of 1st led matrix
  lc.setColumn(1, 7 - row, false);      //delete the euclidean sequence of 2nd led matrix
  switch (row) {                        //switch off the trigger
    case 0:
      digitalWrite(trigger0Pin, LOW);
      break;
    case 1:
      digitalWrite(trigger1Pin, LOW);
      break;
    case 2:
      digitalWrite(trigger2Pin, LOW);
      break;
    case 3:
      digitalWrite(trigger3Pin, LOW);
      break;
    case 4:
      digitalWrite(trigger4Pin, LOW);
      break;
    case 5:
      digitalWrite(trigger5Pin, LOW);
      break;
    case 6:
      digitalWrite(trigger6Pin, LOW);
      break;
    case 7:
      digitalWrite(trigger7Pin, LOW);
      break;
  }
}

//this function saves the time that ocured any kind of human interface input
void markThisEvent() {
  interfaceEvent = millis();
}

