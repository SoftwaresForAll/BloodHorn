#ifndef BLOODHORN_SCRIPT_ENV_H
#define BLOODHORN_SCRIPT_ENV_H
int script_env_set(const char* key, const char* value);
const char* script_env_get(const char* key);
#endif 