//
//  instructionset.h
//  libs/shared
//
//  Created by Zander Otavka on 8/6/16
//  Copyright (c) 2016 Zander Otavka All Rights Reserved.
//


#ifndef zee_instructionset_h
#define zee_instructionset_h

#include "util.h"


#define CONST(n) __const_##n
#define MAKE_CONST(n, s, v...) \
    static const Constant __const_##n = { s, { v } };

#define INSTR(n) __instr_getter_##n()
#define MAKE_INSTR(n) \
    Byte __instr_getter_##n(void);


// Readable type sizes
MAKE_CONST(INT_SIZE, 2, 0x00, 0x04);
MAKE_CONST(FLOAT_SIZE, 2, 0x00, 0x04);


// Leave room for as many instructions as we could possibly want
MAKE_INSTR(see_next_1_byte); // <instruction_set_b:1> <instruction_args:?>
MAKE_INSTR(see_next_2_bytes); // <instruction_set_c:2> <instruction_args:?>

// Malloc size memory and put pointer in op slot4
MAKE_INSTR(op_0_malloc); // <size:2>
MAKE_INSTR(op_1_malloc); // <size:2>
MAKE_INSTR(op_0_big_malloc); // <size:8>
MAKE_INSTR(op_1_big_malloc); // <size:8>

// Dereference pointer in op slot and set value
MAKE_INSTR(set_op_0_val); // <size:2> <value:size>
MAKE_INSTR(set_op_1_val); // <size:2> <value:size>

// Put pointer at stack slot into op slot
MAKE_INSTR(set_op_0_from_stack_8); // <stack_index:1>
MAKE_INSTR(set_op_0_from_stack_16); // <stack_index:1>
MAKE_INSTR(set_op_0_from_stack_32); // <stack_index:1>
MAKE_INSTR(set_op_0_from_stack_64); // <stack_index:1>
MAKE_INSTR(set_op_1_from_stack_8); // <stack_index:1>
MAKE_INSTR(set_op_1_from_stack_16); // <stack_index:1>
MAKE_INSTR(set_op_1_from_stack_32); // <stack_index:1>
MAKE_INSTR(set_op_1_from_stack_64); // <stack_index:1>
MAKE_INSTR(set_op_0_from_big_stack_8); // <stack_index:4>
MAKE_INSTR(set_op_0_from_big_stack_16); // <stack_index:4>
MAKE_INSTR(set_op_0_from_big_stack_32); // <stack_index:4>
MAKE_INSTR(set_op_0_from_big_stack_64); // <stack_index:4>
MAKE_INSTR(set_op_1_from_big_stack_8); // <stack_index:4>
MAKE_INSTR(set_op_1_from_big_stack_16); // <stack_index:4>
MAKE_INSTR(set_op_1_from_big_stack_32); // <stack_index:4>
MAKE_INSTR(set_op_1_from_big_stack_64); // <stack_index:4>

// Put pointer at op slot into stack slot
MAKE_INSTR(set_stack_from_op_0_8); // <stack_index:1>
MAKE_INSTR(set_stack_from_op_0_16); // <stack_index:1>
MAKE_INSTR(set_stack_from_op_0_32); // <stack_index:1>
MAKE_INSTR(set_stack_from_op_0_64); // <stack_index:1>
MAKE_INSTR(set_stack_from_op_1_8); // <stack_index:1>
MAKE_INSTR(set_stack_from_op_1_16); // <stack_index:1>
MAKE_INSTR(set_stack_from_op_1_32); // <stack_index:1>
MAKE_INSTR(set_stack_from_op_1_64); // <stack_index:1>
MAKE_INSTR(set_big_stack_from_op_0_8); // <stack_index:4>
MAKE_INSTR(set_big_stack_from_op_0_16); // <stack_index:4>
MAKE_INSTR(set_big_stack_from_op_0_32); // <stack_index:4>
MAKE_INSTR(set_big_stack_from_op_0_64); // <stack_index:4>
MAKE_INSTR(set_big_stack_from_op_1_8); // <stack_index:4>
MAKE_INSTR(set_big_stack_from_op_1_16); // <stack_index:4>
MAKE_INSTR(set_big_stack_from_op_1_32); // <stack_index:4>
MAKE_INSTR(set_big_stack_from_op_1_64); // <stack_index:4>

// Do an integer operation, op_1 on op_0, and put the result in op_0 
MAKE_INSTR(op_int_add);
MAKE_INSTR(op_int_sub);
MAKE_INSTR(op_int_mul);
MAKE_INSTR(op_int_div);
MAKE_INSTR(op_int_mod);

// Print the object pointed to by op_0
MAKE_INSTR(op_0_print);


void instructionset_init(void);
void instructionset_deinit(void);

Byte instructionset_get_bytecode(const char* name, bool* is_invalid);
const Constant* instructionset_get_constant(const char* name, bool* is_invalid);
size_t instructionset_exec(const Byte* bytecode);

#endif // zee_instructionset_h
