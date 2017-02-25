//
//  instructionset.c
//  libs/shared
//
//  Created by Zander Otavka on 8/6/16
//  Copyright (c) 2016 Zander Otavka All Rights Reserved.
//

#include <string.h>

#include <hashmap/hashmap.h>

#include "globals.h"
#include "instructionset.h"

static const int INSTR_SET_LEN = BYTE_MAX_VAL + 1;

static Instruction* instructions;
static map_t bytecode;
static uint16_t curr_instrset_a_code = 0x00;


#define DEFINE_INSTRUCTION(a, n) \
    static Byte __instr_val_##n; \
    Byte __instr_getter_##n(void) { return __instr_val_##n; } \
    size_t __instr_action_##n(a)

#define DEFINE_OP_INSTRUCTION(a, o, n0, n1) \
    size_t __instr_proto_##n0(a, o); \
    DEFINE_INSTRUCTION(const Byte* args, n0) { return __instr_proto_##n0(args, op_0); } \
    DEFINE_INSTRUCTION(const Byte* args, n1) { return __instr_proto_##n0(args, op_1); } \
    size_t __instr_proto_##n0(a, o)

#define DEFINE_SIZE_INSTRUCTION(a, s, n8, n16, n32, n64) \
    size_t __instr_proto_##n8(a, s); \
    DEFINE_INSTRUCTION(const Byte* args, n8)  { return __instr_proto_##n8(args, 1);  } \
    DEFINE_INSTRUCTION(const Byte* args, n16) { return __instr_proto_##n8(args, 2); } \
    DEFINE_INSTRUCTION(const Byte* args, n32) { return __instr_proto_##n8(args, 4); } \
    DEFINE_INSTRUCTION(const Byte* args, n64) { return __instr_proto_##n8(args, 8); } \
    size_t __instr_proto_##n8(a, s)

#define DEFINE_OP_SIZE_INSTRUCTION(a, o, s, n0_8, n0_16, n0_32, n0_64, n1_8, n1_16, n1_32, n1_64) \
    size_t __instr_proto_compound_##n0_8(a, o, s); \
    DEFINE_SIZE_INSTRUCTION(const Byte* args, size_t size, n0_8, n0_16, n0_32, n0_64) { \
        return __instr_proto_compound_##n0_8(args, op_0, size); \
    } \
    DEFINE_SIZE_INSTRUCTION(const Byte* args, size_t size, n1_8, n1_16, n1_32, n1_64) { \
        return __instr_proto_compound_##n0_8(args, op_1, size); \
    } \
    size_t __instr_proto_compound_##n0_8(a, o, s)


#define REGISTER_CONSTANT(n) \
    { \
        int map_status = hashmap_put(bytecode, #n, (const any_t) &CONST(n)); \
        assert(map_status == MAP_OK); \
    }

#define REGISTER_INSTRUCTION(n) \
    { \
        assert(INSTR(n) < INSTR_SET_LEN); \
        assert(!instructions[INSTR(n)]); \
        instructions[INSTR(n)] = &__instr_action_##n; \
        int map_status = hashmap_put(bytecode, #n, (const any_t) (uintptr_t) INSTR(n)); \
        assert(map_status == MAP_OK); \
    }

#define REGISTER_INSTRUCTION_A(n) \
    { \
        assert(curr_instrset_a_code < 256); \
        __instr_val_##n = curr_instrset_a_code; \
        curr_instrset_a_code++; \
        REGISTER_INSTRUCTION(n); \
    }


DEFINE_INSTRUCTION(const Byte* args, see_next_1_byte) {
    return 0;
}

DEFINE_INSTRUCTION(const Byte* args, see_next_2_bytes) {
    return 0;
}


DEFINE_OP_INSTRUCTION(const Byte* args, Ptr op, op_0_malloc, op_1_malloc) {
    uint16_t size = *((uint16_t*) args);
#ifdef RUNTIME
    op = malloc(size);
#endif
    return sizeof size;
}

DEFINE_OP_INSTRUCTION(const Byte* args, Ptr op, op_0_big_malloc, op_1_big_malloc) {
    uint64_t size = *((uint64_t*) args);
#ifdef RUNTIME
    op = malloc(size);
#endif
    return sizeof size;
}


DEFINE_OP_INSTRUCTION(const Byte* args, Ptr op, set_op_0_val, set_op_1_val) {
    uint16_t size = *((uint16_t*) args);
    const Byte* value = args + sizeof size;
#ifdef RUNTIME
    memcpy(op, value, size);
#endif
    return sizeof size + (sizeof *value) * size;
}

DEFINE_OP_INSTRUCTION(const Byte* args, Ptr op, set_op_0_big_val, set_op_1_big_val) {
    uint64_t size = *((uint64_t*) args);
    const Byte* value = args + sizeof size;
#ifdef RUNTIME
    memcpy(op, value, size);
#endif
    return sizeof size + (sizeof *value) * size;
}


DEFINE_OP_SIZE_INSTRUCTION(const Byte* args, Ptr op, size_t size,
                           set_op_0_from_stack_8,
                           set_op_0_from_stack_16,
                           set_op_0_from_stack_32,
                           set_op_0_from_stack_64,
                           set_op_1_from_stack_8,
                           set_op_1_from_stack_16,
                           set_op_1_from_stack_32,
                           set_op_1_from_stack_64) {
    uint8_t stack_index = args[0];
#ifdef RUNTIME
    uint64_t stack_value = *((uint64_t*) (stack_head + stack_index));
    assert(size <= sizeof stack_value);
    stack_value >>= (sizeof stack_value - size) * BYTE_BIT_SIZE;
    memcpy(&op, &stack_value, sizeof stack_value);
#endif
    return sizeof stack_index;
}

DEFINE_OP_SIZE_INSTRUCTION(const Byte* args, Ptr op, size_t size,
                           set_op_0_from_big_stack_8,
                           set_op_0_from_big_stack_16,
                           set_op_0_from_big_stack_32,
                           set_op_0_from_big_stack_64,
                           set_op_1_from_big_stack_8,
                           set_op_1_from_big_stack_16,
                           set_op_1_from_big_stack_32,
                           set_op_1_from_big_stack_64) {
    uint32_t stack_index = ((uint32_t*) args)[0];
#ifdef RUNTIME
    uint64_t stack_value = *((uint64_t*) (stack_head + stack_index));
    assert(size <= sizeof stack_value);
    stack_value >>= (sizeof stack_value - size) * BYTE_BIT_SIZE;
    memcpy(&op, &stack_value, sizeof stack_value);
#endif
    return sizeof stack_index;
}


DEFINE_OP_SIZE_INSTRUCTION(const Byte* args, Ptr op, size_t size,
                           set_stack_from_op_0_8,
                           set_stack_from_op_0_16,
                           set_stack_from_op_0_32,
                           set_stack_from_op_0_64,
                           set_stack_from_op_1_8,
                           set_stack_from_op_1_16,
                           set_stack_from_op_1_32,
                           set_stack_from_op_1_64) {
    uint8_t stack_index = args[0];
#ifdef RUNTIME
    Byte* stack_loc = stack_head + stack_index;
    uint64_t op_contents = op;
    assert(size <= sizeof op_contents);
    op_contents <<= (sizeof stack_value - size) * BYTE_BIT_SIZE;
    memcpy(stack_loc, &op_contents, size);
#endif
    return sizeof stack_index;
}

DEFINE_OP_SIZE_INSTRUCTION(const Byte* args, Ptr op, size_t size,
                           set_big_stack_from_op_0_8,
                           set_big_stack_from_op_0_16,
                           set_big_stack_from_op_0_32,
                           set_big_stack_from_op_0_64,
                           set_big_stack_from_op_1_8,
                           set_big_stack_from_op_1_16,
                           set_big_stack_from_op_1_32,
                           set_big_stack_from_op_1_64) {
    uint32_t stack_index = ((uint32_t*) args)[0];
#ifdef RUNTIME
    Byte* stack_loc = stack_head + stack_index;
    uint64_t op_contents = op;
    assert(size <= sizeof op_contents);
    op_contents <<= (sizeof stack_value - size) * BYTE_BIT_SIZE;
    memcpy(stack_loc, &op_contents, size);
#endif
    return sizeof stack_index;
}


DEFINE_INSTRUCTION(const Byte* args, op_int_add) {
#ifdef RUNTIME
    op_0 = (int64_t) op_0 + (int64_t) op_1;
#endif
    return 0;
}

DEFINE_INSTRUCTION(const Byte* args, op_int_sub) {
#ifdef RUNTIME
    op_0 = (int64_t) op_0 - (int64_t) op_1;
#endif
    return 0;
}

DEFINE_INSTRUCTION(const Byte* args, op_int_mul) {
#ifdef RUNTIME
    op_0 = (int64_t) op_0 * (int64_t) op_1;
#endif
    return 0;
}

DEFINE_INSTRUCTION(const Byte* args, op_int_div) {
#ifdef RUNTIME
    op_0 = (int64_t) op_0 / (int64_t) op_1;
#endif
    return 0;
}

DEFINE_INSTRUCTION(const Byte* args, op_int_mod) {
#ifdef RUNTIME
    op_0 = (int64_t) op_0 % (int64_t) op_1;
#endif
    return 0;
}


void instructionset_init(void) {
    instructions = calloc(INSTR_SET_LEN, sizeof *instructions);
    bytecode = hashmap_new();
    assert(instructions);
    assert(bytecode);


    REGISTER_CONSTANT(INT_SIZE);
    REGISTER_CONSTANT(FLOAT_SIZE);


    REGISTER_INSTRUCTION_A(see_next_1_byte);
    REGISTER_INSTRUCTION_A(see_next_2_bytes);

    REGISTER_INSTRUCTION_A(op_0_malloc);
    REGISTER_INSTRUCTION_A(op_1_malloc);
    REGISTER_INSTRUCTION_A(op_0_big_malloc);
    REGISTER_INSTRUCTION_A(op_1_big_malloc);

    REGISTER_INSTRUCTION_A(set_op_0_val);
    REGISTER_INSTRUCTION_A(set_op_1_val);
    REGISTER_INSTRUCTION_A(set_op_0_big_val);
    REGISTER_INSTRUCTION_A(set_op_1_big_val);

    REGISTER_INSTRUCTION_A(set_op_0_from_stack_8);
    REGISTER_INSTRUCTION_A(set_op_0_from_stack_16);
    REGISTER_INSTRUCTION_A(set_op_0_from_stack_32);
    REGISTER_INSTRUCTION_A(set_op_0_from_stack_64);
    REGISTER_INSTRUCTION_A(set_op_1_from_stack_8);
    REGISTER_INSTRUCTION_A(set_op_1_from_stack_16);
    REGISTER_INSTRUCTION_A(set_op_1_from_stack_32);
    REGISTER_INSTRUCTION_A(set_op_1_from_stack_64);
    REGISTER_INSTRUCTION_A(set_op_0_from_big_stack_8);
    REGISTER_INSTRUCTION_A(set_op_0_from_big_stack_16);
    REGISTER_INSTRUCTION_A(set_op_0_from_big_stack_32);
    REGISTER_INSTRUCTION_A(set_op_0_from_big_stack_64);
    REGISTER_INSTRUCTION_A(set_op_1_from_big_stack_8);
    REGISTER_INSTRUCTION_A(set_op_1_from_big_stack_16);
    REGISTER_INSTRUCTION_A(set_op_1_from_big_stack_32);
    REGISTER_INSTRUCTION_A(set_op_1_from_big_stack_64);

    REGISTER_INSTRUCTION_A(set_stack_from_op_0_8);
    REGISTER_INSTRUCTION_A(set_stack_from_op_0_16);
    REGISTER_INSTRUCTION_A(set_stack_from_op_0_32);
    REGISTER_INSTRUCTION_A(set_stack_from_op_0_64);
    REGISTER_INSTRUCTION_A(set_stack_from_op_1_8);
    REGISTER_INSTRUCTION_A(set_stack_from_op_1_16);
    REGISTER_INSTRUCTION_A(set_stack_from_op_1_32);
    REGISTER_INSTRUCTION_A(set_stack_from_op_1_64);
    REGISTER_INSTRUCTION_A(set_big_stack_from_op_0_8);
    REGISTER_INSTRUCTION_A(set_big_stack_from_op_0_16);
    REGISTER_INSTRUCTION_A(set_big_stack_from_op_0_32);
    REGISTER_INSTRUCTION_A(set_big_stack_from_op_0_64);
    REGISTER_INSTRUCTION_A(set_big_stack_from_op_1_8);
    REGISTER_INSTRUCTION_A(set_big_stack_from_op_1_16);
    REGISTER_INSTRUCTION_A(set_big_stack_from_op_1_32);
    REGISTER_INSTRUCTION_A(set_big_stack_from_op_1_64);

    REGISTER_INSTRUCTION_A(op_int_add);
    REGISTER_INSTRUCTION_A(op_int_sub);
    REGISTER_INSTRUCTION_A(op_int_mul);
    REGISTER_INSTRUCTION_A(op_int_div);
    REGISTER_INSTRUCTION_A(op_int_mod);
}

void instructionset_deinit(void) {
    hashmap_free(bytecode);
    free(instructions);
}

Byte instructionset_get_bytecode(const char* name, bool* is_invalid) {
    assert(is_invalid != NULL);
    any_t instr_code;
    int status = hashmap_get(bytecode, name, &instr_code);
    if (status == MAP_MISSING) {
        *is_invalid = true;
        return 0;
    }

    assert(status == MAP_OK);
    assert((uintptr_t) instr_code < INSTR_SET_LEN);

    *is_invalid = false;
    return (Byte) instr_code;
}

const Constant* instructionset_get_constant(const char* name, bool* is_invalid) {
    assert(is_invalid != NULL);
    Constant* constant;
    int status = hashmap_get(bytecode, name, (any_t*) &constant);
    if (status == MAP_MISSING) {
        *is_invalid = true;
        return 0;
    }

    assert(status == MAP_OK);

    *is_invalid = false;
    return constant;
}

size_t instructionset_exec(const Byte* bytecode_array) {
    return instructions[*bytecode_array](bytecode_array + 1);
}
