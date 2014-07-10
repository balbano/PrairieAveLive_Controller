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
// Use define to allow use in function argument definitions.
#define LEDS_PER_STRIP 66
#define STRIPS_PER_PANEL 7

int panel[STRIPS_PER_PANEL][LEDS_PER_STRIP];
int panelBuffer[STRIPS_PER_PANEL][LEDS_PER_STRIP];

// OCTOWS2811 setup
DMAMEM int displayMemory[LEDS_PER_STRIP*6];
int drawingMemory[LEDS_PER_STRIP*6];

const int config = WS2811_GRB | WS2811_800kHz;
OctoWS2811 leds(LEDS_PER_STRIP, displayMemory, drawingMemory, config);

// XBee setup.
XBee xbee = XBee();

//Tx
XBeeAddress64 addr64 = XBeeAddress64(0x13A200, 0x40B79908);
uint8_t payload[] = {255};
ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload)); // Set addr before sending.

ZBTxStatusResponse txStatus = ZBTxStatusResponse();

// Rx
XBeeResponse response = XBeeResponse();
ZBRxResponse rx = ZBRxResponse();
ModemStatusResponse msr = ModemStatusResponse();

// Audio mote setup 
const int numberOfInteriorMotes = 3; // Each interior mote has 1 mic.
const int numberOfExteriorMics = 2; // The single exterior mote has many mics.
const int numberOfNodes = numberOfInteriorMotes + numberOfExteriorMics;

const int moteAddr64Msb = 0x13A200;
const int interiorMoteAddr64Lsbs[] = {0x40ACB022, 0x40AE998C, 0x40B79908};
const int exteriorMoteAddr64Lsb = 0x40ACB3EC;
uint16_t interiorMoteAddr16s[numberOfInteriorMotes];
uint16_t exteriorMoteAddr16;

const int samplesPerInteriorTx = numberOfInteriorMotes*2;
const int samplesPerExteriorTx = numberOfExteriorMics*2;

int interiorMoteData[numberOfInteriorMotes][samplesPerInteriorTx];
int exteriorMoteData[samplesPerExteriorTx];

// Mote data position counters.
int interiorMoteCounter = 0;
unsigned long previousPullTime;

// The node coords for the interior mics match the indexing of the interior
// mote addresses. The node coords of the exterior mics are offset by the
// number of interior motes.
//                           |Interior Motes/Mics           |Exterior Mics       |
//                           |------------------------------|--------------------|
int nodeCoordinates[][2]    = {{2, 4},   {2, 11},  {4, 4},   {2, 21},  {2, 30}};
int nodeColors[]            = {0xFF00FF, 0x00FFFF, 0x00FF00, 0xFFFF00, 0x00FF00};
float audioScalingFactors[] = {2.,       2.,       2.,       2.,       2.};

int dataForCurrentFrame[numberOfNodes];
float levels[numberOfNodes];

// Framerate setup
const float FPS = 12;
const float frameLength = 1000 / FPS;
unsigned long previousFrameTime;

// REPORTING
const int timeBetweenReports = 5000; // Report once per 5 seconds.
unsigned long previousReportTime;
int numberOfPacketsRead = 0;
int numberOfErrors = 0;
int numberOfAttemptsToReadEmptyBuffer = 0;
int numberOfPacketsByMote[numberOfInteriorMotes + 1];

void setup() {
  leds.begin();
  leds.show();
  
  // Start serial
  Serial.begin(57600);
  Serial1.begin(57600);
  xbee.setSerial(Serial1);
  
  Serial.println("Setting up.");

  // Tx setup
  zbTx.setPayload(payload);
  zbTx.setPayloadLength(sizeof(payload));
  zbTx.setFrameId(0); // No Ack.

  // Set defaults.
  for (int i = 0; i < numberOfNodes; i++) {
    dataForCurrentFrame[i] = 0;
    interiorMoteAddr16s[i] = 0xFFFE;
  }

  for (int i = 0; i < numberOfInteriorMotes; i++) {
    for (int j = 0; j < samplesPerInteriorTx; j++) {
      interiorMoteData[i][j] = 0;
    }
  }

  for (int i = 0; i < samplesPerExteriorTx; i++) {
    exteriorMoteData[i] = 0;
  }

  exteriorMoteAddr16 = 0xFFFE;
  
  for (int i = 0; i < numberOfNodes + 1; i++) {
    numberOfPacketsByMote[i] = 0;
  }

  // Timers
  previousFrameTime = millis();
  previousReportTime = millis();
  previousPullTime = millis();
}

void loop() {
  // Alternate pulling data from one of the interior motes and from the
  // exterior mote. Iterate GoL after each pull. Thus, each loop() advances the
  // GoL by 2 frames. The amount of data transmitted should be such that there
  // is new data for each frame. The visualization will be delayed from the
  // time of sampling by the number of sample per Tx times the length of a
  // frame.

  // Pull data from interior motes.
  addr64 = XBeeAddress64(moteAddr64Msb, interiorMoteAddr64Lsbs[interiorMoteCounter]);
  zbTx.setAddress64(addr64);

//  Serial.print("Sending ");
//  Serial.print(zbTx.getPayload()[0]);
//  Serial.print(" to: ");
//  Serial.print(zbTx.getAddress64().getMsb(), HEX);
//  Serial.print(", ");
//  Serial.println(zbTx.getAddress64().getLsb(), HEX);

  xbee.send(zbTx);
  lookForData();

  // Set the data for the current frame and iterate.
  int interiorMoteDataPosition = interiorMoteCounter * 2; // There are twice as many samples as motes.
  int exteriorMoteDataPosition = numberOfExteriorMics; // Use the second set of data.

  // Serial.print("interiorMoteDataPosition: ");
  // Serial.print(interiorMoteDataPosition);
  // Serial.print(", exteriorMoteDataPosition: ");
  // Serial.println(exteriorMoteDataPosition);

  setCurrentData(interiorMoteDataPosition, exteriorMoteDataPosition); 
  advanceFrame();

  // Pull data from exterior mote.
  addr64 = XBeeAddress64(moteAddr64Msb, exteriorMoteAddr64Lsb);
  zbTx.setAddress64(addr64);

//  Serial.print("Sending ");
//  Serial.print(zbTx.getPayload()[0]);
//  Serial.print(" to: ");
//  Serial.print(zbTx.getAddress64().getMsb(), HEX);
//  Serial.print(", ");
//  Serial.println(zbTx.getAddress64().getLsb(), HEX);

  xbee.send(zbTx);
  lookForData();

  // Set the data for the current frame and iterate.
  interiorMoteDataPosition++; // Use the next sample.
  exteriorMoteDataPosition = 0; // Use the first set of data.
  
  // Serial.print("interiorMoteDataPosition: ");
  // Serial.print(interiorMoteDataPosition);
  // Serial.print(", exteriorMoteDataPosition: ");
  // Serial.println(exteriorMoteDataPosition);

  setCurrentData(interiorMoteDataPosition, exteriorMoteDataPosition); 
  advanceFrame();

  // Update the position counters.
  interiorMoteCounter = (interiorMoteCounter + 1) % numberOfInteriorMotes;
  
  if (millis() - previousReportTime > timeBetweenReports) {
    printReport();
  }
}

void advanceFrame() {
  previousFrameTime = millis();
      
  iterateGameOfLife(panel, panelBuffer);

  // Serial.print("Levels: ");

  for (int i = 0; i < numberOfNodes; i++) {
    levels[i] = float(dataForCurrentFrame[i]);
    // Serial.print(levels[i]);
    // Serial.print(", ");
  }

  // Serial.println();
  // Serial.println();

  birthCellsFromAudio(panel, levels, nodeCoordinates, nodeColors,
                      audioScalingFactors, numberOfNodes);

  setAllPixels(panel);
  leds.show();

  for (int i = 0; i < numberOfNodes; i++) {
    dataForCurrentFrame[i] = 0;
  }
}

void printReport() {
  previousReportTime = millis();
  Serial.print("Packets read: ");
  Serial.print(numberOfPacketsRead);
  Serial.print("; Errors: ");
  Serial.print(numberOfErrors);
  Serial.print("; Attempts to read empty buffer: ");
  Serial.println(numberOfAttemptsToReadEmptyBuffer);

  for(int i = 0; i < numberOfInteriorMotes; i++){
    Serial.print(interiorMoteAddr64Lsbs[i], HEX);
    Serial.print(": ");
    Serial.println(numberOfPacketsByMote[i]);
  }

  Serial.print(exteriorMoteAddr64Lsb, HEX);
  Serial.print(": ");
  Serial.println(numberOfPacketsByMote[numberOfInteriorMotes]);
  Serial.println();
  
  numberOfPacketsRead = 0;
  numberOfErrors = 0;
  numberOfAttemptsToReadEmptyBuffer = 0;
  for (int i = 0; i < numberOfNodes + 1; i++) {
    numberOfPacketsByMote[i] = 0;
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
