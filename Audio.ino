void setCurrentData(int intDataPos, int extDataPos) {
  for (int i = 0; i < numberOfInteriorMotes; i++) {
    int dataIndex = (numberOfInteriorMotes - i + intDataPos) % numberOfInteriorMotes;
    dataForCurrentFrame[i] = interiorMoteData[i][dataIndex];
  }
  for (int i = 0; i < numberOfExteriorMics; i++) {
    dataForCurrentFrame[i + numberOfInteriorMotes] = exteriorMoteData[i + extDataPos];
  }
}

void birthCellsFromAudio(int cells[STRIPS_PER_PANEL][LEDS_PER_STRIP], float levels[], int nodes[][2], int colors[], float scalingFactors[], int nodesLength) {
  for (int y = 0; y < STRIPS_PER_PANEL; y++){
    for (int x = 0; x < LEDS_PER_STRIP; x++){
      float level = 0;
      int nodeRed = 0;
      int nodeGreen = 0;
      int nodeBlue = 0;
      boolean isNode = false;
      for (int i = 0; i < nodesLength; i++){
        float factoredLevel = factorByDistance(i, x, y, levels[i], scalingFactors[i]);
        level += factoredLevel;
        float colorFactor = factoredLevel/255;
        nodeRed += colorFactor * hexToRGB(colors[i], 'r');
        nodeGreen += colorFactor * hexToRGB(colors[i], 'g');
        nodeBlue += colorFactor * hexToRGB(colors[i], 'b');
        if ((x == nodes[i][1]) && (y == nodes[i][0])) {
          isNode = true;
        }
      }
      nodeRed = constrain(nodeRed, 0, 255);
      nodeGreen = constrain(nodeGreen, 0, 255);
      nodeBlue = constrain(nodeBlue, 0, 255);
      if ((random(128) < level) || isNode) {
        cells[y][x] = RGBToHex(nodeRed, nodeGreen, nodeBlue);
      }
    }
  }
}

float factorByDistance(int nodeIndex, int x, int y, float level, float scalingFactor) {
  // Used for the "loudness gradient" circles around each audio node
  // x1, y1 is the source of the sound. x2, y2, is the position at which to return the factored level.
  float decay = 64;
  level *= scalingFactor;
  level = constrain(level, 0, 255);
  float pointDistance = distanceToNode[nodeIndex][y][x];
  float levelByDistance = level - pointDistance * decay;
  levelByDistance = constrain(levelByDistance, 0, 255);
  return levelByDistance;
}

void precomputeDistances() {
  for (int nodeIndex = 0; nodeIndex < numberOfNodes; nodeIndex++) {
    for (int row = 0; row < STRIPS_PER_PANEL; row++){
      for (int col = 0; col < LEDS_PER_STRIP; col++){
        float y1 = float(nodeCoordinates[nodeIndex][0]);
        float x1 = float(nodeCoordinates[nodeIndex][1]);
        float y2 = float(row);
        float x2 = float(col);
        distanceToNode[nodeIndex][row][col] = distance(x1, y1, x2, y2);
      }
    }
  }
}

float distance(float x1, float y1, float x2, float y2)
{
  return sqrt(pow(x2-x1, 2) + pow(y2-y1, 2));
}
