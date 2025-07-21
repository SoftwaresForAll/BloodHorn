#ifndef BLOODHORN_SHELL_NET_H
#define BLOODHORN_SHELL_NET_H
int shell_cmd_ping(const char* host, char* out, int maxlen);
int shell_cmd_ifconfig(char* out, int maxlen);
#endif 