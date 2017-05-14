/*Step Sequencer with Euclidean Rythm Generator

  Creative Commons License

  Step Sequencer with Euclidean Rhythm Generator by Pantala Labs
  is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
  Based on a work at https://github.com/PantalaLabs/Euclidean.

  Gibran Curtiss Salomão. FEB/2017 - CC-BY-NC-SA
*/

#include <EEPROM.h>

long startSaveTime = 0L;

void saveStateStep() {
  Serial.println("Saving patterns:");
  startSaveTime = millis();

  int rowValue;
  int addr = 0;
  for (int row = 0; row < 8; row++) {
    rowValue = convertMatrixRowToInteger(row, 7);
    EEPROM.write(addr, rowValue);
    addr = addr + 1;
    Serial.println(rowValue);
  }

  for (int row = 0; row < 8; row++) {
    rowValue = convertMatrixRowToInteger(row, 15);
    EEPROM.write(addr, rowValue);
    addr = addr + 1;
    Serial.println(rowValue);
  }
  Serial.print("Elapsed:");
  Serial.print(millis() - startSaveTime);
  Serial.println("ms");
  Serial.println("Done!");
}


int loadSavedState() {

  int addr = 0;

  Serial.println("Loading last patterns:");
  startSaveTime = millis();
  int rowValue = 0;
  for (int row = 0; row < 8; row++) {
    rowValue = EEPROM.read(addr);
    addr = addr + 1;
    for (int col = 7; col >= 0; col--) {
      if (rowValue & (1 << col)) {
        matrix[row][7-col] = true;
      }
    }
    Serial.println(rowValue);
  }

  for (int row = 0; row < 8; row++) {
    rowValue = EEPROM.read(addr);
    addr = addr + 1;
    for (int col = 7; col >= 0 ; col--) {
      if (rowValue & (1 << col)) {
        matrix[row][(7-col)+8] = true;
      }
    }
    Serial.println(rowValue);
  }
  Serial.print("Elapsed:");
  Serial.println(millis() - startSaveTime);
  Serial.println("ms");
  Serial.println("Done!");
}

