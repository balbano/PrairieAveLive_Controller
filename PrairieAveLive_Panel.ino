/*  
  Required Connections
  --------------------
    pin 2:  LED Strip #1    OctoWS2811 drives 8 LED Strips.
    pin 14: LED strip #2    All 8 are the same length.
    pin 7:  LED strip #3
    pin 8:  LED strip #4    A 100 ohm resistor should used
    pin 6:  LED strip #5    between each Teensy pin and the
    pin 20: LED strip #6    wire to the LED strip, to minimize
    pin 21: LED strip #7    high frequency ringining & noise.
    pin 5:  LED strip #8
    pin 15 & 16 - Connect together, but do not use
    pin 4 - Do not use
    pin 3 - Do not use as PWM.  Normal use is ok.

  This test is useful for checking if your LED strips work, and which
  color config (WS2811_RGB, WS2811_GRB, etc) they require.
*/

#include <OctoWS2811.h>

#define LEDS_PER_STRIP 11
#define STRIPS_PER_PANEL 7

long panel[STRIPS_PER_PANEL][LEDS_PER_STRIP];
long panelBuffer[STRIPS_PER_PANEL][LEDS_PER_STRIP];

DMAMEM int displayMemory[LEDS_PER_STRIP*6];
int drawingMemory[LEDS_PER_STRIP*6];

const int config = WS2811_GRB | WS2811_800kHz;

OctoWS2811 leds(LEDS_PER_STRIP, displayMemory, drawingMemory, config);

void setup() {
  leds.begin();
  leds.show();
  initializeWithRandomColors(panel);
}

void loop() {
  iterateGameOfLife(panel, panelBuffer);
  setPixelsAndShow(panel);
  delay(500);
}

void initializeWithRandomColors(long cells[STRIPS_PER_PANEL][LEDS_PER_STRIP])
{
  for (int y = 0; y < STRIPS_PER_PANEL; y++){
    for(int x = 0; x < LEDS_PER_STRIP; x++){
      if(random(2)){
        long randomColor = random(0xFFFFFF);
        cells[y][x] = randomColor;
      }
      else {
        cells[y][x] = 0;
      }
    }
  }
}

void iterateGameOfLife(long cells[STRIPS_PER_PANEL][LEDS_PER_STRIP], long cellsBuffer[STRIPS_PER_PANEL][LEDS_PER_STRIP])
{
  // Load the current array of cells into the buffer.
  for (int y = 0; y < STRIPS_PER_PANEL; y++){
    for(int x = 0; x < LEDS_PER_STRIP; x++){
      cellsBuffer[y][x] = cells[y][x];
    }
  }
  
  // Breed and kill the cells
  for (int y = 0; y < STRIPS_PER_PANEL; y++){
    for(int x = 0; x < LEDS_PER_STRIP; x++){
      int neighbors = 0;
      long neighbors_red_sum = 0;
      long neighbors_green_sum = 0;
      long neighbors_blue_sum = 0;
      // Count the neighbors
      for (int yy = y-1; yy <= y+1; yy++) {
        for (int xx = x-1; xx <= x+1; xx++) {
          if ( ((xx >= 0)&&(xx < LEDS_PER_STRIP)) && ((yy >= 0)&&(yy < STRIPS_PER_PANEL)) ) {
            if (!((xx == x) && (yy == y))) {
              if (cellsBuffer[yy][xx]) {
                neighbors++;
                // Extract the individual colors from the 3-byte hex color.
                int neighbor_red = (cellsBuffer[yy][xx] >> 16) & 0xFF;
                int neighbor_green = (cellsBuffer[yy][xx] >> 8) & 0xFF;
                int neighbor_blue = cellsBuffer[yy][xx] & 0xFF;
                neighbors_red_sum += neighbor_red;
                neighbors_green_sum += neighbor_green;
                neighbors_blue_sum += neighbor_blue;
              }
            }
          }
        }
      }
     // Birth and death!
      if (cellsBuffer[y][x]) {
        if ((neighbors < 2) || (neighbors > 3)) {
          cells[y][x] = 0;
        }
      }
      else {
        if (neighbors == 3) {
          int red = neighbors_red_sum / neighbors;
          int green = neighbors_green_sum / neighbors;
          int blue = neighbors_blue_sum / neighbors;
          long color = (red << 16) + (green << 8) + blue;
          cells[y][x] = color;
        }
      } 
    }
  }  
}

void setPixelsAndShow(long cells[STRIPS_PER_PANEL][LEDS_PER_STRIP])
{
  for (int y = 0; y < STRIPS_PER_PANEL; y++){
    for(int x = 0; x < LEDS_PER_STRIP; x++){
      leds.setPixel(y*LEDS_PER_STRIP + x, cells[y][x]);
    }
  }
  leds.show();
}
