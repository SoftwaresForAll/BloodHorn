#include "shell_history.h"
#include "compat.h"
#include <string.h>
static char history[16][128];
static int hist_count = 0;
int shell_history_add(const char* cmd) {
    if (hist_count < 16) {
        strncpy(history[hist_count++], cmd, 127);
        history[hist_count-1][127] = 0;
    }
    return 0;
}
const char* shell_history_get(int idx) {
    if (idx < 0 || idx >= hist_count) return 0;
    return history[idx];
} 