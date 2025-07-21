#ifndef BLOODHORN_CONFIG_JSON_H
#define BLOODHORN_CONFIG_JSON_H
struct config_json { char key[32]; char value[128]; };
int config_json_parse(const char* json, struct config_json* out);
#endif 