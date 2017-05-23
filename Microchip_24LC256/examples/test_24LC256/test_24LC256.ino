#include <Wire.h>
#include "Microchip_24LC256.h"
#include <Streaming.h>

Microchip_24LC256 eeprom;

void setup()
{
  Serial.begin(115200);
  
  byte buf1[PAGE_SIZE];
  byte buf2[PAGE_SIZE];

  //write / read from each page of EEPROM
  for (int page = 0; page < 512; page++) {
    
    //fill buf1 with a random letter, A-Z
    memset(buf1, rand() % ('Z' - 'A') + 'A', 64);
    
    //fill buf2 with all 0s
    memset(buf2, 0, 64);

    //write buf1 to page in EEPROM
    eeprom.write_page(page, buf1);
    
    //read page from EEPROM, fill buf2
    eeprom.read_page(page, buf2);

    //verify...
    for (int i = 0; i < PAGE_SIZE; i++) {
      Serial << page << ":" << i << "= " << (char) buf1[i] << " " << (char) buf2[i] << endl;
    }
  }
}

void loop()
{
  //do nothing
}
