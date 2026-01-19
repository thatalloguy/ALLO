#ifndef allo_value_h
#define allo_value_h

typedef double Value;

typedef struct {
    int capacity;
    int count;
    //note this can only hold up to 256 bytes.
    //todo implement OP_CONSTANT_LONG
    Value* values;
} ValueArray;

void init_value_array(ValueArray* array);
void write_value_array(ValueArray* array, Value value);
void free_value_array(ValueArray* array);

#endif