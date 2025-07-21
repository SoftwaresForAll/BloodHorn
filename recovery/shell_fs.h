#ifndef BLOODHORN_SHELL_FS_H
#define BLOODHORN_SHELL_FS_H
int shell_cmd_ls_fat32(uint32_t lba, char* out, int maxlen);
int shell_cmd_cat_fat32(uint32_t lba, const char* filename, char* out, int maxlen);
int shell_cmd_ls_ext2(uint32_t lba, char* out, int maxlen);
int shell_cmd_cat_ext2(uint32_t lba, const char* filename, char* out, int maxlen);
#endif 