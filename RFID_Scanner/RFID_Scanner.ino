#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include "ThingSpeak.h"

#define RST_PIN D3
#define SS_PIN  D4

char ssid[] = "VM6828066";
char pass[] = "t7hmJxyBddvv";

int status = WL_IDLE_STATUS;

unsigned long channelNumber = 1275216;
const char* writeAPIKey = "SUKVUEUI83YKUZME";

WiFiClient client;

MFRC522 rfid(SS_PIN, RST_PIN);

MFRC522::MIFARE_Key key; 

byte nuidPICC[4];

boolean Status = false;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  delay(100);

  WiFi.mode(WIFI_STA);

  Serial.println("\nAttempting to connect to WPA network...");
  WiFi.begin(ssid, pass);

  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("Connected, IP address: ");
  Serial.print(WiFi.localIP());
  Serial.println();

  ThingSpeak.begin(client);

  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);


  // Define granted 1821169043 (Card)
  nuidPICC[0] = 182;
  nuidPICC[1] = 116;
  nuidPICC[2] = 90;
  nuidPICC[3] = 43;
}
 
void loop() {

  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  if ( ! rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  if (rfid.uid.uidByte[0] != nuidPICC[0] || 
    rfid.uid.uidByte[1] != nuidPICC[1] || 
    rfid.uid.uidByte[2] != nuidPICC[2] || 
    rfid.uid.uidByte[3] != nuidPICC[3] ) {
    Status = false;
    Serial.println(F("Access Denied. Card not recognised."));
  } else {
    Status = true;
    Serial.println(F("Access Granted. Card was recognised."));
  }

  String NUID = "";
  Serial.println("ID:");
  for (byte i = 0; i < 4; i++) {
    //nuidPICC[i] = rfid.uid.uidByte[i];
    Serial.println(rfid.uid.uidByte[i]);
    NUID = NUID + String(rfid.uid.uidByte[i]);
  }
 
  Serial.println(F("The NUID tag is:"));
  Serial.print(F("In hex: "));
  printHex(rfid.uid.uidByte, rfid.uid.size);
  Serial.println();
  
  rfid.PICC_HaltA();

  rfid.PCD_StopCrypto1();

  Serial.print("Time: ");
  unsigned long time = millis();
  Serial.println(time);

  
  Serial.println("Writing to ThingSpeak");

  ThingSpeak.setField(1, NUID);
  ThingSpeak.setField(2, Status);

  int httpCode = ThingSpeak.writeFields(channelNumber, writeAPIKey);

  if (httpCode == 200)
    Serial.println("Channel write successful");
  else
    Serial.println("Problem writing to channel. Http error code " + String(httpCode));
}


void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}
