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
  file_system.read_file();
  Serial.println("DONE");
}

void loop() {
  // put your main code here, to run repeatedly:

}