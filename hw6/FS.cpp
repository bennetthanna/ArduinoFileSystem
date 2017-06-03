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

void FS::create_file(char *file_name) {
  num_free_blocks = find_num_free_blocks();
  Serial.println("here before num free blocks");
  if (num_free_blocks < 1) {
    Serial.println("ERROR: No free blocks available");
    return;
  }
  Serial.println("here before num files");
  if (num_files > 31) {
    Serial.println("ERROR: No more files can be added");
    return;
  }
  Serial.println("here before find file");
  if (find_file_name(file_name)) {
    Serial.println("ERROR: File already exists");
    return;
  }
  Serial.println("here after");
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
  fcb->file_offset = 0;
  for (int i = 0; i < 16; ++i) {
    fcb->data_blocks[i] = -1;
  }
  eeprom.write_page(block_number, (byte*)fcb);
  commit_to_EEPROM();
  num_files++;
  return;
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

void FS::list_files() {
  Serial.println("Listing files...");
  for (int i = 0; i < FILES_IN_DIRECTORY; ++i) {
    if (file_directory[i] != -1) {
      eeprom.read_page(file_directory[i], (byte*)fcb);
      Serial.print("File: ");
      Serial.print(fcb->file_name);
      Serial.print(", ");
      Serial.print(fcb->file_offset);
      Serial.println(" bytes");
    }
  }
  return;
}

void FS::delete_file(char *file_name) {
  for (int i = 0; i < FILES_IN_DIRECTORY; ++i) {
    if (file_directory[i] != -1) {
      eeprom.read_page(file_directory[i], (byte*)fcb);
      if (strcmp(fcb->file_name, file_name) == 0) {
        Serial.println("File found");
        int block_number = file_directory[i];
        int block_index = block_number / 8;
        int block_offset = block_number % 8;
        flip_bit(block_index, block_offset);
        file_directory[i] = -1;
        num_files--;
        fcb->file_name[0] = 0;
        Serial.print("Reset file name = ");
        Serial.println(fcb->file_name);
        fcb->file_offset = 0;
        for (int i = 0; i < 16; ++i) {
          fcb->data_blocks[i] = -1;
        }
        commit_to_EEPROM();
        return;
      }
    }
  }
  Serial.print("ERROR: File ");
  Serial.print(file_name);
  Serial.println(" not found");
  return;
}

void FS::open_file(char *file_name) {
  for (int i = 0; i < FILES_IN_DIRECTORY; ++i) {
    if (file_directory[i] != -1) {
      eeprom.read_page(file_directory[i], (byte*)fcb);
      if (strcmp(fcb->file_name, file_name) == 0) {
        Serial.println("File found");
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
  commit_to_EEPROM();
}

void FS::seek_file(char *file_name) {
  for (int i = 0; i < FILES_IN_DIRECTORY; ++i) {
    if (file_directory[i] != -1) {
      eeprom.read_page(file_directory[i], (byte*)fcb);
      if (strcmp(fcb->file_name, file_name) == 0) {
        Serial.println("File found");
        fcb->file_offset = 0;
        return;
      }
    }
  }
  Serial.print("ERROR: File ");
  Serial.print(file_name);
  Serial.println(" not found");
  return;
}

void FS::write_file(char *file_name, byte* input_buffer, int count) {
  byte buffer[64];
  buffer[0] = 0;
  if (count > 1024) {
    Serial.println("ERROR: Files can be no larger than 1024 bytes");
    return;
  }
  for (int i = 0; i < FILES_IN_DIRECTORY; ++i) {
    if (file_directory[i] != -1) {
      eeprom.read_page(file_directory[i], (byte*)fcb);
      if (strcmp(fcb->file_name, file_name) == 0) {
        Serial.println("File found");
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
          eeprom.read_page(fcb->data_blocks[data_block_num], (byte*)buffer);
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
        strncat((char*)buffer, (char*)input_buffer, count);
        Serial.println("after strncat");
        Serial.println(fcb->file_offset);
        fcb->file_offset += count;
        Serial.print("file offset = ");
        Serial.println(fcb->file_offset);
        Serial.print("buffer = ");
        Serial.println((char*)buffer);
        Serial.print("input buffer = ");
        Serial.println((char*)input_buffer);
        eeprom.write_page(fcb->data_blocks[data_block_num], (byte*)buffer);
        eeprom.write_page(file_directory[i], (byte*)fcb);
        commit_to_EEPROM();
      }
    }
  }
}

void FS::read_file(char *file_name) {
  byte heres_another_buffer[60];
  heres_another_buffer[0] = 0;
  for (int i = 0; i < FILES_IN_DIRECTORY; ++i) {
    if (file_directory[i] != -1) {
      eeprom.read_page(file_directory[i], (byte*)fcb);
      if (strcmp(fcb->file_name, file_name) == 0) {
        Serial.println("File found");
        for (int i = 0; i < 16; ++i) {
          if (fcb->data_blocks[i] != -1) {
            Serial.println("data block != -1");
            eeprom.read_page(fcb->data_blocks[i], (byte*)heres_another_buffer);
            Serial.println((char*)heres_another_buffer);
          }
        }
        return;
      }
    }
  }
  Serial.print("ERROR: File ");
  Serial.print(file_name);
  Serial.println(" not found");
  return;
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
