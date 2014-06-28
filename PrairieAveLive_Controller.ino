/*
  Prairie Avenue Live - Controller
  ================================
  
  Terminology
  -----------
  
  - Mote: Arduino Fio and XBee collects sound levels from mic and transmits to controller.
    - There are multiple interior motes with one mic each and one exterior mote with multiple mics.
  - Mic: the electret mics used by the motes.
  - Controller: Teensy 3.1 and XBee that receive the sound data from the motes and control the LEDs.
  - Node: the origin point (on the LED array) of the visualization for a particular mic.
  
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
#include <XBee.h>

// Panel setup.
// Use define to allow use in array initialization.
#define LEDS_PER_STRIP 66
#define STRIPS_PER_PANEL 7

int panel[STRIPS_PER_PANEL][LEDS_PER_STRIP];
int panelBuffer[STRIPS_PER_PANEL][LEDS_PER_STRIP];

// Audio node setup 
const int numberOfInteriorMotes = 2; // Each interior mote has 1 mic.
const int numberOfExteriorMics = 2; // The single exterior mote has multiple mics.
const int numberOfNodes = numberOfInteriorMotes + numberOfExteriorMics;

int interiorMoteAddresses[][2] = {{0x13A200, 0x40ACB022}, {0x13A200, 0x40AE998C}};
int exteriorMoteAddress[] = {0x13A200, 0x40ACB3EC};

// The node coords for the interior mics match the indexing of the interior mote
// addresses. The node coords of the exterior mics are offset by the number of 
// interior motes.
//                           |Interior Motes/Mics |Exterior Mics       |
//                           |--------------------|--------------------|
int nodeCoordinates[][2]    = {{2, 4},   {2, 11},  {2, 21},  {2, 30}};
int nodeColors[]            = {0xFF00FF, 0x00FFFF, 0xFFFF00, 0x00FF00};
float audioScalingFactors[] = {2.,       2.,       2.,       2.};

int maxVolumes[numberOfNodes];
float levels[numberOfNodes];

// Framerate setup
const float FPS = 12;
const float frameLength = 1000 / FPS;
unsigned long previousFrameTime;
// float radioFPS;
// unsigned long previousPacketTime;


// OCTOWS2811 setup
DMAMEM int displayMemory[LEDS_PER_STRIP*6];
int drawingMemory[LEDS_PER_STRIP*6];

const int config = WS2811_GRB | WS2811_800kHz;
OctoWS2811 leds(LEDS_PER_STRIP, displayMemory, drawingMemory, config);

// XBee setup.
XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
// create reusable response objects for responses we expect to handle 
ZBRxResponse rx = ZBRxResponse();
ModemStatusResponse msr = ModemStatusResponse();

// REPORTING
const int timeBetweenReports = 1000; // Report once per second.
unsigned long prevReportTime;
int numberOfPacketsRead = 0;
int numberOfErrors = 0;
int numberOfAttemptsToReadEmptyBuffer = 0;

void setup() {
  leds.begin();
  leds.show();
  
  // start serial
  Serial.begin(9600);
  Serial1.begin(9600);
  xbee.setSerial(Serial1);
  
  //randomSeed(analogRead(0));
  //initializeWithRandomColors(panel);
  
  previousFrameTime = millis();
  prevReportTime = millis();
  
  // previousPacketTime = millis();
  
  for (int i = 0; i < numberOfNodes; i++) {
    maxVolumes[i] = 0;
  }
}

void loop() {
  getXBeeDataAndSet(maxVolumes);
  // Serial.print("Max volume:");
  // Serial.println(maxVolume);
  if ((millis() - previousFrameTime) > frameLength){
    previousFrameTime = millis();
        
    iterateGameOfLife(panel, panelBuffer);
    for (int i = 0; i < numberOfNodes; i++) {
      levels[i] = float(maxVolumes[i]);
    }
    // killAllCells(panel);
    birthCellsFromAudio(panel, levels, nodeCoordinates, nodeColors, audioScalingFactors, numberOfNodes);
    setAllPixels(panel);
    leds.show();
    for (int i = 0; i < numberOfNodes; i++) {
      maxVolumes[i] = 0;
    }
    // Serial.println("GoL iterated!");
  }
  
  if (millis() - prevReportTime > timeBetweenReports) {
    prevReportTime = millis();
    Serial.print("Packets read: ");
    Serial.print(numberOfPacketsRead);
    Serial.print("; Errors: ");
    Serial.print(numberOfErrors);
    Serial.print("; Attempts to read empty buffer: ");
    Serial.println(numberOfAttemptsToReadEmptyBuffer);
    numberOfPacketsRead = 0;
    numberOfErrors = 0;
    numberOfAttemptsToReadEmptyBuffer = 0;
  }
}
