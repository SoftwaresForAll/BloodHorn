#ifndef BLOODHORN_SHELL_HISTORY_H
#define BLOODHORN_SHELL_HISTORY_H
int shell_history_add(const char* cmd);
const char* shell_history_get(int idx);
#endif 