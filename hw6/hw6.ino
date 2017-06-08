#include <Arduino.h>
#include "Microchip_24LC256.h"
#include "FS.h"

void setup()
{
  Serial.begin(115200);
  Serial.flush();
  FS file_system;
  file_system.reformat();
  
  char filename[26];
  // cretae 5 files and list
  for (int i = 0; i < 5; ++i) {
    sprintf(filename, "test_%02i.txt", i);
    file_system.create_file(filename);
  }
  memset(filename, 0, 26);

  // delete 3 files and list
  file_system.delete_file("test_00.txt");
  file_system.delete_file("test_01.txt");
  file_system.delete_file("test_02.txt");
  file_system.list_files();

  // write 256 bytes to file and read
  file_system.open_file("test_03.txt");
  file_system.write_file("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuv", 256);
  file_system.read_file();

  // create a file that exists
  file_system.create_file("test_03.txt");

  // open a file that does not exists
  file_system.open_file("uh_oh_spaghettio.txt");

  // delete a file that does not exist
  file_system.open_file("aint_nothin_here.txt");

  // write more than 1024 bytes to a file
  file_system.open_file("test_04.txt");
  for (int i = 0; i < 16; ++i) {
    file_system.write_file("Here are 64 bytes. Stupid filler text. Pay no attention to this.", 64);
  }
  file_system.write_file("Ooopsies file overload oh no waz gonna happen?!", 47);
  file_system.read_file();
  file_system.close_file();

  // list files to show size change
  file_system.list_files();

  // seek to beginning of file and list to show it won't change file size
  file_system.open_file("test_04.txt");
  file_system.seek_file();
  file_system.close_file();
  file_system.list_files();
  file_system.open_file("test_04.txt");
  file_system.write_file("I'm overwriting my filler text. Vroom vroom motherfucker. ", 58);
  file_system.read_file();
  file_system.close_file();

  // try to read, seek, and write when no file is open
  file_system.write_file("Knock knock. No one's home.", 27);
  file_system.seek_file();
  file_system.read_file();
  
  Serial.println(F("DONE"));
}

void loop() {
  // put your main code here, to run repeatedly:

}