struct FCB {
  // file offset = 2 bytes
  // data blocks = 32 bytes
  // block number = 2 bytes
  // file directory index = 2 bytes
  // max file name of 26
  char file_name[26];
  int file_offset;
  int data_blocks[16];
  int block_number;
  int file_directory_index;
};
