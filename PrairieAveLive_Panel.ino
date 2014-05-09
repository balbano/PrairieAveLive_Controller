/*  
  Required Connections (From OCTOWS2811 example)
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

  NOTES
  -----
  Assumes a 4-byte int, such as that on the Teensy3.1
*/

#include <OctoWS2811.h>

// Panel setup.
#define LEDS_PER_STRIP 11
#define STRIPS_PER_PANEL 7

int panel[STRIPS_PER_PANEL][LEDS_PER_STRIP];
int panelBuffer[STRIPS_PER_PANEL][LEDS_PER_STRIP];

// Framerate setup
const float FPS = 2;
const float frameLength = 1000 / FPS;
unsigned long previousFrameTime;

// OCTOWS2811 setup
DMAMEM int displayMemory[LEDS_PER_STRIP*6];
int drawingMemory[LEDS_PER_STRIP*6];

const int config = WS2811_GRB | WS2811_800kHz;
OctoWS2811 leds(LEDS_PER_STRIP, displayMemory, drawingMemory, config);

void setup() {
  leds.begin();
  leds.show();
  randomSeed(analogRead(0));
  initializeWithRandomColors(panel);
  previousFrameTime = millis();
}

void loop() {
  if ((millis() - previousFrameTime) > frameLength){
    previousFrameTime = millis();
    iterateGameOfLife(panel, panelBuffer);
    setPixelsAndShow(panel);
  }
}

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

void setPixelsAndShow(int cells[STRIPS_PER_PANEL][LEDS_PER_STRIP])
{
  for (int y = 0; y < STRIPS_PER_PANEL; y++){
    for(int x = 0; x < LEDS_PER_STRIP; x++){
      leds.setPixel(y*LEDS_PER_STRIP + x, cells[y][x]);
    }
  }
  leds.show();
}

int RGBToHex(int red, int green, int blue)
{
  return (red << 16) + (green << 8) + blue;
}

int hexToRGB(int hexColor, char colorToReturn)
{
  switch (colorToReturn) {
    case 'r':
      return hexColor >> 16;
    case 'g':
      return (hexColor >> 8) & 0xFF;
    case 'b':
      return hexColor & 0xFF;
  }
}
