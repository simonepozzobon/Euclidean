/*Step Sequencer with Euclidean Rythm Generator 
 
Creative Commons License

Step Sequencer with Euclidean Rhythm Generator by Pantala Labs 
is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
Based on a work at https://github.com/PantalaLabs/Euclidean.

Gibran Curtiss Salomão. FEB/2017 - CC-BY-NC-SA
*/

//calculate the euclidean sequence using K and N parameters
void calculateESequence(int k, int n) {
  int progression = 1;
  int thisTokenId;

  for (int i = 1; i <= n ; i++) {
    if (i <= k) {
      E[i] = "1";
    }
    else {
      E[i] = "0";
    }
  }

  thisTokenId = lastToken(n);
  while (thisTokenId > 1) {
    E[0] = E[thisTokenId];
    for (int i = thisTokenId ; i > 1 ; i--) {
      if (E[i] == E[0] && E[progression] != "0" && progression < i ) {
        E[progression] = E[progression] + E[i];
        E[i] = "";
        progression = progression + 1;
      }
      else {
        break;
      }
    }
    progression = 1;
    thisTokenId = lastToken(n);
  }
  E[0] = "";
  for (int i = 1; i <= n ; i++) {
    E[0] = E[0] + E[i];
  }
  progression = 0;
  for (int i = n + 1; i < matrixDim1  ; i++) {
    E[0] = E[0] + E[0].substring(progression, progression + 1);
    progression++;
  }

}

//part of euclidean function.
//find the position of the last token "n" in E
int lastToken(int n) {
  for (int i = n ; i > 2 ; i--) {
    if (E[i] != "") {
      return i;
    }
  }
  return 0;
}


//copy the euclidean sequence to matrix ROW
void copyEtoMatrix(int row, String sequence) {
  for (int col = 0; col < 16; col++) {
    if (sequence.substring(col, col + 1) == "1") {
      matrix[row][col] = true;
    }
    else {
      matrix[row][col] = false;
    }
  }
}

