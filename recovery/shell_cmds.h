#ifndef BLOODHORN_SHELL_CMDS_H
#define BLOODHORN_SHELL_CMDS_H
int shell_cmd_help(char* out, int maxlen);
int shell_cmd_clear(void);
int shell_cmd_reboot(void);
int shell_cmd_exit(void);
#endif 