#ifndef BLOODHORN_FS_COMMON_H
#define BLOODHORN_FS_COMMON_H
int fs_filename_cmp(const char* a, const char* b);
void fs_path_split(const char* path, char* dir, char* file);
#endif 