#include <Arduino.h>

#include "Microchip_24LC256.h"
#include "FS.h"
#include <Wire.h>

void setup()
{
  Serial.begin(115200);
  Serial.flush();
  FS file_system;
  file_system.reformat();
  file_system.create_file("file_00.txt");
  file_system.write_file("hi", 2);
  file_system.open_file("file_00.txt");
  file_system.seek_file();
  file_system.write_file("bye", 3);
  file_system.write_file("please please please work I really want to be done I'm over it I give up I think this works that'd be cool but I need to fix the clearing of the buffer", 151);
  file_system.write_file("hot dogs are nice but not as nice as hamburgers", 47); 
  file_system.write_file("I hope this works and doesn't let me write more than 1024 bytes. If it doesn't, I don't know what to do.", 104);
  file_system.write_file("I need to write real stuff to make sure it prints out correctly but I don't know what to write. I'm running out of ideas. Maybe I should copy and paste one of my essays in here.", 177);
  file_system.write_file("I'm not even half way there holy guacamole how am I supposed to write so many characters? Let's just do big words. Supercalifradgilisticexpialidocious. I bet that's spelt wrong. But I don't care.", 195);
  file_system.write_file("bye", 3);
  file_system.write_file("please please please work I really want to be done I'm over it I give up I think this works that'd be cool but I need to fix the clearing of the buffer", 151);
  file_system.write_file("hot dogs are nice but not as nice as hamburgers", 47); 
  file_system.write_file("I hope this works and doesn't let me write more than 1024 bytes. If it doesn't, I don't know what to do.", 104);
  file_system.write_file("I need to write real stuff to make sure it prints out correctly but I don't know what to write. I'm running out of ideas. Maybe I should copy and paste one of my essays in here.", 177);
  file_system.read_file();
  Serial.println("DONE");
}

void loop() {
  // put your main code here, to run repeatedly:

}