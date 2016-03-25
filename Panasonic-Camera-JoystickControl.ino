#include <SPI.h>
#include <Ethernet2.h>

#include "SkaarhojUtils.h"
SkaarhojUtils utils;   // joystick for Pan & Tilt
SkaarhojUtils utils2;  // joystick for Zoom

//define where your pins are
int latchPin =  9;
int dataPin  = 10;
int clockPin = 12;

byte settingVal[] = {0, 0, 0, 0, 0, 0, 0};
byte switchVar1 = 72;  //01001000

EthernetClient client;
byte mac[] = {
  0x90, 0xA2, 0xDA, 0x00, 0xF2, 0x00
};
IPAddress ip(10, 255, 255, 98);
IPAddress camera(10, 255, 255, 25);

#include <ClientPanaAWHExTCP.h>
ClientPanaAWHExTCP cameraObj;

#include <MemoryFree.h>
#include <Streaming.h>

// DEBUG Switch.
#define DEBUG     // Debug Setting
#define MEM_DEBUG // Debug Setting

void setup() {
  Serial.begin(9600);
  Serial.println("Serial Start");

#ifdef MEM_DEBUG
  // Shows free memory:
  Serial << F("freeMemory()=") << freeMemory() << "\n";
#endif

  // give the ethernet module time to boot up:
  delay(1000);

  //if (Ethernet.begin(mac) == 0) {
  //  Serial.println("Failed to configure Ethernet using DHCP");
  Ethernet.begin(mac, ip);
  // }

#ifdef DEBUG
  Serial.print("IP:");
  Serial.println(Ethernet.localIP());
#endif

  //cameraObj.begin(camera);
  //cameraObj.connect();

  // Initializing the joystick at A12(H) and A13(V) + 35 (Joystick 1).
  // A14 and A15, 36 is also possible for Joystick 2
  utils.joystick_init (10, A1, A2, 35);
  utils2.joystick_init(10, A3, A4, 36);

  DDRD = 0b00000000;    //All pins in PORTD are inputs
  PORTD = 0b00011100;    //Pull-ups enabled in the pins 2,3,4 and pull-ups disabled in pins 4,5,6 and 7

  //  pinMode(sw_call,   INPUT_PULLUP); //PINB BV3
  //  pinMode(sw_modify, INPUT_PULLUP); //PINB BV4
  //  pinMode(sw_delete, INPUT_PULLUP); //PINB BV5
}

void loop() {
  //cameraObj.runLoop();
  //if (cameraObj.isReady())  {
  //    if (utils.joystick_hasMoved(0))  {
  //      cameraObj.doPanTilt((utils.joystick_position(0) + 100) / 2, (utils.joystick_position(1) + 100) / 2);
  //#ifdef DEBUG
  //      Serial << F("Pan&Tilt  X: ") << ((utils.joystick_position(0) + 100) / 2) << F("\n");
  //      Serial << F("Pan&Tilt  Y: ") << ((utils.joystick_position(1) + 100) / 2) << F("\n");
  //#endif
  //    }
  //  }
  //  if (cameraObj.isReady())  {
  //    if (utils2.joystick_hasMoved(1))  {
  //      cameraObj.doZoom((utils2.joystick_position(1) + 100) / 2);
  //#ifdef DEBUG
  //      Serial << F("Zoom: ") << ((utils2.joystick_position(1) + 100) / 2) << F("\n");
  //#endif
  //    }
  //  }
  //Pulse the latch pin:
  //set it to 1 to collect parallel data
  digitalWrite(latchPin, 1);
  //set it to 1 to collect parallel data, wait
  delayMicroseconds(20);
  //set it to 0 to transmit data serially
  digitalWrite(latchPin, 0);

  //while the shift register is in serial mode
  //collect each shift register into a byte
  //the register attached to the chip comes in first
  switchVar1 = shiftIn(dataPin, clockPin);
  switch (switchVar1) {
    case B00101010:
      Serial.println("Preset 1");
      //  camera_preset(1);
      break;
    case B00010101:
      Serial.println("Preset 2");
      //  camera_preset(2);
      break;
    case B01010100:
      Serial.println("Preset 3");
      //  camera_preset(3);
      break;
    case B00101001:
      Serial.println("Preset 4");
      //  camera_preset(4);
      break;
    case B01010010:
      Serial.println("Preset 5");
      //  camera_preset(5);
      break;
    case B00100101:
      Serial.println("Preset 6");
      //  camera_preset(6);
      break;
    case B01001010:
      Serial.println("Preset 7");
      //  camera_preset(7);
      break;
          case B01001010:
      Serial.println("Preset 8");
      //  camera_preset(8);
      break;
    default:
      // if nothing else matches, do the default
      Serial.println("Play It, Joe");
  }
}

void camera_preset(int pri_no) {
#ifdef DEBUG
  Serial << F("pri_no: ") << pri_no << F("\n");
#endif
  if (PIND & _BV(2)) { //if (digitalRead(sw_call)) {
    cameraObj.recallPreset(pri_no);
#ifdef DEBUG
    Serial << F("call: ") << pri_no << F("\n");
#endif
  } else if (PIND & _BV(3)) { //} else if (digitalRead(sw_modify)) {

    cameraObj.storePreset(pri_no);
#ifdef DEBUG
    Serial << F("modify: ") << pri_no << F("\n");
#endif
  } else if (PIND & _BV(4)) { //} else if (digitalRead(sw_delete)) {
    cameraObj.deletePreset(pri_no);
#ifdef DEBUG
    Serial << F("delete: ") << pri_no << F("\n");
#endif
  } else {
#ifdef DEBUG
    Serial.println(F("do nothing: "));
#endif
  }
}



////// ----------------------------------------shiftIn function
///// just needs the location of the data pin and the clock pin
///// it returns a byte with each bit in the byte corresponding
///// to a pin on the shift register. leftBit 7 = Pin 7 / Bit 0= Pin 0
byte shiftIn(int myDataPin, int myClockPin) {
  int i;
  int temp = 0;
  int pinState;
  byte myDataIn = 0;

  pinMode(myClockPin, OUTPUT);
  pinMode(myDataPin, INPUT);
  //we will be holding the clock pin high 8 times (0,..,7) at the
  //end of each time through the for loop

  //at the begining of each loop when we set the clock low, it will
  //be doing the necessary low to high drop to cause the shift
  //register's DataPin to change state based on the value
  //of the next bit in its serial information flow.
  //The register transmits the information about the pins from pin 7 to pin 0
  //so that is why our function counts down
  for (i = 7; i >= 0; i--)
  {
    digitalWrite(myClockPin, 0);
    delayMicroseconds(2);
    temp = digitalRead(myDataPin);
    if (temp) {
      pinState = 1;
      //set the bit to 0 no matter what
      myDataIn = myDataIn | (1 << i);
    }
    else {
      //turn it off -- only necessary for debuging
      //print statement since myDataIn starts as 0
      pinState = 0;
    }

    //Debuging print statements
#ifdef DEBUG
    Serial.print(pinState);
    Serial.print("     ");
    Serial.println (myDataIn, BIN);
#endif
    digitalWrite(myClockPin, 1);
  }
  //debuging print statements whitespace
#ifdef DEBUG
  Serial.println();
  Serial.println(myDataIn, BIN);
#endif
  return myDataIn;
}
