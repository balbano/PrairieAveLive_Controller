void getXBeeDataAndSet(int volumes[])
{
  xbee.readPacket();
  if (xbee.getResponse().isAvailable()) {
    // got something

    if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
      // got a zb rx packet

      // now fill our zb rx class
      xbee.getResponse().getZBRxResponse(rx);

      if (rx.getOption() == ZB_PACKET_ACKNOWLEDGED) {
        // the sender got an ACK
        Serial.println("Packet acknowledged");
      } 
      else {
        // we got it (obviously) but sender didn't get an ACK
        Serial.println("Packet not acknowledged");
      }
      
      int msb = rx.getRemoteAddress64().getMsb();
      int lsb = rx.getRemoteAddress64().getLsb();
      Serial.print("Remote Address MSB: ");
      Serial.println(msb, HEX);
      Serial.print("Remote Address lSB: ");
      Serial.println(lsb, HEX);
      Serial.print("Data: ");
      Serial.println(rx.getData(0));
      
      for (int i = 0; i < NUMBER_OF_NODES; i++) {
        if (msb == nodeAddresses[i][0] && lsb == nodeAddresses[i][1]) {
          volumes[i] = rx.getData(0);
        }
      }

      radioFPS = 1000.0 / (millis() - previousPacketTime);
      Serial.print("Radio FPS: ");
      Serial.println(radioFPS);
      previousPacketTime = millis();
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
    Serial.print("oh no!!! error code:");
    Serial.println(xbee.getResponse().getErrorCode());
  }
  else {
    // Serial.println("No packet available.");
  }
}
