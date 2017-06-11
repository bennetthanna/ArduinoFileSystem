#include <Arduino.h>
#include "Microchip_24LC256.h"
#include "FCB.h"

//#define FREE_LIST_SIZE 64
//#define FILES_IN_DIRECTORY 32

class FS {
  private:
    // free list of 64 bytes = 512 bits = 1 bit per block
    byte free_space_list[64];
    // max 32 files
    int file_directory[32];
    Microchip_24LC256 eeprom;
    int num_free_blocks;
    // one instance of fcb
    FCB fcb;
    int num_files;

  public:
    FS();
    void reformat();
    void initialize();
    void create_file(char *file_name);
    void open_file(char *file_name);
    void write_file(byte* input_buffer, int count);
    void read_file();
    void seek_file();
    void close_file();
    void delete_file(char *file_name);
    void list_files();
    int find_first_free_block();
    int find_num_free_blocks();
    void flip_bit(int block_index, int offset);
    void commit_to_EEPROM();
    void print_free_list();
    void print_file_directory();
    int find_empty_directory_slot();
    void reset_fcb();
};
