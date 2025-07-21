#ifndef BLOODHORN_LUA_H
#define BLOODHORN_LUA_H

typedef int (*lua_callback)(void);

struct lua_function {
    char name[32];
    lua_callback callback;
};

void lua_init(void);
void lua_push(int value);
int lua_pop(void);
void lua_add_function(const char* name, lua_callback func);
int lua_call_function(const char* name);
int lua_execute_script(const char* script);

#endif // BLOODHORN_LUA_H 