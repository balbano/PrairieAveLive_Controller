float factorByDistance(int x1, int y1, int x2, int y2, float level, float scalingFactor, float radius) {
  // Used for the "loudness gradient" circles around each audio node
  // x1, y1 is the source of the sound. x2, y2, is the position at which to return the factored level.
  level = level * scalingFactor;
  radius = constrain(radius, 2, 256);
  float pointDistance = distance(float(x1), float(y1), float(x2), float(y2));
  pointDistance = constrain(pointDistance, 0, radius);
  float levelByDistance = map(pointDistance, 0, radius, level, 0);
  levelByDistance = constrain(levelByDistance, 0, level);
  return levelByDistance;
}

void birthCellsFromAudio(int cells[STRIPS_PER_PANEL][LEDS_PER_STRIP], float levels[], int nodes[][2], int colors[], float scalingFactors[], int nodesLength) {
  for (int y = 0; y < STRIPS_PER_PANEL; y++){
    for (int x = 0; x < LEDS_PER_STRIP; x++){
      float level = 0;
      int nodeRed = 0;
      int nodeGreen = 0;
      int nodeBlue = 0;
      for (int i = 0; i < nodesLength; i++){
        level += factorByDistance(nodes[i][1], nodes[i][0], x, y, levels[i], scalingFactors[i], levels[i]/32);
        nodeRed += int(factorByDistance(nodes[i][1], nodes[i][0], x, y, hexToRGB(colors[i], 'r'), 1., levels[i]/32));
        nodeGreen += int(factorByDistance(nodes[i][1], nodes[i][0], x, y, hexToRGB(colors[i], 'g'), 1., levels[i]/32));
        nodeBlue += int(factorByDistance(nodes[i][1], nodes[i][0], x, y, hexToRGB(colors[i], 'b'), 1., levels[i]/32));
      }
      nodeRed = constrain(nodeRed, 0, 255);
      nodeGreen = constrain(nodeGreen, 0, 255);
      nodeBlue = constrain(nodeBlue, 0, 255);
      if (random(100) < level){
        cells[y][x] = RGBToHex(nodeRed, nodeGreen, nodeBlue);
      }
    }
  }
  for (int i = 0; i < nodesLength; i++){
     cells[nodes[i][0]][nodes[i][1]] = colors[i];
  }
}
