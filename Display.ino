void initializeWithRandomColors(int cells[STRIPS_PER_PANEL][LEDS_PER_STRIP])
{
  for (int y = 0; y < STRIPS_PER_PANEL; y++){
    for(int x = 0; x < LEDS_PER_STRIP; x++){
      if(random(2)){
        int randomColor = random(0xFFFFFF);
        cells[y][x] = randomColor;
      }
      else {
        cells[y][x] = 0;
      }
    }
  }
}

void killAllCells(int cells[STRIPS_PER_PANEL][LEDS_PER_STRIP])
{
  for (int y = 0; y < STRIPS_PER_PANEL; y++){
    for(int x = 0; x < LEDS_PER_STRIP; x++){
      cells[y][x] = 0;
    }
  }
}


void setAllPixels(int cells[STRIPS_PER_PANEL][LEDS_PER_STRIP])
{
  for (int y = 0; y < STRIPS_PER_PANEL; y++){
    for(int x = 0; x < LEDS_PER_STRIP; x++){
      leds.setPixel(y*LEDS_PER_STRIP + x, cells[y][x]);
    }
  }
}
