#ifndef BLOODHORN_SHELL_H
#define BLOODHORN_SHELL_H

void shell_init(void);
void shell_parse_command(const char* input);
void shell_execute_command(void);
void shell_run(void);
EFI_STATUS shell_start(void);

#endif // BLOODHORN_SHELL_H 