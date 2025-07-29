#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>
#include "HomeSpan.h" 

//relays for central locking
int rGnln= D0; //a1
int rGn= D1; //a2
int rSwln= D2; //a3
int rSw= D3; //a4
int rFPS= D4; // b1
bool rFPSstate= HIGH;
// relay for disarm
int rDisarm= D5; //b2
bool rDisarmstate= HIGH;
// relays for ignition
int rIgnition= D8; //b3
int rBrake= D10; //b4
// homekit
int siri= D9;

struct DEV_IGNITION : Service::LightBulb {       
  int ledPin;               
  SpanCharacteristic *power;                   
  // constructor:
  DEV_IGNITION(int ledPin) : Service::LightBulb(){
    power=new Characteristic::On();             
    this->ledPin=ledPin;                 
    pinMode(ledPin,OUTPUT);     
  }
  // update:
  boolean update(){  
    digitalWrite(ledPin,power->getNewVal());  
    rDisarmstate = digitalRead(rDisarm);
    if (rDisarmstate == LOW) {
      digitalWrite(rDisarm,HIGH); digitalWrite(rFPS,LOW);  
      delay(10);
      digitalWrite(rIgnition, HIGH); delay(500); digitalWrite(rIgnition, LOW); delay(500);
      digitalWrite(rIgnition, HIGH); delay(500); digitalWrite(rIgnition, LOW); delay(500);
      digitalWrite(rBrake, HIGH); delay(2000); digitalWrite(rIgnition, HIGH); delay(450); digitalWrite(rIgnition, LOW); delay(2000); digitalWrite(rBrake, LOW);
      delay(10);
      Serial.println(" Locked, Disarmed > Ignition "); 
    }
    else if (rDisarmstate == HIGH) {
      digitalWrite(rBrake, HIGH); delay(2000); digitalWrite(rIgnition, HIGH); delay(450); digitalWrite(rIgnition, LOW); delay(2000); digitalWrite(rBrake, LOW);
      delay(10);
      digitalWrite(rDisarm,LOW); digitalWrite(rFPS,LOW);  
      Serial.println(" *** SECURE *** ");  
    }
    return(true);   
  }
};

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial0); //Serial1 on xiao(7,6) Serial0 on xiaoesp32c3(7,6) and Serial2 on esp32(16,17)

/////////////////////// setup ///////////////////////////////////
void setup() {
  //outputs for central locking
  pinMode(rGnln, OUTPUT); digitalWrite(rGnln, LOW);
  pinMode(rGn, OUTPUT); digitalWrite(rGn, LOW);
  pinMode(rSwln, OUTPUT); digitalWrite(rSwln, LOW);
  pinMode(rSw, OUTPUT); digitalWrite(rSw, LOW);
  pinMode(rFPS, OUTPUT); digitalWrite(rFPS, rFPSstate);
  //outputs for disarm
  pinMode(rDisarm, OUTPUT); digitalWrite(rDisarm, rDisarmstate);
  //outputs for ignition
  pinMode(rIgnition, OUTPUT); digitalWrite(rIgnition, LOW);
  pinMode(rBrake, OUTPUT); digitalWrite(rBrake, LOW);
  Serial.begin(115200); // fast and reliable speed
  homeSpan.enableOTA();
  homeSpan.begin(Category::Bridges,"007 Bridge");
  //homeSpan.enableWebLog(); // http://homespan-19ad214bea32.local/status
  new SpanAccessory();  
    new Service::AccessoryInformation();
      new Characteristic::Identify(); 
      new Characteristic::Name("007 Bridge");  
      new Characteristic::Manufacturer("Tensor"); 
      new Characteristic::SerialNumber("x");   
      new Characteristic::Model("x");    
      new Characteristic::FirmwareRevision("1"); 
  new SpanAccessory(); 
    new Service::AccessoryInformation(); 
      new Characteristic::Identify();      
      new Characteristic::Name("007Led");
    new DEV_IGNITION(siri);
  Serial.println("\n\nAdafruit finger detect test"); //fingerprint stuff
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

/////////////////////// loop ///////////////////////////////////
void loop() {
  homeSpan.poll(); 
  getFingerprintID(); 
  delay(50); //no need for full speed
}

/////////////////////// getFingerprintID ///////////////////////////////////
uint8_t getFingerprintID() {    //byte = uint8_t = unsigned char
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK: 
      //Serial.println("Image taken"); 
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
      //Serial.println("Image converted");
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
    //Serial.println("Found a print match!"); 
    Serial.print(">>>>> ID "); Serial.print(finger.fingerID); Serial.print(", >>>>> confidence "); Serial.println(finger.confidence);
    // B.L.U.E.: Tensor and Carie have six fingerprints stored on the fingerprint sensor. Their fingers 1 and 4 have full access to toggle central locking, disarm and ignition.
    if (finger.fingerID >= 1 && finger.fingerID <= 4) { 
      finger.LEDcontrol(FINGERPRINT_LED_BREATHING, 20, FINGERPRINT_LED_BLUE, 1); // blue led indicates full access granted.
      rFPSstate = digitalRead(rFPS);
      if (rFPSstate == LOW) {
        digitalWrite(rGnln, HIGH); digitalWrite(rGn, HIGH); digitalWrite(rSwln, HIGH); digitalWrite(rSw, HIGH); //Serial.print("wires cut. ");
        delay(500);
        digitalWrite(rFPS,HIGH); digitalWrite(rDisarm,HIGH);  
        digitalWrite(rGnln, LOW); digitalWrite(rGn, LOW); digitalWrite(rSwln, LOW); digitalWrite(rSw, LOW); //Serial.print("wires connected. "); 
        delay(10);
        digitalWrite(rIgnition, HIGH); delay(500); digitalWrite(rIgnition, LOW); delay(500);
        digitalWrite(rIgnition, HIGH); delay(500); digitalWrite(rIgnition, LOW); delay(500);
        digitalWrite(rBrake, HIGH); delay(2000); digitalWrite(rIgnition, HIGH); delay(450); digitalWrite(rIgnition, LOW); delay(2000); digitalWrite(rBrake, LOW);
        delay(10);
        Serial.println(" *** ADVENTURE *** "); //Unlocked, Disarmed > Ignition
      }
      else if (rFPSstate == HIGH) {
        digitalWrite(rGnln, HIGH); digitalWrite(rGn, HIGH); digitalWrite(rSwln, HIGH); digitalWrite(rSw, HIGH); //Serial.print("wires cut. ");
        delay(500);
        digitalWrite(rFPS,LOW); digitalWrite(rDisarm,LOW); 
        digitalWrite(rGnln, LOW); digitalWrite(rGn, LOW); digitalWrite(rSwln, LOW); digitalWrite(rSw, LOW); //Serial.println("wires connected."); 
        delay(10);
        Serial.println(" *** SECURE *** ");  
      }
   }
    // G.R.E.E.N.: Tensor and Carie have six fingerprints stored on the fingerprint sensor. Their fingers 5 to 8 can toggle the central locking and disarm.
    else if (finger.fingerID >= 5 && finger.fingerID <= 8) { 
      finger.LEDcontrol(FINGERPRINT_LED_BREATHING, 40, FINGERPRINT_LED_GREEN, 1); // blue indicates full access granted.
      rFPSstate = digitalRead(rFPS);
      if (rFPSstate == LOW) {
        digitalWrite(rGnln, HIGH); digitalWrite(rGn, HIGH); digitalWrite(rSwln, HIGH); digitalWrite(rSw, HIGH); //Serial.print("wires cut. ");
        delay(500);
        digitalWrite(rFPS,HIGH); digitalWrite(rDisarm,HIGH); 
        digitalWrite(rGnln, LOW); digitalWrite(rGn, LOW); digitalWrite(rSwln, LOW); digitalWrite(rSw, LOW); //Serial.println("wires connected. "); 
        delay(10);
        Serial.println(" Unlocked, Disarmed ");  
      }
      else if (rFPSstate == HIGH) {
        digitalWrite(rGnln, HIGH); digitalWrite(rGn, HIGH); digitalWrite(rSwln, HIGH); digitalWrite(rSw, HIGH); //Serial.print("wires cut. ");
        delay(500);
        digitalWrite(rFPS,LOW); digitalWrite(rDisarm,LOW); 
        digitalWrite(rGnln, LOW); digitalWrite(rGn, LOW); digitalWrite(rSwln, LOW); digitalWrite(rSw, LOW); //Serial.println("wires connected."); 
        delay(10);
        Serial.println(" *** SECURE *** ");  
      }
    }
    // A.L.L. other fingerprint stored on the fingerprint sensor are only allowed to toggle the central locking.
    else { 
      finger.LEDcontrol(FINGERPRINT_LED_BREATHING, 50, FINGERPRINT_LED_PURPLE, 1); // purple indiicates access granted to toggle the central locking.
      rFPSstate = digitalRead(rFPS);
      if (rFPSstate == LOW) {
        digitalWrite(rGnln, HIGH); digitalWrite(rGn, HIGH); digitalWrite(rSwln, HIGH); digitalWrite(rSw, HIGH); //Serial.print("wires cut. ");
        delay(500);
        digitalWrite(rFPS,HIGH); digitalWrite(rDisarm,LOW); 
        digitalWrite(rGnln, LOW); digitalWrite(rGn, LOW); digitalWrite(rSwln, LOW); digitalWrite(rSw, LOW); //Serial.println("wires connected. "); 
        delay(10);
        Serial.println(" Unlocked, Armed ^_^ ");  
      }
      else if (rFPSstate == HIGH) {
        digitalWrite(rGnln, HIGH); digitalWrite(rGn, HIGH); digitalWrite(rSwln, HIGH); digitalWrite(rSw, HIGH); //Serial.print("wires cut. ");
        delay(500);
        digitalWrite(rFPS,LOW); digitalWrite(rDisarm,LOW); 
        digitalWrite(rGnln, LOW); digitalWrite(rGn, LOW); digitalWrite(rSwln, LOW); digitalWrite(rSw, LOW); //Serial.println("wires connected."); 
        delay(10);
        Serial.println(" *** SECURE *** ");  
      }
    }
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR) { 
    Serial.println("Communication error"); 
    //the finger.LEDcontrol() function takes these parameters (control, speed, coloridx, count)  YELLOW CYAN COLDWHITE WARMWHITE
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
  Serial.println("----------------------    hmmmmmm    -------------------------");
  return finger.fingerID; 
}

/////////////////////// getFingerprintIDez ///////////////////////////////////
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
  Serial.println("----------------------    hmmmmmm    -------------------------");
  return finger.fingerID; 
}
