#include <SPI.h>
#include <Ethernet2.h>
#include <EEPROM.h>      // For storing IP numbers
#include <EthernetUdp2.h>

#include "SkaarhojUtils.h"
SkaarhojUtils utils;   // joystick for Pan & Tilt
SkaarhojUtils utils2;  // joystick for Zoom

#define MISO_1 12     // データの入力ピン(74HC165-QH)
#define SCK_1  13     // クロック出力ピン(74HC165-CK)
#define SL_1   10     // レジスタロードピン(74HC165-SL)

int sw_call   = 3;
int sw_modify = 4;
int sw_delete = 5;

byte ShiftData = 72;  //01001000
byte dt ;

EthernetClient client;
byte mac[] = {
  0x90, 0xA2, 0xDA, 0x10, 0x59, 0xEB
};
uint8_t ip[4] = { 10, 255, 255, 98 };        // Will hold the Arduino IP address
uint8_t camera_ip[4] = { 10, 255, 255, 98};  // Will hold tthe Panasonic Camera IP address

#include <ClientPanaAWHExTCP.h>
ClientPanaAWHExTCP cameraObj;

#include <MemoryFree.h>
#include <Streaming.h>

EthernetUDP Udp;
// ターゲットのブロードキャストアドレスを指定
unsigned int _remotePort = 10670;
unsigned int localPort   = 10669;   //local port to listen on

// DEBUG Switch.
// #define DEBUG     // Debug Setting
// #define MEM_DEBUG // Debug Setting

// Setup logic
void setup() {
#ifdef DEBUG
  delay(5000);
#endif
  Serial.begin(115200);
  Serial.println(F("Serial Start"));
#ifdef MEM_DEBUG
  // Shows free memory:
  Serial << F("Setup freeMemory()=") << freeMemory() << "\n";
#endif
  // give the ethernet module time to boot up:
  delay(1000);
  // Network Config (IP address / MAC Address /Server IP) from EEPROM
  eeprom_read();

  if (Ethernet.begin(mac) == 0) { // Update EEPROM IP to DHCP Address
    Serial.println(F("Failed to configure Ethernet using DHCP"));
    Ethernet.begin(mac, ip);
  }
#ifdef DEBUG
  Serial.print(F("Local IP:"));
  Serial.println(Ethernet.localIP());
#endif

  // camera find udp communication
  camera_find();

  cameraObj.begin(IPAddress(Udp.remoteIP()));
  cameraObj.connect();

  // Initializing the joystick at A12(H) and A13(V) + 35 (Joystick 1).
  // A14 and A15, 36 is also possible for Joystick 2
  utils.joystick_init (10, 1, 2, 35);
  utils2.joystick_init(10, 3, 4, 35);

  pinMode(sw_call,   INPUT_PULLUP); //PINB BV3
  pinMode(sw_modify, INPUT_PULLUP); //PINB BV4
  pinMode(sw_delete, INPUT_PULLUP); //PINB BV5

  // 74HC165のピン情報初期化
  pinMode(MISO_1, INPUT) ;
  pinMode(SCK_1,  OUTPUT) ;
  pinMode(SL_1,   OUTPUT) ;
  digitalWrite(SL_1, HIGH) ;
  digitalWrite(SCK_1, LOW) ;
  // ３秒後に開始
  delay(3000) ;
  // 74HC165入力端子のデータを読込み表示を行う
  ShiftData = ShiftIn(MISO_1, SCK_1, SL_1) ;
#ifdef DEBUG
  Serial.println(F("ShiftData: "));
  Serial.println(ShiftData, BIN);
#endif

}

void loop() {
  cameraObj.runLoop();
  if (cameraObj.isReady())  {
    if (utils.joystick_hasMoved(0) || utils.joystick_hasMoved(1))  {
      cameraObj.doPanTilt((utils.joystick_position(0) + 100) / 2, (utils.joystick_position(1) + 100) / 2);
#ifdef DEBUG
      Serial << F("Pan&Tilt  X: ") << ((utils.joystick_position(0) + 100) / 2) << F("\n");
      Serial << F("Pan&Tilt  Y: ") << ((utils.joystick_position(1) + 100) / 2) << F("\n");
#endif
    }
  }
  if (cameraObj.isReady())  {
    if (utils2.joystick_hasMoved(0))  {
      cameraObj.doZoom((utils2.joystick_position(0) + 100) / 2);
#ifdef DEBUG
      Serial << F("Zoom: ") << ((utils2.joystick_position(0) + 100) / 2) << F("\n");
      Serial << F("????: ") << ((utils2.joystick_position(1) + 100) / 2) << F("\n");
#endif
    }
  }

  //while the shift register is in serial mode
  //collect each shift register into a byte
  //the register attached to the chip comes in first
  dt = ShiftIn(MISO_1, SCK_1, SL_1) ;
  if ( dt != ShiftData ) {
    // データに変化が有れば表示する
    ShiftData = dt ;
    switch (ShiftData) {
      case B01111111:
#ifdef DEBUG
        Serial.println(F("Preset 5"));
#endif
        camera_preset(5);
        break;
      case B10111111:
#ifdef DEBUG
        Serial.println(F("Preset 6"));
#endif
        camera_preset(6);
        break;
      case B11011111:
#ifdef DEBUG
        Serial.println(F("Preset 7"));
#endif
        camera_preset(7);
        break;
      case B11101111:
#ifdef DEBUG
        Serial.println(F("Preset 8"));
#endif
        camera_preset(8);
        break;
      case B11110111:
#ifdef DEBUG
        Serial.println(F("Preset 1"));
#endif
        camera_preset(1);
        break;
      case B11111011:
#ifdef DEBUG
        Serial.println(F("Preset 2"));
#endif
        camera_preset(2);
        break;
      case B11111101:
#ifdef DEBUG
        Serial.println(F("Preset 3"));
#endif
        camera_preset(3);
        break;
      case B11111110:
#ifdef DEBUG
        Serial.println(F("Preset 4"));
#endif
        camera_preset(4);
        break;
#ifdef DEBUG
      default:
        //if nothing else matches, or 2 switch pushed do the default
        Serial.print(F("OTHER switchVar1:"));
        Serial.println(ShiftData, BIN);
#endif
    }
  }
#ifdef MEM_DEBUG
  // Shows free memory:
  Serial << F("freeMemory()=") << freeMemory() << F("\n");
#endif
}

void camera_preset(int pri_no) {
#ifdef DEBUG
  Serial << F("pri_no: ") << pri_no << F("\n");
#endif
  if (digitalRead(sw_call) == LOW) {
    cameraObj.recallPreset(pri_no);
#ifdef DEBUG
    Serial << F("call: ") << pri_no << F("\n");
#endif
  } else if (digitalRead(sw_modify) == LOW) {
    cameraObj.storePreset(pri_no);
#ifdef DEBUG
    Serial << F("modify: ") << pri_no << F("\n");
#endif
  } else if (digitalRead(sw_delete) == LOW) {
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

byte ShiftIn(int dataPin, int clockPin, int loadPin)
{
  unsigned char x ;
  int  i ;

  x = 0 ;
  digitalWrite(loadPin, LOW) ;         // 入力端子(A-H)の情報をレジスタに読込めと指示する
  digitalWrite(loadPin, HIGH) ;
  x = x | (digitalRead(dataPin) << 7) ;// H端子の内容を受信する
  for (i = 6 ; i >= 0 ; i--) {         // G端子～A端子まで繰り返す
    digitalWrite(clockPin, HIGH) ;        // 1ビットシフト指示を出す
    digitalWrite(clockPin, LOW) ;
    x = x | (digitalRead(dataPin) << i) ; // シフトされた内容を受信する
  }
  return x ;
}

void eeprom_read() {
  // *********************************
  // INITIALIZE EEPROM memory:
  // *********************************
  // Check if EEPROM has ever been initialized, if not, install default IP
  Serial.println(F("eeprom_read proc"));
  if (EEPROM.read(0) != 12 ||  EEPROM.read(1) != 232)  {  // Just randomly selected values which should be unlikely to be in EEPROM by default.
    Serial.println(F("EEPROM write if not available"));
    // Set these random values so this initialization is only run once!
    EEPROM.write(0, 12);
    EEPROM.write(1, 232);

    // Set default IP address for Arduino/C100 panel (172.16.99.57)
    EEPROM.write(2, 10);
    EEPROM.write(3, 255);
    EEPROM.write(4, 255);
    EEPROM.write(5, 98); // Just some value I chose, probably below DHCP range?

    // Set default IP address for ATEM Switcher (172.16.99.54):
    EEPROM.write(6, 10);
    EEPROM.write(7, 255);
    EEPROM.write(8, 255);
    EEPROM.write(9, 25);
  }


  // *********************************
  // Setting up IP addresses, starting Ethernet
  // *********************************
#ifdef DEBUG
  Serial.println(F("eeprom read"));

  ip[0] = EEPROM.read(0 + 2);
  ip[1] = EEPROM.read(1 + 2);
  ip[2] = EEPROM.read(2 + 2);
  ip[3] = EEPROM.read(3 + 2);


  // Setting the camera IP address:
  camera_ip[0] = EEPROM.read(0 + 2 + 4);
  camera_ip[1] = EEPROM.read(1 + 2 + 4);
  camera_ip[2] = EEPROM.read(2 + 2 + 4);
  camera_ip[3] = EEPROM.read(3 + 2 + 4);

  Serial << F("EEPROM Local IP Address: ") << ip[0] << "." << ip[1] << "." << ip[2] << "." << ip[3] << F("\n");
  Serial << F("EEPROM Camera IP Address: ") << camera_ip[0] << "." << camera_ip[1] << "." << camera_ip[2] << "." << camera_ip[3] << F("\n");
#endif

  // Setting MAC address:
  mac[0] = EEPROM.read(10);
  mac[1] = EEPROM.read(11);
  mac[2] = EEPROM.read(12);
  mac[3] = EEPROM.read(13);
  mac[4] = EEPROM.read(14);
  mac[5] = EEPROM.read(15);
  char buffer[18];
  sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial << F("EEPROM Local MAC address: ") << buffer << F(" - Checksum: ")
         << ((mac[0] + mac[1] + mac[2] + mac[3] + mac[4] + mac[5]) & 0xFF) << "\n";
  if ((uint8_t)EEPROM.read(16) != ((mac[0] + mac[1] + mac[2] + mac[3] + mac[4] + mac[5]) & 0xFF))  {
    Serial << F("MAC address not found in EEPROM memory!\n") <<
           F("Please load example sketch ConfigEthernetAddresses to set it.\n") <<
           F("The MAC address is found on the backside of your Ethernet Shield/Board\n (STOP)");
    while (true);
  }
}
void camera_find() {
  // 0xFFはMACアドレス、0xAAはIPアドレス（コードで実アドレスに修正）
  int time = 0;
  int i = 0;
  bool packet_flg = false;
  byte packet[94] = {
    0x00, 0x01, 0x00, 0x2a, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xAA, 0xAA,
    0xAA, 0xAA, 0x00, 0x00, 0x20, 0x11, 0x1e, 0x11, 0x23, 0x1f,
    0x1e, 0x19, 0x13, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xf0,
    0x00, 0x26, 0x00, 0x20, 0x00, 0x21, 0x00, 0x22, 0x00, 0x23,
    0x00, 0x25, 0x00, 0x28, 0x00, 0x40, 0x00, 0x41, 0x00, 0x42,
    0x00, 0x44, 0x00, 0xa5, 0x00, 0xa6, 0x00, 0xa7, 0x00, 0xa8,
    0x00, 0xad, 0x00, 0xb3, 0x00, 0xb4, 0x00, 0xb7, 0x00, 0xb8,
    0xff, 0xff, 0x11, 0xe2
  };

  //byte remoteIP[] = { ip[0], ip[1], ip[2], 255 };
  byte _remoteIP[4] = { 255, 255, 255, 255 };
  packet[12] = mac[0];
  packet[13] = mac[1];
  packet[14] = mac[2];
  packet[15] = mac[3];
  packet[16] = mac[4];
  packet[17] = mac[5];

  packet[18] = ip[0];
  packet[19] = ip[1];
  packet[20] = ip[2];
  packet[21] = ip[3];
  Udp.begin(localPort);
  // Packet Send
#ifdef DEBUG
  Serial.println(F("-> UDP Packet send"));
#endif

  Udp.beginPacket(_remoteIP, _remotePort);
  Udp.write(packet, 94);
  Udp.endPacket();

  // Packet Read
  while (packet_flg == false) {
    int packetsize = Udp.parsePacket();
    if (packetsize) {
      byte packetBuffer[packetsize];
      Udp.read(packetBuffer, packetsize);
#ifdef DEBUG
      Serial.println(F("<- UDP Packet recieved"));
      Serial.print(F("Camera IP:"));
      Serial.println(Udp.remoteIP());
      //Serial.print(F("Camera Port:"));
      //Serial.println(Udp.remotePort());
      //Serial.print(F("Packet Size:"));
      //Serial.println(packetsize);
#endif
      Udp.stop();
      packet_flg = true;
    }
  }
}
