#include "FS.h"

// write to eeprom every time you create or delete a file

// 0 is file directory
// 1 is free space list

FS::FS():eeprom() {
  Serial.println("Inside constructor!");
  num_files = 0;
  return;
}

void FS::print_free_list() {
  for (int i = 0; i < FREE_LIST_SIZE; ++i) {
    Serial.print("free space list[");
    Serial.print(i);
    Serial.print("] = ");
    Serial.println(free_space_list[i], BIN);
  }
  return;
}

void FS::print_file_directory() {
  for (int i = 0; i < 32; ++i) {
    Serial.print("file directory[");
    Serial.print(i);
    Serial.print("] = ");
    Serial.println(file_directory[i]);
  }
  return;
}

void FS::reformat() {
  Serial.println("Formatting EEPROM...");
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
  return;
}

void FS::initialize() {
  Serial.println("Initializing...");
  // read block 0 from EEPROM to fill file directory buffer
  eeprom.read_page(0, (byte*)file_directory);
  // read block 1 from EEPROM to fill free space list buffer
  eeprom.read_page(1, free_space_list);
  num_free_blocks = find_num_free_blocks();
  return;
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
  // should error check for -1 return value - signals no free blocks
  return -1;
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
  //num_free_blocks = counter;
  return counter;
}

void FS::flip_bit(int block_index, int offset) {
  int x = 128;
  free_space_list[block_index] = ((x >> offset) ^ free_space_list[block_index]);
  return;
}

void FS::commit_to_EEPROM() {
  eeprom.write_page(0, (byte*)file_directory);
  eeprom.write_page(1, free_space_list);
  return;
}

int FS::find_empty_directory_slot() {
  for (int i = 0; i < FILES_IN_DIRECTORY; ++i) {
    if (file_directory[i] == -1) {
      return i;
    }
  }
}

void FS::reset_fcb() {
  fcb.file_name[0] = 0;
  fcb.file_offset = -1;
  for (int i = 0; i < 16; ++i) {
    fcb.data_blocks[i] = -1;
  }
  fcb.block_number = -1;
  fcb.file_directory_index = -1;
}

void FS::create_file(char *file_name) {
//  reset_fcb();
  num_free_blocks = find_num_free_blocks();
  if (num_free_blocks < 1) {
    Serial.println("ERROR: No free blocks available");
    return;
  }
  if (num_files > 31) {
    Serial.println("ERROR: No more files can be added");
    return;
  }
  for (int i = 0; i < FILES_IN_DIRECTORY; i++) {
    if (file_directory[i] != -1) {
      if (strcmp(fcb.file_name, file_name) == 0) {
        Serial.println("ERROR: File already exists");
        return;
      }
    }
  }
  Serial.print("Creating file: ");
  Serial.println(file_name);
  // find first free block
  int block_number = find_first_free_block();
  fcb.block_number = block_number;
  int block_index = block_number / 8;
  int block_offset = block_number % 8;
  // update the free space list by flipping that bit to occupied
  flip_bit(block_index, block_offset);
  int file_directory_slot = find_empty_directory_slot();
  fcb.file_directory_index = file_directory_slot;
  // update file directory to show a file is occupying the slot
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

void FS::list_files() {
  Serial.println("Listing files...");
  for (int i = 0; i < FILES_IN_DIRECTORY; ++i) {
    if (file_directory[i] != -1) {
      eeprom.read_page(file_directory[i], (byte*)&fcb);
      Serial.print("File: ");
      Serial.print(fcb.file_name);
      Serial.print(", ");
      // this is wrong. the offset might be 0 after seeking to begin
      Serial.print(fcb.file_offset);
      Serial.println(" bytes");
    }
  }
  return;
}

void FS::delete_file(char *file_name) {
  for (int i = 0; i < FILES_IN_DIRECTORY; i++) {
    if (file_directory[i] != -1) {
      if (strcmp(fcb.file_name, file_name) == 0) {
        Serial.print("Deleting file: ");
        Serial.println(file_name);
        int block_number = fcb.block_number;
        int block_index = block_number / 8;
        int block_offset = block_number % 8;
        flip_bit(block_index, block_offset);
        file_directory[fcb.file_directory_index] = -1;
        num_files--;
        fcb.file_name[0] = 0;
        fcb.file_offset = 0;
        for (int i = 0; i < 16; ++i) {
          fcb.data_blocks[i] = -1;
        }
        commit_to_EEPROM();
      }
    }
  }
  return;
  Serial.print("ERROR: File ");
  Serial.print(file_name);
  Serial.println(" not found");
}

void FS::open_file(char *file_name) {
  for (int i = 0; i < FILES_IN_DIRECTORY; i++) {
    if (file_directory[i] != -1) {
      if (strcmp(fcb.file_name, file_name) == 0) {
        Serial.println("Opening file...");
        return;
      }
    }
  }
  Serial.print("ERROR: File ");
  Serial.print(file_name);
  Serial.println(" not found");
  return;
}

void FS::close_file() {
  if (fcb.file_name[0] != 0) {
    Serial.println("Closing file...");
    reset_fcb();
    eeprom.write_page(fcb.block_number, (byte*)&fcb);
    commit_to_EEPROM();
  } else {
    Serial.println("ERROR: no file is currently open to close");
  }
}

void FS::seek_file() {
  if (fcb.file_name[0] != 0) {
    Serial.println("Seeking to beginning of file...");
    fcb.file_offset = 0;
    eeprom.write_page(fcb.block_number, (byte*)&fcb);
    Serial.println(fcb.file_offset);
  } else {
    Serial.println("ERROR: no file is currently open to seek");
  }
}

void FS::write_file(byte* input_buffer, int count) {
  // for loop to do byte by byte
  Serial.println("Writing to file...");
  byte buff[64];
  memset(buff, 0, 64);
  int num_overflows;
  if (count > 1024) {
    Serial.println("ERROR: Files can be no larger than 1024 bytes");
    return;
  }
  if (fcb.file_name[0] != 0) {
    if ((fcb.file_offset + count) > 1024) {
      Serial.println("ERROR: Files can be no larger than 1024 bytes");
      return;
    }
    num_overflows = ((fcb.file_offset % 64) + count) / 64;
    int data_block_num = fcb.file_offset / 64;
//    Serial.print("data block num = ");
//    Serial.println(data_block_num);
    if (fcb.data_blocks[data_block_num] != -1) {
//      Serial.println("fcb.data_blocks != -1");
//      Serial.println(fcb.data_blocks[data_block_num]);
      memset(buff, 0, 64);
      eeprom.read_page(fcb.data_blocks[data_block_num], (byte*)buff);
    } else {
      memset(buff, 0, 64);
      int block_num = find_first_free_block();
//      Serial.print("block num = ");
//      Serial.println(block_num);
      fcb.data_blocks[data_block_num] = block_num;
      int index = find_empty_directory_slot();
//      Serial.print("index = ");
//      Serial.println(index);
      file_directory[index] = block_num;
      flip_bit((block_num / 8) , (block_num % 8));
    }
    int num_chars_written = 0;
    if (num_overflows == 0) {
      // just write count bytes instead of going to the 64th
      for (int i = (fcb.file_offset % 64); i < ((fcb.file_offset % 64) + count); i++) {
        buff[i] = input_buffer[i - (fcb.file_offset % 64)];
      }
    } else {
      // write as much as you can until you hit 64th byte
      for (int i = (fcb.file_offset % 64); i < 64; i++) {
        buff[i] = input_buffer[i - (fcb.file_offset % 64)];
        num_chars_written++;
      }
      eeprom.write_page(fcb.data_blocks[data_block_num], (byte*)buff);
      while (num_overflows >= 0) {
        num_overflows--;
        // grab next data block
        data_block_num++;
//        Serial.print("data block num = ");
//        Serial.println(data_block_num);
        if (fcb.data_blocks[data_block_num] != -1) {
//          Serial.println("fcb.data_blocks != -1");
//          Serial.println(fcb.data_blocks[data_block_num]);
          memset(buff, 0, 64);
          eeprom.read_page(fcb.data_blocks[data_block_num], (byte*)buff);
        } else {
          memset(buff, 0, 64);
          int block_num = find_first_free_block();
//          Serial.print("block num = ");
//          Serial.println(block_num);
          fcb.data_blocks[data_block_num] = block_num;
          int index = find_empty_directory_slot();
//          Serial.print("index = ");
//          Serial.println(index);
          file_directory[index] = block_num;
          flip_bit((block_num / 8) , (block_num % 8));
        }
        for (int i = 0; i < 64; i++) {
          buff[i] = input_buffer[i + num_chars_written];
        }
        eeprom.write_page(fcb.data_blocks[data_block_num], (byte*)buff);
        num_chars_written += 64;
      } 
    }
//    Serial.print("buffer = ");
//    Serial.println((char*)buff);
//    Serial.print("input buffer = ");
//    Serial.println((char*)input_buffer);
    fcb.file_offset += count;
//    Serial.print("file offset = ");
//    Serial.println(fcb.file_offset);
    eeprom.write_page(fcb.data_blocks[data_block_num], (byte*)buff);
    eeprom.write_page(fcb.block_number, (byte*)&fcb);
    commit_to_EEPROM();
  } else {
    Serial.println("ERROR: no file is currently open to write to");
    return;
  }
}

// reads the whole file of the current file in the fcb
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
    Serial.println("ERROR: no file currently open to read");
  }
}