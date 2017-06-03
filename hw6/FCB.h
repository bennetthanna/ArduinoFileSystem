struct FCB {
    char file_name[26];
    int file_offset;
    int data_blocks[16];
    int block_number;
    int file_directory_index;
};
