#include <SPI.h>
#include <WiFiNINA.h>
#include "ThingSpeak.h"

int buzzer = 8;

char ssid[] = "VM6828066";
char pass[] = "t7hmJxyBddvv";

int status = WL_IDLE_STATUS;

unsigned long channelNumber = 1275216;
const char* readAPIKey = "V2B0B27WLFZU1PW1";
const char* writeAPIKey = "SUKVUEUI83YKUZME";

WiFiClient client;

void setup () {
  Serial.begin(9600);
  SPI.begin();
  pinMode(buzzer, OUTPUT);

  Serial.println("\nAttempting to connect to WPA network...");
  status = WiFi.begin(ssid, pass);
  
  if (status != WL_CONNECTED) {
    Serial.print("Could not connect to WPA network...");
    delay(500);
  } else {
    Serial.println();
    Serial.println("Connected, IP address: ");
    Serial.print(WiFi.localIP());
    Serial.println();
  }
  Serial.println("\nAttempting to connect to ThingSpeak...");
  ThingSpeak.begin(client);
  if (ThingSpeak.getLastReadStatus())
    Serial.println("Connected.");
  else {
    Serial.println("Not connected.");
    return;
  }
}

void loop () {

  String NUID = ThingSpeak.readStringField(channelNumber, 1, readAPIKey);
  int Status = ThingSpeak.readIntField(channelNumber, 2, readAPIKey);

  Serial.println("NUID: ");
  Serial.println(NUID);
  Serial.println("Status: ");
  Serial.println(Status);
  
  if (Status == 0)
  {
    int alarm = true;
    while (alarm) {
      for (byte j = 0; j < 5; j++) {
        for (byte i = 0; i <100; i++) {
          digitalWrite(buzzer, HIGH);
          delay(1) ;
          digitalWrite(buzzer, LOW);
          delay(1) ;
        }
        for (byte i = 0; i <100; i++) {
          digitalWrite(buzzer, HIGH);
          delay(2) ;
          digitalWrite(buzzer, LOW);
          delay(2) ;
        }
      }
      alarm = false;
    }
  } else if (Status == 1) {
    for (byte i = 0; i <100; i++) {
      digitalWrite(buzzer, HIGH);
      delay(1) ;
      digitalWrite(buzzer, LOW);
      delay(1) ;
    }
    delay(5000);
  } else {
    delay(5000);
    return;
  }

  ThingSpeak.setField(1, NUID);
  ThingSpeak.setField(2, 2);
  
  int httpCode = ThingSpeak.writeFields(channelNumber, writeAPIKey);

  if (httpCode == 200)
    Serial.println("Channel write successful");
  else
    Serial.println("Problem writing to channel. Http error code " + String(httpCode));
  
}
