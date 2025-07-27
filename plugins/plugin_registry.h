#ifndef BLOODHORN_PLUGIN_REGISTRY_H
#define BLOODHORN_PLUGIN_REGISTRY_H
#include <stdint.h>
#include "compat.h"
int plugin_registry_register(const char* name, void* data, uint32_t size);
 