#include <Streaming.h>
#include "FS.h"

// write to eeprom every time you create or delete a file

// 0 is file directory
// 1 is free space list

FS::FS():eeprom() {
  Serial << "Inside constructor!" << endl;
}

void FS::print_free_list() {
  for (int i = 0; i < FREE_LIST_SIZE; ++i) {
    Serial << "free space list[" << i << "] = ";
    Serial.print(free_space_list[i], BIN);
    Serial << endl;
  }
}

void FS::print_file_directory() {
  for (int i = 0; i < FILES_IN_DIRECTORY; ++i) {
    Serial << "file directory[" << i << "] = " << file_directory[i] << endl;
  }
}

void FS::reformat() {
  // initialize the first 2 bits to taken - taken by free list and directory
  free_space_list[0] = 0b00111111;
  for (int i = 1; i < FREE_LIST_SIZE; ++i) {
     // initialize each byte to 255 so each bit will be 1
    free_space_list[i] = 255;
  }
  for (int i = 0; i < FILES_IN_DIRECTORY; ++i) {
    // initialize directory pointers to NULL
    file_directory[i] = -1;
  }
}

void FS::initialize() {
  // read page 0 from EEPROM to fill file directory buffer
  eeprom.read_page(0, (byte*)file_directory);
  // read page 1 from EEPROM to fill free space list buffer
  eeprom.read_page(1, free_space_list);
  num_free_blocks = find_num_free_blocks();
}

int FS::find_first_free_block() {
  // returns the offset of the first block that is free
  int offset = 0;
  for (int i = 0; i < FREE_LIST_SIZE; ++i) {
    int comparator = 128;
    for (int k = 0; k < 8; ++k) {
      if ((free_space_list[i] & comparator) != 0) {
        return offset;
      } else {
        offset++;
        comparator = comparator >> 1;
      }
    }
  }

}

int FS::find_num_free_blocks() {
  //returns the total number of free blocks (total number of one's in bit string)
  int counter = 0;
  for (int i = 0; i < FREE_LIST_SIZE; ++i) {
    int j = 1;
    for (int k = 0; k < 8; ++k) {
      if ((free_space_list[i] & j) != 0) {
        counter++;
      }
      j = j << 1;
    }
  }
  num_free_blocks = counter;
  return counter;
}

void FS::flip_bit(int block_index, int offset) {
  int x = 128;
  free_space_list[block_index] = ((x >> offset) ^ free_space_list[block_index]);
}

void FS::commit_to_EEPROM() {
  eeprom.write_page(0, (byte*)file_directory);
  eeprom.write_page(1, free_space_list);
}

int FS::find_empty_directory_slot() {
  for (int i = 0; i < FILES_IN_DIRECTORY; ++i) {
    if (file_directory[i] == -1) {
      return i;
    }
  }
}

void FS::create_file(char *file_name) {
  num_free_blocks = find_num_free_blocks();
  if (num_free_blocks == 0) {
    Serial << "No free space" << endl;
    Serial.flush();
    exit(1);
  }
  if (num_files == 32) {
    Serial << "No more files can be added" << endl;
    Serial.flush();
    exit(1);
  }
  if (find_file_name(file_name)) {
    Serial << "File already exists" << endl;
    Serial.flush();
    return;
  }
  // find first free block
  int block_number = find_first_free_block();
  int block_index = block_number / 8;
  int block_offset = block_number % 8;
  // update the free space list by flipping that bit to occupied
  flip_bit(block_index, block_offset);
  int file_directory_spot = find_empty_directory_slot();
  // update file directory to show a file is occupying the slot
  file_directory[file_directory_spot] = block_number;
  strcpy(fcb->file_name, file_name);
  fcb->file_offset = 10;
  for (int i = 0; i < 16; ++i) {
    fcb->data_blocks[i] = -1;
  }
  eeprom.write_page(block_number, (byte*)fcb);
  num_files++;
}

bool FS::find_file_name(char *file_name) {
  for (int i = 0; i < FILES_IN_DIRECTORY; ++i) {
    if (file_directory[i] != -1) {
      eeprom.read_page(file_directory[i], (byte*)fcb);
      if (strcmp(fcb->file_name, file_name) == 0) {
        return true;
      }
    }
  }
  return false;
}

// create file
  // check name is okay
  // find free block
  // modify file control block
  // update directory
  // update free list
  // write free list and directory to EEPROM
  // write FCB block to the block dedicated by free list

// delete file
  // modify free list
  // update directory (change pointer to NULL or -1)
  // write to EEPROM

// list files
  // go through directory structure
  // grab FCB from EEPROM
  // bring in each FCB for each file
  // print FCB->file_name

// close file
  // write to EEPROM
  





