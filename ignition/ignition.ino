#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>
#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial mySerial(2, 3); //y,g
#else
#define mySerial Serial1
#endif
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
int rFPS = 4; int rFPSStatus = 0;

void setup() {
  Serial.begin(9600);
  pinMode(rFPS, OUTPUT); digitalWrite(rFPS, LOW);
  while (!Serial); delay(100);  //or {;} instead of delay
  finger.begin(57600); delay(5);
  if (finger.verifyPassword()) { Serial.println("found sensor :)"); }
  else { Serial.println("shit happens :("); 
  while (1) { delay(1); } }
  Serial.println(F("identifying...")); //F() macro moves this stuff into FLASH instead of sRAM
  finger.getTemplateCount();
  if (finger.templateCount == 0) { Serial.print("enroll prints"); }
  else { Serial.print(finger.templateCount); Serial.println(" prints on sensor"); } 
}

uint8_t getFingerprintID() {    // byte = uint8_t = unsigned char   // int8_t = char  
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK: Serial.println("Image taken"); break;
    case FINGERPRINT_NOFINGER: return p;
    case FINGERPRINT_PACKETRECIEVEERR: Serial.println("Communication error"); return p;
    case FINGERPRINT_IMAGEFAIL: Serial.println("Imaging error"); return p;
    default: Serial.println("Unknown error"); return p; }
  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK: Serial.println("Image converted"); break;
    case FINGERPRINT_IMAGEMESS: Serial.println("Image too messy"); 
    finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 3); //(control, speed, coloridx, count);
    return p;
    case FINGERPRINT_PACKETRECIEVEERR: Serial.println("Communication error");
    finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 3); //(control, speed, coloridx, count);
return p;
    case FINGERPRINT_FEATUREFAIL: Serial.println("Could not find fingerprint features"); 
    finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 3); //(control, speed, coloridx, count);
    return p;
    case FINGERPRINT_INVALIDIMAGE: Serial.println("Could not find fingerprint features");    
    finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 3); //(control, speed, coloridx, count);
return p;
    default: Serial.println("Unknown error"); 
    finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 3); //(control, speed, coloridx, count);
return p; }
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) { Serial.println("Found a print match!"); } 
  else if (p == FINGERPRINT_PACKETRECIEVEERR) { Serial.println("Communication error"); 
  finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 3); //(control, speed, coloridx, count);
return p; } 
  else if (p == FINGERPRINT_NOTFOUND) { Serial.println("no match"); 
  finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 3); //(control, speed, coloridx, count);
  return p; } 
  else { Serial.println("Unknown error"); 
  finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 3); //(control, speed, coloridx, count);
return p; }
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  finger.LEDcontrol(FINGERPRINT_LED_BREATHING, 50, FINGERPRINT_LED_BLUE, 1); //(control, speed, coloridx, count);
  Serial.println("in");
  delay(100);
  int  lol = 1-rFPSStatus;
  digitalWrite(rFPS, lol); Serial.println("on / off");
  delay(100);
  Serial.println("out");
  return finger.fingerID; 
}
/*
int getFingerprintIDez() {   // returns "-1" when fail or "ID" when success
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;
  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;
  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID; 
}
*/
void loop() {
  rFPSStatus = digitalRead(rFPS);
  getFingerprintID(); 
  delay(50); 
}
