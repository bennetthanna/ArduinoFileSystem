#include "FS.h"

// write to eeprom every time you create or delete a file

// 0 is file directory
// 1 is free space list

FS::FS():eeprom() {
  Serial.println("Inside constructor!");
  num_files = 0;
  return;
}

bool FS::load_in_fcb(char *file_name) {
  for (int i = 0; i < FILES_IN_DIRECTORY; i++) {
    if (file_directory[i] != -1) {
      Serial.println("file_directory[i] != -1");
      Serial.println(file_directory[i]);
      eeprom.read_page(file_directory[i], (byte*)fcb);
      Serial.print("loaded in fcb file name = ");
      Serial.println(fcb->file_name);
      Serial.print("loaded in fcb file offset = ");
      Serial.println(fcb->file_offset);
      Serial.print("loaded in fcb block number = ");
      Serial.println(fcb->block_number);
      Serial.print("loaded in fcb file directory index = ");
      Serial.println(fcb->file_directory_index);
      for (int j = 0; j < 16; j++) {
        Serial.print("data_blocks[");
        Serial.print(j);
        Serial.print("] = ");
        Serial.println(fcb->data_blocks[j]);
      }
      if (strcmp(fcb->file_name, file_name) == 0) {
        return true;
      }
    }
  }
  return false;
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
  fcb->file_name[0] = 0;
  fcb->file_offset = -1;
  for (int i = 0; i < 16; ++i) {
    fcb->data_blocks[i] = -1;
  }
  fcb->block_number = -1;
  fcb->file_directory_index = -1;
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
  if (load_in_fcb(file_name)) {
    Serial.println("ERROR: File already exists");
    return;
  }
  Serial.print("Creating file: ");
  Serial.println(file_name);
  // find first free block
  int block_number = find_first_free_block();
  fcb->block_number = block_number;
  Serial.println(fcb->block_number);
  int block_index = block_number / 8;
  int block_offset = block_number % 8;
  // update the free space list by flipping that bit to occupied
  flip_bit(block_index, block_offset);
  int file_directory_slot = find_empty_directory_slot();
  fcb->file_directory_index = file_directory_slot;
  Serial.println(fcb->file_directory_index);
  // update file directory to show a file is occupying the slot
  file_directory[file_directory_slot] = block_number;
  strcpy(fcb->file_name, file_name);
  Serial.println(fcb->file_name);
  fcb->file_offset = 0;
  Serial.println(fcb->file_offset);
  for (int i = 0; i < 16; ++i) {
    fcb->data_blocks[i] = -1;
  }
  eeprom.write_page(block_number, (byte*)fcb);
  commit_to_EEPROM();
  num_files++;
  return;
}

void FS::list_files() {
  Serial.println("Listing files...");
  for (int i = 0; i < FILES_IN_DIRECTORY; ++i) {
    if (file_directory[i] != -1) {
      eeprom.read_page(file_directory[i], (byte*)fcb);
      Serial.print("File: ");
      Serial.print(fcb->file_name);
      Serial.print(", ");
      // this is wrong. the offset might be 0 after seeking to begin
      Serial.print(fcb->file_offset);
      Serial.println(" bytes");
    }
  }
  return;
}

void FS::delete_file(char *file_name) {
  if (load_in_fcb(file_name)) {
    Serial.print("Deleting file: ");
    Serial.println(file_name);
    int block_number = fcb->block_number;
    int block_index = block_number / 8;
    int block_offset = block_number % 8;
    flip_bit(block_index, block_offset);
    file_directory[fcb->file_directory_index] = -1;
    num_files--;
    fcb->file_name[0] = 0;
    fcb->file_offset = 0;
    for (int i = 0; i < 16; ++i) {
      fcb->data_blocks[i] = -1;
    }
    commit_to_EEPROM();
    return;
  } else {
    Serial.print("ERROR: File ");
    Serial.print(file_name);
    Serial.println(" not found");
    return;
  }
}

void FS::open_file(char *file_name) {
  for (int i = 0; i < FILES_IN_DIRECTORY; i++) {
    if (file_directory[i] != -1) {
      if (strcmp(fcb->file_name, file_name) == 0) {
        Serial.println("File opened");
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
  reset_fcb();
  eeprom.write_page(fcb->block_number, (byte*)fcb);
  commit_to_EEPROM();
}

void FS::seek_file() {
  if (fcb->file_name[0] != 0) {
    Serial.println("Seeking to beginning of file");
    fcb->file_offset = 0;
    eeprom.write_page(fcb->block_number, (byte*)fcb);
    Serial.println(fcb->file_offset);
  } else {
    Serial.println("ERROR: no file is currently open to seek");
  }
}

// writes to current file that is open in fcb
void FS::write_file(byte* input_buffer, int count) {
  Serial.println("Inside write file");
  buff[0] = 0;
  if (count > 1024) {
    Serial.println("ERROR: Files can be no larger than 1024 bytes");
    return;
  }
  Serial.println("here");
  if (fcb->file_name[0] != 0) {
    if ((fcb->file_offset + count) > 1024) {
      Serial.println("ERROR: Files can be no larger than 1024 bytes");
      return;
    }
    int data_block_num = fcb->file_offset / 64;
    Serial.print("data block num = ");
    Serial.println(data_block_num);
    if (fcb->data_blocks[data_block_num] != -1) {
      Serial.println("fcb->data_blocks != -1");
      Serial.println(fcb->data_blocks[data_block_num]);
      eeprom.read_page(fcb->data_blocks[data_block_num], (byte*)buff);
    } else {
      int block_num = find_first_free_block();
      Serial.print("block num = ");
      Serial.println(block_num);
      fcb->data_blocks[data_block_num] = block_num;
      int index = find_empty_directory_slot();
      Serial.print("index = ");
      Serial.println(index);
      file_directory[index] = block_num;
      flip_bit((block_num / 8) , (block_num % 8));
      Serial.println("here");
    }
    strncat((char*)buff, (char*)input_buffer, count);
    Serial.println("after strncat");
    Serial.println(fcb->file_offset);
    fcb->file_offset += count;
    Serial.print("file offset = ");
    Serial.println(fcb->file_offset);
    Serial.print("buffer = ");
    Serial.println((char*)buff);
    Serial.print("input buffer = ");
    Serial.println((char*)input_buffer);
    eeprom.write_page(fcb->data_blocks[data_block_num], (byte*)buff);
    eeprom.write_page(fcb->block_number, (byte*)fcb);
    commit_to_EEPROM();
  } else {
    Serial.println("ERROR: no file is currently open to write to");
    return;
  }
}

// reads the whole file of the current file in the fcb
void FS::read_file() {
  byte heres_another_buffer[64];
  heres_another_buffer[0] = 0;
  if (fcb->file_name[0] != 0) {
    for (int i = 0; i < 16; ++i) {
      if (fcb->data_blocks[i] != -1) {
        eeprom.read_page(fcb->data_blocks[i], (byte*)heres_another_buffer);
        Serial.println((char*)heres_another_buffer);
      }
    }
  } else {
    Serial.print("ERROR: no file currently open to read");
    return;
  }
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
  // QUESTION: do I delete the fcb block from EEPROM? how?

// list files
  // go through directory structure
  // grab FCB from EEPROM
  // bring in each FCB for each file
  // print FCB->file_name

// close file
  // write to EEPROM
  // set existing fcb to NULL
  // QUESTION: can you assume the file is currently open?
    // as in the file is the current fcb represented
    // so you can just write_page(block, (byte*)fcb)
      // how do you know what the block number to write to is?
      // make block number a fcb member variable
    // or do you have to pass it a file name and find that fcb then write it?

// open file
  // bring in the FCB from EEPROM
  // QUESTION: what else do I do?

// seek file
  // set fcb file offset to 0

// write file
  // takes fcb, byte *buffer, int counter
  // example: fs.write_file(fcb, "hello", 5)
  // grab a data block based on offset
    // if offset between 0 and 64 grab pointer 0
    // if offset between 64 and 128 grab pointer 1
  // if pointer is -1 then grab free block
  // update data block pointer
    // if offset is 67, get free block 17, then data_block[1] = 17
  // bring in data block into buffer
  // modify buffer
    // example: buffer = [hello..........]
    // pointer stuff
  // update offset
    // offset + counter
  // write to EEPROM
    // buffer to block 17 (data blocks)
    // free list
    // fcb
  // easy case: writing within one disk block (<= 64 bytes)
  // hard case: overflows into another disk block
    // if ((float)(offset + counter) / (float)64) > 1 then overflows

// read file
  // read in from offset not from beginning of file
  // pass it the number of bytes you want to read

// make helper function to bring in fcb based on file name
  // bool load_in_fcb(char *file_name)
  // if false then error
