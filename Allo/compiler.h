#ifndef allo_compiler_h
#define allo_compiler_h
#include <stdbool.h>
#include "virtual_machine.h"

bool compile(const char* source, Chunk* chunk);
void advance_compiler();
#endif
