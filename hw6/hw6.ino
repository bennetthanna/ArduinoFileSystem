#include <Arduino.h>

#include "Microchip_24LC256.h"
#include "FS.h"
#include <Wire.h>

void setup()
{
  // Serial.begin(115200);
  // Serial.flush();
  // FS file_system;
  // file_system.reformat();
  //
  // file_system.print_free_list();
  // file_system.print_file_directory();
  // Serial.println(file_system.find_num_free_blocks());
  // Serial.println(file_system.find_first_free_block());
  // file_system.commit_to_EEPROM();
  // file_system.initialize();
  // for (int i = 4; i < FREE_LIST_SIZE; ++i) {
  //  file_system.flip_bit(i, (rand() % 8));
  // }
  // file_system.flip_bit(3, 0);
  // file_system.flip_bit(3, 1);
  // file_system.flip_bit(3, 2);
  // file_system.print_free_list();
  // file_system.print_file_directory();
  // file_system.num_free_blocks = file_system.find_num_free_blocks();
  // Serial.println(file_system.num_free_blocks);
  // Serial.println(file_system.find_first_free_block());
  // Serial.println("here");
  // file_system.create_file("hello_world.txt");
  // file_system.create_file("please.txt");
  // file_system.create_file("hello_world.txt");
  // file_system.print_free_list();
  // file_system.print_file_directory();
  // file_system.list_files();
  // file_system.delete_file("cool_beans.txt");
  // file_system.delete_file("hello_world.txt");
  // file_system.print_free_list();
  // file_system.print_file_directory();
  // file_system.list_files();
  // byte *input_buffer;
  // input_buffer = "please";
  // file_system.write_file("please.txt", input_buffer, 6);
  // Serial.println("DONE");

  // byte *input_buffer;
  // int count = 6;
  // byte buffer[1024];
  // buffer[0] = 0;
  //
  // input_buffer = "please";
  // strncat((char*)buffer, (char*)input_buffer, count);
  // Serial.print("input_buffer = ");
  // Serial.println((char*)input_buffer);
  // Serial.print("buffer = ");
  // Serial.println((char*)buffer);
  // strncat((char*)buffer, (char*)input_buffer, count);
  // Serial.print("input_buffer = ");
  // Serial.println((char*)input_buffer);
  // Serial.print("buffer = ");
  // Serial.println((char*)buffer);
}

void loop()
{

}
