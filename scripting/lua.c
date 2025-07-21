#include <stdint.h>
#include <string.h>
#include "lua.h"

#define LUA_STACK_SIZE 256
#define LUA_MAX_FUNCTIONS 64

struct lua_state {
    int stack[LUA_STACK_SIZE];
    int stack_top;
    struct lua_function functions[LUA_MAX_FUNCTIONS];
    int function_count;
};

static struct lua_state lua_global_state;

void lua_init(void) {
    lua_global_state.stack_top = 0;
    lua_global_state.function_count = 0;
}

void lua_push(int value) {
    if (lua_global_state.stack_top < LUA_STACK_SIZE) {
        lua_global_state.stack[lua_global_state.stack_top++] = value;
    }
}

int lua_pop(void) {
    if (lua_global_state.stack_top > 0) {
        return lua_global_state.stack[--lua_global_state.stack_top];
    }
    return 0;
}

void lua_add_function(const char* name, lua_callback func) {
    if (lua_global_state.function_count < LUA_MAX_FUNCTIONS) {
        strcpy(lua_global_state.functions[lua_global_state.function_count].name, name);
        lua_global_state.functions[lua_global_state.function_count].callback = func;
        lua_global_state.function_count++;
    }
}

int lua_call_function(const char* name) {
    for (int i = 0; i < lua_global_state.function_count; i++) {
        if (strcmp(lua_global_state.functions[i].name, name) == 0) {
            return lua_global_state.functions[i].callback();
        }
    }
    return 0;
}

int lua_execute_script(const char* script) {
    char* token = strtok((char*)script, " \n");
    while (token) {
        if (strcmp(token, "push") == 0) {
            token = strtok(NULL, " \n");
            if (token) lua_push(atoi(token));
        } else if (strcmp(token, "add") == 0) {
            int b = lua_pop();
            int a = lua_pop();
            lua_push(a + b);
        } else if (strcmp(token, "sub") == 0) {
            int b = lua_pop();
            int a = lua_pop();
            lua_push(a - b);
        } else if (strcmp(token, "call") == 0) {
            token = strtok(NULL, " \n");
            if (token) lua_call_function(token);
        }
        token = strtok(NULL, " \n");
    }
    return lua_pop();
} 