struct FCB {
    char* file_name[30];
    int file_offset;
    int data_blocks[16];
};
