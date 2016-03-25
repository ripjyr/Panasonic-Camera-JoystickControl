// Including libraries:
#include <EEPROM.h>      // For storing IP numbers
#include <Streaming.h>


static uint8_t ip[] = {
  10, 255, 255, 98
};   

static uint8_t atemip[] = {
  10, 255, 255, 25
};   
static uint8_t mac[] = {
  0x90, 0xA2, 0xDA, 0x10, 0x59, 0xEB
};   

char buffer[18];
void setup() {
  delay(10000);
  Serial.begin(115200);
  Serial.println("Serial Start");

  EEPROM.write(0, 12);
  EEPROM.write(1, 232);

  EEPROM.write(2, ip[0]);
  EEPROM.write(3, ip[1]);
  EEPROM.write(4, ip[2]);
  EEPROM.write(5, ip[3]);
  sprintf(buffer, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  Serial.print("writing Device (Arduino) IP address: \n    ");
  Serial.println(buffer);

  EEPROM.write(6, atemip[0]);
  EEPROM.write(7, atemip[1]);
  EEPROM.write(8, atemip[2]);
  EEPROM.write(9, atemip[3]);
  sprintf(buffer, "%d.%d.%d.%d", atemip[0], atemip[1], atemip[2], atemip[3]);
  Serial.print("Storing Camera IP address: \n    ");
  Serial.println(buffer);

  // 10-16: Arduino MAC address (6+1 byte)
  // Set MAC address + checksum:
  EEPROM.write(10, mac[0]);
  EEPROM.write(11, mac[1]);
  EEPROM.write(12, mac[2]);
  EEPROM.write(13, mac[3]);
  EEPROM.write(14, mac[4]);
  EEPROM.write(15, mac[5]);
  EEPROM.write(16, (mac[0] + mac[1] + mac[2] + mac[3] + mac[4] + mac[5]) & 0xFF);

  // Reading MAC address:
  mac[0] = EEPROM.read(10);
  mac[1] = EEPROM.read(11);
  mac[2] = EEPROM.read(12);
  mac[3] = EEPROM.read(13);
  mac[4] = EEPROM.read(14);
  mac[5] = EEPROM.read(15);
  char buffer[18];
  sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial << F("Device MAC address: \n    ") << buffer << F(" - Checksum: ")
         << ((mac[0] + mac[1] + mac[2] + mac[3] + mac[4] + mac[5]) & 0xFF);
  if ((uint8_t)EEPROM.read(16) != ((mac[0] + mac[1] + mac[2] + mac[3] + mac[4] + mac[5]) & 0xFF))  {
    Serial << F("\nStored checksum mismatched: ") << (uint8_t)EEPROM.read(16) << F(" => MAC address not found in EEPROM memory!\n") <<
           F("Please load example sketch ConfigEthernetAddresses to set it.\n") <<
           F("The MAC address is found on the backside of your Ethernet Shield/Board/Device\n (STOP)");
    while (true);
  } else {
    Serial << " => OK";
  }
  Serial << "\n";

  Serial.println("\nDONE! Everything worked! Now, time to upload the sketch for this model...\n");
}

void loop() {
}

