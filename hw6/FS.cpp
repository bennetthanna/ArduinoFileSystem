#include "FS.h"

// 0 is file directory
// 1 is free space list

FS::FS():eeprom() {
  Serial.println(F("Inside constructor!"));
}

void FS::print_free_list() {
  for (int i = 0; i < 64; ++i) {
    Serial.print(F("free space list["));
    Serial.print(i);
    Serial.print(F("] = "));
    Serial.println(free_space_list[i], BIN);
  }
}

void FS::print_file_directory() {
  for (int i = 0; i < 32; ++i) {
    Serial.print(F("file directory["));
    Serial.print(i);
    Serial.print(F("] = "));
    Serial.println(file_directory[i]);
  }
}

void FS::reformat() {
  Serial.println(F("Formatting EEPROM..."));
  // initialize the first 2 bits to taken - taken by free list and directory
  free_space_list[0] = 0b00111111;
  for (int i = 1; i < 64; ++i) {
     // initialize each byte to 255 so each bit will be 1
    free_space_list[i] = 255;
  }
  for (int i = 0; i < 32; ++i) {
    // initialize directory pointers to -1
    file_directory[i] = -1;
  }
  num_files = 0;
//  for (int i = 0; i < 512; ++i) {
//    Serial.println(i);
//    byte clear_buffer[64];
//    memset(clear_buffer, 0, 64);
//    eeprom.write_page(i, clear_buffer);
//  }
}

void FS::initialize() {
  Serial.println(F("Initializing..."));
  // read block 0 from EEPROM to fill file directory buffer
  eeprom.read_page(0, (byte*)file_directory);
  // read block 1 from EEPROM to fill free space list buffer
  eeprom.read_page(1, free_space_list);
  num_free_blocks = find_num_free_blocks();
  return;
}

// returns the offset of the first block that is free
int FS::find_first_free_block() {
  int offset = 0;
  for (int i = 0; i < 64; ++i) {
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
  return -1;
}

//returns the total number of free blocks (total number of one's in bit string)
int FS::find_num_free_blocks() {
  int counter = 0;
  for (int i = 0; i < 64; ++i) {
    int j = 1;
    for (int k = 0; k < 8; ++k) {
      if ((free_space_list[i] & j) != 0) {
        counter++;
      }
      j = j << 1;
    }
  }
  return counter;
}

// flips the specified bit
void FS::flip_bit(int block_index, int offset) {
  int x = 128;
  free_space_list[block_index] = ((x >> offset) ^ free_space_list[block_index]);
  return;
}

// write the file directory and free space list to desired blocks on EEPROM
void FS::commit_to_EEPROM() {
  eeprom.write_page(0, (byte*)file_directory);
  eeprom.write_page(1, free_space_list);
  return;
}

// returns the next free file directory slot
int FS::find_empty_directory_slot() {
  for (int i = 0; i < 32; ++i) {
    if (file_directory[i] == -1) {
      return i;
    }
  }
}

// reset fcb elements
void FS::reset_fcb() {
  fcb.file_name[0] = 0;
  fcb.file_offset = -1;
  for (int i = 0; i < 16; ++i) {
    fcb.data_blocks[i] = -1;
  }
  fcb.block_number = -1;
  fcb.file_directory_index = -1;
}

// checks for available free blocks
// error checks file name
// finds the first fre block and changes the free list and directory accordingly
// updtaes the fcb and writes changes to eeprom
void FS::create_file(char *file_name) {
  num_free_blocks = find_num_free_blocks();
  for (int i = 0; i < 32; i++) {
    if (file_directory[i] != -1) {
      eeprom.read_page(file_directory[i], (byte*)&fcb);
      if (strcmp(fcb.file_name, file_name) == 0) {
        Serial.println(F("ERROR: File already exists"));
        return;
      }
    }
  }
  if (num_free_blocks < 1) {
    Serial.println(F("ERROR: No free blocks available"));
    return;
  }
  if (num_files > 31) {
    Serial.println(F("ERROR: No more files can be added"));
    return;
  }
  Serial.print(F("Creating file: "));
  Serial.println(file_name);
  int block_number = find_first_free_block();
  fcb.block_number = block_number;
  flip_bit((block_number / 8), (block_number % 8));
  int file_directory_slot = find_empty_directory_slot();
  fcb.file_directory_index = file_directory_slot;
  file_directory[file_directory_slot] = block_number;
  strcpy(fcb.file_name, file_name);
  fcb.file_offset = 0;
  for (int i = 0; i < 16; ++i) {
    fcb.data_blocks[i] = -1;
  }
  eeprom.write_page(block_number, (byte*)&fcb);
  commit_to_EEPROM();
  num_files++;
  return;
}

// lists all files in the directory
// prints out file size up to the next page boundary
  // example: if 23 bytes then puts 64 byte size
void FS::list_files() {
  Serial.println(F("Listing files..."));
  for (int i = 0; i < 32; ++i) {
    if (file_directory[i] != -1) {
      eeprom.read_page(file_directory[i], (byte*)&fcb);
      Serial.print(F("File: "));
      Serial.print(fcb.file_name);
      Serial.print(F(", "));
      int num_pages = 0;
      for (int j = 0; j < 16; ++j) {
        if (fcb.data_blocks[j] != -1) {
          num_pages++;
        }
      }
      Serial.print(num_pages * 64);
      Serial.println(F(" bytes"));
    }
  }
  return;
}

// error checks to make sure file exists
// updates the directory and free list according to where the file used to occupy
// clears the fcb
// commits the changes to EEPROM
void FS::delete_file(char *file_name) {
  for (int i = 0; i < 32; i++) {
    if (file_directory[i] != -1) {
      eeprom.read_page(file_directory[i], (byte*)&fcb);
      if (strcmp(fcb.file_name, file_name) == 0) {
        Serial.print(F("Deleting file: "));
        Serial.println(file_name);
        int block_number = fcb.block_number;
        flip_bit((block_number / 8), (block_number % 8));
        file_directory[fcb.file_directory_index] = -1;
        num_files--;
//        fcb.file_name[0] = 0;
//        fcb.file_offset = -1;
//        for (int i = 0; i < 16; ++i) {
//          fcb.data_blocks[i] = -1;
//        }
        reset_fcb();
        commit_to_EEPROM();
        return;
      }
    }
  }
  Serial.print(F("ERROR: File "));
  Serial.print(file_name);
  Serial.println(F(" not found"));
}

// error cecks the existence of the file
// brings the specified file into the fcb
void FS::open_file(char *file_name) {
  for (int i = 0; i < 32; i++) {
    if (file_directory[i] != -1) {
      eeprom.read_page(file_directory[i], (byte*)&fcb);
      if (strcmp(fcb.file_name, file_name) == 0) {
        Serial.println(F("Opening file..."));
        return;
      }
    }
  }
  Serial.print(F("ERROR: File "));
  Serial.print(file_name);
  Serial.println(F(" not found"));
}

// error checks for open file
// resets the fcb and commits to EEPROM
void FS::close_file() {
  if (fcb.file_offset != -1) {
    Serial.println(F("Closing file..."));
    reset_fcb();
    eeprom.write_page(fcb.block_number, (byte*)&fcb);
    commit_to_EEPROM();
  } else {
    Serial.println(F("ERROR: No file is currently open to close"));
  }
}

// error checks for open file
// sets file offset to 0 of whichever file is currently open
void FS::seek_file() {
  if (fcb.file_offset != -1) {
    Serial.println(F("Seeking to beginning of file..."));
    fcb.file_offset = 0;
    eeprom.write_page(fcb.block_number, (byte*)&fcb);
  } else {
    Serial.println(F("ERROR: No file is currently open to seek"));
  }
}

// error checks for open file
// writes count number of bytes to file that is currently open
// updates fcb data blocks, free list, and file offset
// commits changes to EEPROM
void FS::write_file(byte* input_buffer, int count) {
  Serial.println(F("Writing to file..."));
  byte buff[64];
  memset(buff, 0, 64);
  int num_overflows;
  if (fcb.file_offset != -1) {
    if ((fcb.file_offset + count) > 1024) {
      Serial.println(F("ERROR: Files can be no larger than 1024 bytes"));
      return;
    }
    num_overflows = ((fcb.file_offset % 64) + count) / 64;
    int data_block_num = fcb.file_offset / 64;
    if (fcb.data_blocks[data_block_num] != -1) {
      eeprom.read_page(fcb.data_blocks[data_block_num], (byte*)buff);
    } else {
      int block_num = find_first_free_block();
      fcb.data_blocks[data_block_num] = block_num;
      flip_bit((block_num / 8) , (block_num % 8));
    }
    int num_chars_written = 0;
    if (num_overflows == 0) {
      // just write count bytes instead of going to the 64th
      for (int i = (fcb.file_offset % 64); i < ((fcb.file_offset % 64) + count); i++) {
        buff[i] = input_buffer[i - (fcb.file_offset % 64)];
      }
//      strncat(buff, input_buffer, (count));
    } else {
      // write as much as you can until you hit 64th byte
      for (int i = (fcb.file_offset % 64); i < 64; i++) {
        buff[i] = input_buffer[i - (fcb.file_offset % 64)];
        num_chars_written++;
      }
//      strncat(buff, input_buffer, (64 - (fcb.file_offset % 64)));
//      num_chars_written += (64 - (fcb.file_offset % 64)); 
      eeprom.write_page(fcb.data_blocks[data_block_num], (byte*)buff);
      while (num_overflows > 0) {
        num_overflows--;
        // grab next data block
        data_block_num++;
        if (fcb.data_blocks[data_block_num] != -1) {
          memset(buff, 0, 64);
          eeprom.read_page(fcb.data_blocks[data_block_num], (byte*)buff);
        } else {
          memset(buff, 0, 64);
          int block_num = find_first_free_block();
          fcb.data_blocks[data_block_num] = block_num;
          flip_bit((block_num / 8) , (block_num % 8));
        }
        for (int i = 0; i < 64; i++) {
          buff[i] = input_buffer[i + num_chars_written];
        }
        eeprom.write_page(fcb.data_blocks[data_block_num], (byte*)buff);
        num_chars_written += 64;
      } 
    }
    fcb.file_offset += count;
    eeprom.write_page(fcb.data_blocks[data_block_num], (byte*)buff);
    eeprom.write_page(fcb.block_number, (byte*)&fcb);
    commit_to_EEPROM();
  } else {
    Serial.println(F("ERROR: No file is currently open to write to"));
  }
}

// error checks for open file
// reads the whole file of the current open file in the fcb
void FS::read_file() {
  byte buffy[64];
  if (fcb.file_name[0] != 0) {
    for (int i = 0; i < 16; ++i) {
      if (fcb.data_blocks[i] != -1) {
        memset(buffy, 0, 64);
        eeprom.read_page(fcb.data_blocks[i], (byte*)buffy);
        Serial.println((char*)buffy);
      }
    }
  } else {
    Serial.println(F("ERROR: No file is currently open to read"));
  }
}
