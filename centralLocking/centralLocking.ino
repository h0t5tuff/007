#include <Adafruit_Fingerprint.h>
#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial mySerial(4, 5); //ESP8266 use (4 , 5)
#else 
#define mySerial Serial1 // (green , white)
#endif

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
int rFPS= 4; int rFPSStatus= 0;
int rGn= 3;
int rSw= 2;
int rGnLine= 1;

void setup() {
  pinMode(rFPS, OUTPUT); digitalWrite(rFPS, LOW); 
  pinMode(rGn, OUTPUT); digitalWrite(rGn, LOW);
  pinMode(rSw, OUTPUT); digitalWrite(rSw, LOW);
  pinMode(rGnLine, OUTPUT);  digitalWrite(rGnLine, LOW);
  Serial1.begin(9600); //seeeduino xiao
  //Serial.begin(115200); Serial2.begin(9600, SERIAL_8N1, 16, 17); //esp32 (rx,tx) 
  while (!Serial); delay(100);  //or {;} instead of delay
  Serial.println("\n\nAdafruit finger detect test");
  finger.begin(57600); delay(5);
  if (finger.verifyPassword()) { 
    Serial.println("found sensor :)"); 
  }
  else { 
    Serial.println("shit happens :("); 
    while (1) { 
      delay(1);
    } 
  }
  Serial.println(F("identifying...")); //F() macro moves this stuff into FLASH instead of sRAM
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX); //bla bla bla
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);
  finger.getTemplateCount();
  if (finger.templateCount == 0) { 
    Serial.print("no fingers, run (enroll) code"); 
  }
  else { 
    Serial.print(" We got "); Serial.print(finger.templateCount); Serial.println(" prints on sensor"); 
  } 
}

void loop() {
  rFPSStatus = digitalRead(rFPS);
  getFingerprintID(); 
  delay(50); // no need for full speed
}

uint8_t getFingerprintID() {    //  byte = uint8_t = unsigned char  &  int8_t = char  
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK: Serial.println("Image taken"); break;
    case FINGERPRINT_NOFINGER: return p;
    case FINGERPRINT_PACKETRECIEVEERR: Serial.println("Communication error"); return p;
    case FINGERPRINT_IMAGEFAIL: Serial.println("Imaging error"); return p;
    default: Serial.println("Unknown error"); return p; 
  } 
  // ok success
  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK: Serial.println("Image converted"); break;
    case FINGERPRINT_IMAGEMESS: Serial.println("Image too messy"); return p;
    case FINGERPRINT_PACKETRECIEVEERR: Serial.println("Communication error"); return p;
    case FINGERPRINT_FEATUREFAIL: Serial.println("Could not find fingerprint features"); return p;
    case FINGERPRINT_INVALIDIMAGE: Serial.println("Could not find fingerprint features"); return p;
    default: Serial.println("Unknown error"); return p; 
  } 
  // ok converted
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) { 
    Serial.println("Found a print match!"); 
  } 
  else if (p == FINGERPRINT_PACKETRECIEVEERR) { 
    Serial.println("Communication error"); 
    finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 3); //(control, speed, coloridx, count); // try again, this time use your finger!
    return p; 
  } 
  else if (p == FINGERPRINT_NOTFOUND) { 
    Serial.println("no match"); 
    finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 3); // Let's put on a light show for the Thieves
    return p; 
  } 
  else { 
    Serial.println("Unknown error"); 
    finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 3); // the univerese , god, buddha, etc.. has intervined. (OMG particle)
    return p; 
  } 
  // found a match
  Serial.print("Found ID "); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  finger.LEDcontrol(FINGERPRINT_LED_BREATHING, 50, FINGERPRINT_LED_BLUE, 1); //Blue means good??! I guess..
  digitalWrite(rGn, HIGH); digitalWrite(rSw, HIGH); digitalWrite(rGnLine, HIGH); Serial.println("on");
  delay(500);
  int OnOff = 1-rFPSStatus;
  digitalWrite(rFPS, OnOff); Serial.println("on/off");
  delay(500);
  digitalWrite(rGn, LOW); digitalWrite(rSw, LOW); digitalWrite(rGnLine, LOW); Serial.println("off");
  return finger.fingerID; 
}

// returns "-1" when fail or "ID" when success
int getFingerprintIDez() {   
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;
  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;
  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with the confidence "); Serial.println(finger.confidence);
  return finger.fingerID; 
}