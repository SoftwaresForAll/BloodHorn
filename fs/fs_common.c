#include "fs_common.h"
#include <string.h>
int fs_filename_cmp(const char* a, const char* b) {
    return strcasecmp(a, b);
}
void fs_path_split(const char* path, char* dir, char* file) {
    const char* slash = strrchr(path, '/');
    if (slash) {
        strncpy(dir, path, slash-path); dir[slash-path] = 0;
        strcpy(file, slash+1);
    } else {
        dir[0] = 0; strcpy(file, path);
    }
} 