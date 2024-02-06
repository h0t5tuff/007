


How can I get the Arduino and it's fingerprint reader to do different tasks for different recognized fingers? E.g.: person A's finger only can open main door. Person B's finger can only trigger side door.




When one of the two fingers get recognized both happens: Pin 4 and 8 and then 7 and 13 get toggled.
Correct would be one finger toggles 4 and 8, the other finger 7 and 13.
void setup() {// Start the serial connection to enable debug output to be displayed on the Serial MonitorSerial.begin(9600);pinMode(LED_BUILTIN, OUTPUT);int ledPin = 4;pinMode(ledPin, OUTPUT);
int ledPin2 = 7;pinMode(ledPin2, OUTPUT);
int ledPin3 = 8;pinMode(ledPin3, OUTPUT);
int ledPin4 = 12;pinMode(ledPin4, OUTPUT);
Serial.print(F("Connecting to fingerprint sensor..."));// Begin serial communication with the fingerprint sensorfinger.begin(57600);// Wait for stabilisationdelay(100);if (finger.verifyPassword()) {Serial.println("OK!");} else {Serial.println("FAILED");}
// Retrieve the count of registered fingerprintsfinger.getTemplateCount();Serial.print(F("Sensor database contains "));Serial.print(finger.templateCount);Serial.println(F(" fingerprint templates"));
Serial.println(F("Ready..."));}
void loop() {// Step One - if finger is detected, attempt to take a photoint status = finger.getImage();if(status == FINGERPRINT_OK) {Serial.print(F("Finger detected... "));}else { return; }
// Step Two - attempt to extract features from the imagestatus = finger.image2Tz();if(status == FINGERPRINT_OK) {Serial.print(F("Features extracted... "));}else {Serial.println(F("Could not extract features."));return;}
// Step Three - search for matching features in the database of registered fingerprintsstatus = finger.fingerFastSearch();if(status == FINGERPRINT_OK) {Serial.print(F("Match found! ID:"));Serial.print(finger.fingerID);Serial.print(F(", confidence level:"));Serial.println(finger.confidence);
}else {Serial.println(F("No match found."));Serial.print(F("good luck next time ----->>>>"));digitalWrite( 12, HIGH);delay(1000);digitalWrite( 12, LOW);return;}
if(Serial.print(finger.fingerID==1)){Serial.print(F("opening door1 ----->>>>"));Serial.println();digitalWrite( 4, HIGH);digitalWrite( 8, HIGH);
delay(1000);digitalWrite(4, LOW);digitalWrite( 8,LOW);delay(500);}
if(Serial.print(finger.fingerID == 2)){Serial.print(F("opening door2 ----->>>>"));Serial.println();digitalWrite( 7, HIGH);digitalWrite( 13, HIGH);delay(1000);digitalWrite(7, LOW);digitalWrite( 13, LOW);delay(500);}
}










if(Serial.print(finger.fingerID==1)){
Serial.print() returns the number of characters printed. This will likely be a number other than 0 so the 'if' statement considers that 'true'.
What you PROBABLY wanted to do was:

  Serial.print(finger.fingerID);

  if(finger.fingerID==1) {

HOLY COW!!!!!
Thank you so much!!!! I got it to work. Super great!
And at the end, I look at the advide and think: sure, why didn't see that solution myself..

