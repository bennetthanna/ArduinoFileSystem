#include <Arduino.h>
#include "Microchip_24LC256.h"
#include "FCB.h"

#define FREE_LIST_SIZE 64
#define FILES_IN_DIRECTORY 32
#define BLOCK_SIZE 64

class FS {
//  private:
  public:
    // 1 bit for each block of EEPROM memory
    // 512 blocks of memory
    // 64 bytes per block of memory, 8 bits per byte 
    // 512 bits per block of memory = 1 bit per block
    byte free_space_list[FREE_LIST_SIZE];
    // 32 pointers to file control blocks
    // each pointer = 2 bytes
    byte file_directory[FILES_IN_DIRECTORY * 2];
    Microchip_24LC256 eeprom;
    int num_free_blocks;
    FCB *fcb;

//  public:
    FS();
    // clear free space list and file directory
    void reformat();
    // bring free space list and file directory into memory
    void initialize();
    // create name file
    void create();
    // open a named file
    void open_file();
    // write bytes to a file    
    void write_file();
    // read bytes from a file
    void read_file();
    // seek to beginning of a file
    void seek_file();
    // close a file
    void close_file();
    // delete a named file
    void delete_file();
    // list all files in file system
    // name and page-aligned size
    void list();
    int find_first_free_block_offset(byte n);
    int find_num_free_blocks();
    // call this function with find_free_block_offset?
    void flip_bit(int block_index, int offset);
    void commit_to_EEPROM();
};
