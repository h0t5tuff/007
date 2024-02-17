#include <Adafruit_Fingerprint.h>

//for xiao (yellowTx>7&BrownRx>6) 
#define mySerial Serial1
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial); 

// relay for ignition
int rIgnition= 4;
//relays for central locking
int rFPS= 3; 
int rGn= 2;
int rSw= 1;
int rGnLine= 0;

void setup() {
  pinMode(rIgnition, OUTPUT);  digitalWrite(rIgnition, LOW);
  pinMode(rFPS, OUTPUT); digitalWrite(rFPS, LOW);
  pinMode(rGn, OUTPUT); digitalWrite(rGn, LOW);
  pinMode(rSw, OUTPUT); digitalWrite(rSw, LOW);
  pinMode(rGnLine, OUTPUT);  digitalWrite(rGnLine, LOW);
  Serial1.begin(9600); //for xiao
  while (!Serial); delay(100);
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
  Serial.println(F("identifying...")); //the F() macro function moves data into FLASH instead of sRAM
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
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
    Serial.print("******* We got "); Serial.print(finger.templateCount); Serial.println(" prints on sensor *******"); 
  } 
}

void loop() {
  getFingerprintID(); 
  delay(50); //no need for full speed
}

uint8_t getFingerprintID() {    //byte = uint8_t = unsigned char
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK: 
      Serial.println("Image taken"); 
      break;
    case FINGERPRINT_NOFINGER: //No finger detected
      return p;
    case FINGERPRINT_PACKETRECIEVEERR: 
      Serial.println("Communication error"); 
      return p;
    case FINGERPRINT_IMAGEFAIL: 
      Serial.println("Imaging error"); 
      return p;
    default: 
      Serial.println("Unknown error"); 
      return p; 
  } 
  // ok success
  p = finger.image2Tz();  
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  // ok converted
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) { 
    Serial.println("Found a print match!"); 
  } 
  else if (p == FINGERPRINT_PACKETRECIEVEERR) { 
    Serial.println("Communication error"); 
    //the finger.LEDcontrol() function takes these parameters (control, speed, coloridx, count)
    finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 3); // try again, this time use your finger!
    return p; 
  } 
  else if (p == FINGERPRINT_NOTFOUND) { 
    Serial.println("no match"); 
    finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 3);  // Let's put on a light show for the Thieves.
    return p; 
  } 
  else { 
    Serial.println("Unknown error"); 
    finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 3);// the univerese, god, buddha or OMG particle has intervined.
    return p; 
  } 
  // found a match
  Serial.print(" ID "); Serial.print(finger.fingerID);
  Serial.print(", confidence "); Serial.println(finger.confidence);
  if (finger.fingerID == 1 ) { // Tensor has two fingerprints stored on the fingerprint sensor and has full access to toggle the central locking and toggle the ignition.
    finger.LEDcontrol(FINGERPRINT_LED_BREATHING, 15, FINGERPRINT_LED_BLUE, 1); // blue indicates full access granted.
    //toggling the central locking
    digitalWrite(rGn, HIGH); digitalWrite(rSw, HIGH); digitalWrite(rGnLine, HIGH); Serial.print("3 relays on, ");
    delay(150);
    digitalWrite(rFPS,!digitalRead(rFPS)); Serial.print("FPS relay toggled, ");    //int rFPSToggled = 1-rFPSStatus; digitalWrite(rFPS, rFPSToggled); 
    delay(150);
    digitalWrite(rGn, LOW); digitalWrite(rSw, LOW); digitalWrite(rGnLine, LOW); Serial.println("3 relays off.");
    //toggling the ignition   
    digitalWrite(rIgnition,!digitalRead(rIgnition)); Serial.println("Ignition relay toggled.");
  }
  if (finger.fingerID == 3 ) { // Carie has one fingerprint stored on the fingerprint sensor and is only allowed to toggle the central locking.
    finger.LEDcontrol(FINGERPRINT_LED_BREATHING, 50, FINGERPRINT_LED_PURPLE, 1); // purple indiicates access granted to toggle the central locking.
    //toggling the central locking
    digitalWrite(rGn, HIGH); digitalWrite(rSw, HIGH); digitalWrite(rGnLine, HIGH); Serial.print("3 relays on, ");
    delay(150);
    digitalWrite(rFPS,!digitalRead(rFPS)); Serial.print("FPS relay toggled, ");    
    delay(150);
    digitalWrite(rGn, LOW); digitalWrite(rSw, LOW); digitalWrite(rGnLine, LOW); Serial.println("3 relays off.");
  }
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