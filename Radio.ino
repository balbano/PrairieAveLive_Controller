void lookForData() {
  while (millis() - previousPullTime < frameLength) {
    getData();
  } 

  previousPullTime = millis();
}

void getData()
{ 
  xbee.readPacket();
  if (xbee.getResponse().isAvailable()) {
    numberOfPacketsRead++; // Count the response packets read each frame.

    if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
      // got a zb rx packet

      // now fill our zb rx class
      xbee.getResponse().getZBRxResponse(rx);

      int msb = rx.getRemoteAddress64().getMsb();
      int lsb = rx.getRemoteAddress64().getLsb();

      // XXX: getData() and getDataLength() break eveything... no idea why. Use
      // getDataOffset(), getFrameDataLength() and getFrameData() instead.
      uint8_t frameLength = rx.getFrameDataLength();
      uint8_t dataOffset = rx.getDataOffset();
      uint8_t dataLength = frameLength - dataOffset;

      /*
      Serial.print("Remote Address MSB: ");
      Serial.println(msb, HEX);
      Serial.print("Remote Address lSB: ");
      Serial.println(lsb, HEX);
      Serial.print("Frame data length: ");
      Serial.println(frameLength);
      Serial.print("Data offset: ");
      Serial.println(dataOffset);
      Serial.print("Data: ");
      
      for (int i = 0; i < dataLength; i++) {
        Serial.print(rx.getFrameData()[i + dataOffset]);
        Serial.print(", ");
      }

      Serial.println();
      Serial.println();
      */

      // Check to see if the address corresponds to an interior mote and if the
      // data is the correct length.
      for (int moteIndex = 0; moteIndex < numberOfInteriorMotes; moteIndex++) {
        if ((msb == moteAddr64Msb && lsb == interiorMoteAddr64Lsbs[moteIndex]) && dataLength == (samplesPerInteriorTx)) {
          for (int i = 0; i < dataLength; i++) {
            interiorMoteData[moteIndex][i] = rx.getFrameData()[i + dataOffset];
          }
          numberOfPacketsByMote[moteIndex]++;
        }
      }

      // Check to see if the address corresponds to the exterior mote and if
      // the data is the correct length.
      if ((msb == moteAddr64Msb  && lsb == exteriorMoteAddr64Lsb) && dataLength == (samplesPerExteriorTx)) {
        for (int i = 0; i < dataLength; i++) {
          exteriorMoteData[i] = rx.getFrameData()[i + dataOffset];
        }
        numberOfPacketsByMote[numberOfInteriorMotes]++;
      }
    }
  } 
  else if (xbee.getResponse().isError()) {
    numberOfErrors++;
    /*
    Serial.print("oh no!!! error code:");
    Serial.println(xbee.getResponse().getErrorCode());
    */
  }
  else {
    numberOfAttemptsToReadEmptyBuffer++;
    // Serial.println("No packet available.");
  }
}
