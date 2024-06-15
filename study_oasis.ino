#include <Adafruit_NeoPixel.h>
#include <Wire.h> // Including the library for I2C communication
#include <PN532_I2C.h> // Including the library for I2C communication with the PN532 module
#include <PN532.h> // Including library for the PN532 module 
#include <NfcAdapter.h> // Including library for the NFC
#include <String.h>

int delayval = 100; // timing delay

//---------------------------STATE MACHINE DEF-----------------------------------
// Define the states of the traffic light
enum StudyOasisStates {
  WELCOME,
  WAITFORCARD,
  SESSIONSETUP,
  WRITECARD,
  GOODBYE,
  WAIT
};

StudyOasisStates currentState = WAIT; // Initialize the current state

//------------------------------NEOPIXEL LED STRIP-------------------------------
#define PIN 5	 // input pin Neopixel is attached to

#define NUMPIXELS 10 // number of neopixels in Ring
#define NUMCOLORS 7

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

enum Color {RED, YELLOW, GREEN, CYAN, BLUE, PURPLE, PINK};

int redColor = 255;
int greenColor = 0;
int blueColor = 0;

//------------------------------NFC READER--------------------------
PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);

int temp = 0;
int light = 0;
int color = 0;

//------------------------------SETUP-------------------------------
void setup() {
  Serial.begin(9600);
  pixels.begin(); // Initializes the NeoPixel library
  nfc.begin(); // Initialization of communication with the module
}

//------------------------------LOOP-------------------------------
void loop() {
  if(Serial.available() > 0){
    String message = Serial.readString();
    if(message == "WELCOME"){
      currentState = WELCOME; // Transition to WAITFORCARD
    }
    else if(message == "WRITECARD"){
      currentState = WRITECARD;
    }
  }

  switch (currentState){
    case WELCOME:
      welcomeProcedure();
      currentState = WAITFORCARD; // Transition to WAITFORCARD
      break;
    case WAITFORCARD:
      readCard();
      delay(1000);
      break;
    case SESSIONSETUP:
      sessionSetup();
      delay(1000);
      break;
    case WAIT:
      delay(1000);
      break;
  }
}

/**
------------------------------------FUNCTIONS------------------------------------
**/

//------------------------------OTHERS--------------------------------------
void setLedColor(Color color){
  switch (color){
    case RED:
      redColor = 255;
      greenColor = 0;
      blueColor = 0;
      break;
    case YELLOW:
      redColor = 255;
      greenColor = 255;
      blueColor = 0;
      break;
    case GREEN:
      redColor = 0;
      greenColor = 255;
      blueColor = 0;
      break;
    case CYAN:
      redColor = 0;
      greenColor = 255;
      blueColor = 255;
      break;
    case BLUE:
      redColor = 0;
      greenColor = 0;
      blueColor = 255;
      break;
    case PURPLE:
      redColor = 127;
      greenColor = 0;
      blueColor = 255;
      break;
    case PINK:
      redColor = 255;
      greenColor = 0;
      blueColor = 127;
      break;
    defaul:
      break;
  }
}

//----------------------------STATE MACHINE--------------------------------------
void welcomeProcedure(){
  Serial.println("Welcome");
  setLedColor(2);
  for(int i=0; i<3;i++){
    for(int i=0;i<NUMPIXELS;i++){
      pixels.setPixelColor(i, pixels.Color(redColor, greenColor, blueColor));
      pixels.show(); // Update pixel color to the hardware
      delay(delayval); // Delay for a period of time (in milliseconds)
    }
    for(int i=NUMPIXELS-1;i>=0;i--){
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
      pixels.show(); // Update pixel color to the hardware
      delay(delayval); // Delay for a period of time (in milliseconds)
    }
  }
}

void readCard(){
  Serial.println("Waiting for card...");
  if (nfc.tagPresent()){ // If we have pressed a card, we are reading a message from it and displaying this message immediately on the Serial monitor
    NfcTag tag = nfc.read(); // Reading the NFC card or tag
    
    if(tag.hasNdefMessage()){
      NdefMessage message = tag.getNdefMessage();
      NdefRecord record = message.getRecord(0); // Get the first record

      int payloadLength = record.getPayloadLength();
      byte payload[payloadLength];
      record.getPayload(payload);

      String payloadAsString = ""; // Processes the message as a string vs as a HEX value
      for (int c = 0; c < payloadLength; c++){
        payloadAsString += (char)payload[c];
      }

      // Clean the payload string
      String cleanString = payloadAsString;
      cleanString.remove(0,3);
        
      Serial.print("Cleaned String: ");
      Serial.println(cleanString);

      // Break down payload into variables
      char charArr[50];

      cleanString.toCharArray(charArr, 50);
      
      int i = 0;
      char *str = strtok (charArr, ":");
      char *messageParts[3];

      while (str != NULL){
        messageParts[i++] = str;
        str = strtok (NULL, ":");
      }

      temp = String(messageParts[0]).toInt();
      light = String(messageParts[1]).toInt();
      color = String(messageParts[2]).toInt();
    }
    currentState = SESSIONSETUP; // Transition to SESSIONSETUP
  }
}

void sessionSetup(){
  //Serial.println("Waiting for the session to start...");
}