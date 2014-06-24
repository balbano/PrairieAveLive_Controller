void getXBeeDataAndSet(int volumes[])
{
  xbee.readPacket();
  if (xbee.getResponse().isAvailable()) {
    numberOfPacketsRead++; // Count the packets read each frame.
    // got something

    if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
      // got a zb rx packet

      // now fill our zb rx class
      xbee.getResponse().getZBRxResponse(rx);

      if (rx.getOption() == ZB_PACKET_ACKNOWLEDGED) {
        // the sender got an ACK
        // Serial.println("Packet acknowledged");
      } 
      else {
        // we got it (obviously) but sender didn't get an ACK
        // Serial.println("Packet not acknowledged");
      }
      
      int msb = rx.getRemoteAddress64().getMsb();
      int lsb = rx.getRemoteAddress64().getLsb();
      /*
      Serial.print("Remote Address MSB: ");
      Serial.println(msb, HEX);
      Serial.print("Remote Address lSB: ");
      Serial.println(lsb, HEX);
      Serial.print("Data: ");
      Serial.println(rx.getData()[0]);
      */
      for (int i = 0; i < numberOfInteriorFios; i++) {
        if (msb == interiorAddresses[i][0] && lsb == interiorAddresses[i][1]) {
          volumes[i] = rx.getData()[0];
        }
      }
      if (msb == exteriorAddress[0] && lsb == exteriorAddress[1]) {
        for (int i = 0; i < numberOfExteriorMics; i++) {
          int j = i + numberOfInteriorFios; // Exterior volumes begin after the interior volumes. Offset 'j' accordingly.
          volumes[j] = rx.getData()[i];
        }
      }

      // radioFPS = 1000.0 / (millis() - previousPacketTime);
      // Serial.print("Radio FPS: ");
      // Serial.println(radioFPS);
      // previousPacketTime = millis();
    }
    else if (xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE) {
      xbee.getResponse().getModemStatusResponse(msr);
      // the local XBee sends this response on certain events, like association/dissociation

      if (msr.getStatus() == ASSOCIATED) {
        // yay this is great.  flash led
        Serial.println("Modem associated");
      } 
      else if (msr.getStatus() == DISASSOCIATED) {
        // this is awful.. flash led to show our discontent
        Serial.println("Modem disassociated");
      } 
      else {
        // another status
        Serial.println("Modem did something unknown");
      }
    } 
    else {
      // not something we were expecting
      Serial.println("Something unexpected happened.");  
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
