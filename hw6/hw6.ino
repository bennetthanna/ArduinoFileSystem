#include <Streaming.h>
#include "Microchip_24LC256.h"
#include "FS.h"
#include <Wire.h>

void setup()
{
  Serial.begin(115200);
  FS file_system;
  file_system.reformat();
  for (int i = 0; i < FREE_LIST_SIZE; ++i) {
    Serial << "free space list[" << i << "] = ";
    Serial.print(file_system.free_space_list[i], BIN);
    Serial << endl;
    Serial << "file directory[" << i << "] = " << file_system.file_directory[i] << endl;
  }
  
  file_system.commit_to_EEPROM();
  file_system.initialize();

  for (int i = 4; i < FREE_LIST_SIZE; ++i) {
    file_system.flip_bit(i, (rand() % 7));
  }

  file_system.flip_bit(3, 0);
  file_system.flip_bit(3, 1);
  file_system.flip_bit(3, 2);
  
  for (int i = 0; i < FREE_LIST_SIZE; ++i) {
    Serial << "free space list[" << i << "] = ";
    Serial.print(file_system.free_space_list[i], BIN);
    Serial << endl;
    Serial << "file directory[" << i << "] = " << file_system.file_directory[i] << endl;
  } 

  Serial << file_system.find_num_free_blocks() << endl;
  Serial << file_system.num_free_blocks << endl;

  Serial << file_system.find_first_free_block_offset(file_system.free_space_list[3]) << endl;
  
}

void loop() 
{

}
