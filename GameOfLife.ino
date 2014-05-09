
void iterateGameOfLife(int cells[STRIPS_PER_PANEL][LEDS_PER_STRIP], int cellsBuffer[STRIPS_PER_PANEL][LEDS_PER_STRIP])
{
  // Load the current array of cells into the buffer.
  memcpy(&cellsBuffer, &cells, sizeof(cells));
  
  // Breed and kill the cells
  for (int y = 0; y < STRIPS_PER_PANEL; y++){
    for(int x = 0; x < LEDS_PER_STRIP; x++){
      int neighbors = 0;
      int neighborsRedSum = 0;
      int neighborsGreenSum = 0;
      int neighborsBlueSum = 0;
      
      // Iterate through the adjacent cells, count the living neighbors, and sum their r, g and b values.
      for (int yy = y-1; yy <= y+1; yy++) {
        for (int xx = x-1; xx <= x+1; xx++) {
          if (isValidNeighbor(x, y, xx, yy) && (cellsBuffer[yy][xx] != 0)) { // Live cells have a non-zero color.
                neighbors++;
                neighborsRedSum += hexToRGB(cellsBuffer[yy][xx], 'r');
                neighborsGreenSum += hexToRGB(cellsBuffer[yy][xx], 'g');
                neighborsBlueSum += hexToRGB(cellsBuffer[yy][xx], 'b');
              }
            }
          }
      
      // Kill underpopulated and overcrowded cells.
      if (cellsBuffer[y][x]) {
        if ((neighbors < 2) || (neighbors > 3)) {
          cells[y][x] = 0;
        }
      }
      // Birth cells with 3 neighbors. Make them the average of their neighbors colors.
      else {
        if (neighbors == 3) {
          int avgRed = neighborsRedSum / neighbors;
          int avgGreen = neighborsGreenSum / neighbors;
          int avgBlue = neighborsBlueSum / neighbors;
          cells[y][x] = RGBToHex(avgRed, avgGreen, avgBlue);
        }
      }
      
    }
  }  
}

boolean isValidNeighbor(int x, int y, int xx, int yy)
{
  if ( ( ((xx >= 0)&&(xx < LEDS_PER_STRIP)) && ((yy >= 0)&&(yy < STRIPS_PER_PANEL)) ) // Ignore cells beyond edges.
     && (!((xx == x) && (yy == y))) )// Cells are not their own neighbors.
  {
    return true;
  }
  else {
    return false;
  }
}
