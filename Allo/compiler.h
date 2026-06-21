#ifndef allo_compiler_h
#define allo_compiler_h
#include <stdbool.h>

#include "object.h"
#include "virtual_machine.h"

ObjFunction* compile(const char* source);
void advance_compiler();
#endif
