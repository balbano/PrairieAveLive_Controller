void birthCellsFromAudio(int cells[STRIPS_PER_PANEL][LEDS_PER_STRIP], float levels[], int nodes[][2], int colors[], float scalingFactors[], int nodesLength) {
  for (int y = 0; y < STRIPS_PER_PANEL; y++){
    for (int x = 0; x < LEDS_PER_STRIP; x++){
      float level = 0;
      int nodeRed = 0;
      int nodeGreen = 0;
      int nodeBlue = 0;
      boolean isNode = false;
      for (int i = 0; i < nodesLength; i++){
        float factoredLevel = factorByDistance(nodes[i][1], nodes[i][0], x, y, levels[i], scalingFactors[i]);
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

float factorByDistance(int x1, int y1, int x2, int y2, float level, float scalingFactor) {
  // Used for the "loudness gradient" circles around each audio node
  // x1, y1 is the source of the sound. x2, y2, is the position at which to return the factored level.
  float decay = 64;
  level *= scalingFactor;
  level = constrain(level, 0, 255);
  float pointDistance = distance(float(x1), float(y1), float(x2), float(y2));
  float levelByDistance = level - pointDistance * decay;
  levelByDistance = constrain(levelByDistance, 0, 255);
  return levelByDistance;
}

float distance(float x1, float y1, float x2, float y2)
{
  return sqrt(pow(x2-x1, 2) + pow(y2-y1, 2));
}
