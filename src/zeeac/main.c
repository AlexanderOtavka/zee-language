//
//  main.c
//  src/bytecode_compiler
//
//  Created by Zander Otavka on 8/6/16
//  Copyright (c) 2016 Zander Otavka All Rights Reserved.
//

#include <stdio.h>
#include <string.h>

#include <shared/util.h>
#include <shared/io.h>
#include <shared/instructionset.h>

static const size_t CHUNK_SIZE = 100;
static size_t allocated_len = CHUNK_SIZE;
static size_t bytecode_len = 0;
static Byte* bytecode;

void throw_syntax_error(char* input, char* error_location, char* message) {
    char* line_start = error_location;
    while (line_start > input && line_start[-1] != '\n') line_start--;

    char* line_end = error_location;
    while (line_end && *line_end != '\n') line_end++;

    size_t line_len = line_end - line_start;
    char line_buffer[line_len + 1];
    line_buffer[line_len] = '\0';
    memcpy(line_buffer, line_start, line_len);
    assert(line_buffer[line_len] == '\0');

    int line_num = 1;
    for (char* letter = line_start; letter > input; letter--) {
        if (*letter == '\n') {
            line_num++;
        }
    }

    int column_index = error_location - line_start;
    char padding_buffer[column_index + 1];
    padding_buffer[column_index] = '\0';
    memset(padding_buffer, ' ', column_index);

    fprintf(stderr, "Syntax Error (%d:%d): %s\n%s\n%s^\n", line_num, column_index + 1, message, line_buffer, padding_buffer);
    exit(EXIT_FAILURE);
}

bool is_hex(char letter) {
    return ('0' <= letter && letter <= '9') || ('a' <= letter && letter <= 'f');
}

Byte parse_hex(char digit, bool* error) {
    switch(digit) {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '9': return 9;
        case 'a': return 10;
        case 'b': return 11;
        case 'c': return 12;
        case 'd': return 13;
        case 'e': return 14;
        case 'f': return 15;
        default:
            if (error) *error = true;
            return 0;
    }
}

bool is_const_letter(char letter) {
    return ('A' <= letter && letter <= 'Z');
}

bool is_instr_letter(char letter) {
    return ('a' <= letter && letter <= 'z');
}

size_t get_identifier_len(char* input, char* letter, bool (*is_letter)(char)) {
    size_t len = 0;
    while (is_letter(letter[len]) || ('0' <= letter[len] && letter[len] <= '9') || letter[len] == '_') len++;
    return len;
}

void add_byte(Byte byte) {
    if (bytecode_len == allocated_len) {
        allocated_len += CHUNK_SIZE;
        bytecode = realloc(bytecode, allocated_len * sizeof *bytecode);
    }

    bytecode[bytecode_len++] = byte;
}

bool is_end(char letter) {
    return !letter || letter == ' ' || letter == '\n';
}

void check_end(char* input, char* letter) {
    if (!is_end(*letter)) {
        throw_syntax_error(input, letter, "expected space or newline.");
    }
}

int main(int num_args, const char** args) {
    char* input = io_load_input(num_args, args);

    // Fix line endings
    int offset = 0;
    for (char* letter = input; letter[-1]; letter++) {
        if (offset) {
            letter[offset] = *letter;
        }

        if (*letter == '\r') {
            if (letter[1] != '\n') {
                letter[offset] = '\n';
            } else {
                offset--;
            }
        }
    }

    bytecode = malloc(allocated_len * sizeof *bytecode);
    assert(bytecode);

    instructionset_init();

    strforeach(letter, input) {
        // We are in a comment
        if (*letter == '/' && letter[1] == '/') {
            letter++; // Skip to second slash
            do { letter++; } while (*letter && *letter != '\n');
        }

        // We are in a number, needs to be checked before instruction
        if (is_hex(*letter) && is_hex(letter[1]) && (!(letter + 2) || )) {
            bool* error = false;
            Byte num = parse_hex(*letter, error) << 4;
            letter++;

            num += parse_hex(*letter, error);
            letter++;

            assert(!error);

            add_byte(num);

            check_end(input, letter);
        }

        // We are in a string
        if (*letter == '"') {
            letter++;
            while (*letter != '"') {
                if (*letter == '\0') {
                    throw_syntax_error(input, letter, "unexpected EOF.");
                }

                if (*letter == '\n') {
                    throw_syntax_error(input, letter, "expected closing \".");
                }

                add_byte(*letter);

                letter++;
            }

            letter++;
            check_end(input, letter);
        }

        // We are in a constant
        if (is_const_letter(*letter)) {
            char* const_start = letter;
            size_t const_len = get_identifier_len(input, letter, &is_const_letter);
            letter += const_len;

            char const_buffer[const_len + 1];
            const_buffer[const_len] = '\0';
            memcpy(const_buffer, const_start, const_len);
            assert(!const_buffer[const_len]);

            bool is_invalid = false;
            const Constant* constant = instructionset_get_constant(const_buffer, &is_invalid);
            if (is_invalid) {
                size_t message_len = 20 + const_len;
                char message_buffer[message_len + 1];
                sprintf(message_buffer, "unknown constant '%s'.", const_buffer);
                assert(!message_buffer[message_len]);
                throw_syntax_error(input, letter - 1, message_buffer);
            }

            for (int i = 0; i < constant->size; i++) add_byte(constant->value[i]);

            check_end(input, letter);
        }

        // We are in an instruction
        if (is_instr_letter(*letter)) {
            char* instr_start = letter;
            size_t instr_len = get_identifier_len(input, letter, &is_instr_letter);
            letter += instr_len;

            char instr_buffer[instr_len + 1];
            instr_buffer[instr_len] = '\0';
            memcpy(instr_buffer, instr_start, instr_len);
            assert(!instr_buffer[instr_len]);

            bool is_invalid = false;
            Byte byte = instructionset_get_bytecode(instr_buffer, &is_invalid);
            if (is_invalid) {
                size_t message_len = 23 + instr_len;
                char message_buffer[message_len + 1];
                sprintf(message_buffer, "unknown instruction '%s'.", instr_buffer);
                assert(!message_buffer[message_len]);
                throw_syntax_error(input, letter - 1, message_buffer);
            }

            add_byte(byte);

            check_end(input, letter);
        }
    }

    io_dump_bytes(num_args, args, bytecode_len, (Byte*) bytecode);

    instructionset_deinit();
    free(bytecode);
    io_free_input(input);

    return 0;
}
