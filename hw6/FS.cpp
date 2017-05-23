#include <Streaming.h>
#include "FS.h"

// write to eeprom every time you create or delete a file

// 0 is file directory
// 1 is free space list

FS::FS():eeprom() {
  Serial << "Inside constructor!" << endl;
}

void FS::reformat() {
  for (int i = 0; i < FREE_LIST_SIZE; ++i) {
    // initialize each byte to 255 so each bit will be 1
    free_space_list[i] = 255;
    // initialize directory pointers to NULL
    file_directory[i] = NULL;
  }
}

void FS::initialize() {
  // read page 0 from EEPROM to fill file directory buffer
  eeprom.read_page(0, file_directory);
  // read page 1 from EEPROM to fill free space list buffer
  eeprom.read_page(1, free_space_list);
  num_free_blocks = find_num_free_blocks();
}

int FS::find_first_free_block_offset(byte n) {
  // returns the offset of the first block that is free within a byte block
  int offset = 0;
  int comparator = 128;
  while (!(comparator & n)) {
    comparator = comparator >> 1;
    offset++;
  }
  return offset;
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
  eeprom.write_page(0, file_directory);
  eeprom.write_page(1, free_space_list);
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

// close
  // write to EEPROM
  





