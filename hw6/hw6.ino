#include <Streaming.h>
#include "Microchip_24LC256.h"
#include "FS.h"
#include <Wire.h>

void setup()
{
  Serial.begin(115200);
  Serial.flush();
  FS file_system;
  file_system.reformat();

  file_system.print_free_list();
  file_system.print_file_directory();
  Serial << file_system.find_num_free_blocks() << endl;
  Serial << file_system.find_first_free_block() << endl;
  file_system.commit_to_EEPROM();
  file_system.initialize();
  for (int i = 4; i < FREE_LIST_SIZE; ++i) {
    file_system.flip_bit(i, (rand() % 8));
  }
  file_system.flip_bit(3, 0);
  file_system.flip_bit(3, 1);
  file_system.flip_bit(3, 2);
  file_system.print_free_list();
  file_system.print_file_directory();
  Serial << file_system.find_num_free_blocks() << endl;
  Serial << file_system.num_free_blocks << endl;
  Serial << file_system.find_first_free_block() << endl;
  file_system.create_file("hello_world");
  file_system.create_file("hello_world");
  file_system.print_free_list();
}

void loop() 
{

}
