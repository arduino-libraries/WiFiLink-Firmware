#include "CommLgc.h"
#include "utility/wifi_utils.h"
//#include "CommItf.h"

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// char ssid[] = "****";     //  your network SSID (name)
// char pass[] = "****";
int ledState = LOW;             // ledState used to set the LED
long previousMillis = 0;        // will store last time LED was updated
long interval = 100;           // interval at which to blink (milliseconds)

void setup() {

  // WiFi.mode(WIFI_STA);
  // WiFi.begin(ssid, pass);
  // while (WiFi.waitForConnectResult() != WL_CONNECTED) {
  //   delay(5000);
  //   ESP.restart();
  // }
	// delay(8000);
  ArduinoOTA.begin();
  //pinMode(5,OUTPUT);
  //delay(300);
  //CommunicationInterface.begin();
  CommunicationLogic.begin();
  //while(!CommunicationInterface.begin());
}

void loop() {
  ArduinoOTA.handle();
  //unsigned long currentMillis = millis();
  // if(WiFi.status() == WL_CONNECTED){
  //   if(currentMillis - previousMillis > interval) {
  // // save the last time you blinked the LED
  //     previousMillis = currentMillis;
  //
  //     // if the LED is off turn it on and vice-versa:
  //     if (ledState == LOW)
  //       ledState = HIGH;
  //     else
  //       ledState = LOW;
  //
  //     // set the LED with the ledState of the variable:
  //     digitalWrite(2, ledState);
  //   }
  // }else
  //   digitalWrite(2, LOW);

//  Serial.print("SSID");
//  Serial.println(WiFi.SSID());
  //Serial.println("Prova");
  //CommunicationLogic.handle();

  //String a = CommunicationInterface.read();
  //if(a!="")
   // CommunicationInterface.write("prova");
  //Serial.println("Prova");

}
