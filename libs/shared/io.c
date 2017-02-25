//
//  io.c
//  libs/shared
//
//  Created by Zander Otavka on 8/6/16
//  Copyright (c) 2016 Zander Otavka All Rights Reserved.
//

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "util.h"
#include "io.h"

const char* _get_filename(int num_args, const char** args) {
    if (num_args > 1) {
        return args[num_args - 1];
    } else {
        return NULL;
    }
}

char* io_load_input(int num_args, const char** args) {
    const char* filename = _get_filename(num_args, args);
    char* data;

    if (filename) {
        FILE* file = fopen(filename, "r");
        if (!file) {
            perror(filename);
            exit(EXIT_FAILURE);
        }   

        fseek(file, 0L, SEEK_END);
        size_t file_size = ftell(file);
        rewind(file);

        data = malloc(file_size + 1); 
        assert(data);
        data[file_size] = '\0';
            
        size_t num_elements = 1;
        size_t num_elements_read = fread(data, file_size, num_elements, file);
        assert(num_elements_read == num_elements);

        fclose(file);
    } else {
        static const size_t CHUNK_SIZE = 100;
        size_t num_bytes_read, data_len = 0;
        data = malloc(CHUNK_SIZE + 1);

        do {
            data = realloc(data, data_len + CHUNK_SIZE);
            num_bytes_read = read(STDIN_FILENO, data + data_len, CHUNK_SIZE);
            data_len += num_bytes_read;
        } while(num_bytes_read);

        data = realloc(data, data_len + 1);
        data[data_len] = '\0';
    }

    return data;
}

void io_free_input(char* input) {
    free(input);
}

void io_dump_bytecode_string(size_t length, const Byte* bytecode) {
    if (!length) {
        printf("\n");
        return;
    }

    foreach(byte, bytecode, length - 1) {
        printf("%02x ", *byte);
    }

    printf("%02x\n", bytecode[length - 1]);
}

void io_dump_bytes(int num_args, const char** args, size_t length, const Byte* bytecode) {
    const char* filename = _get_filename(num_args, args);
    if (filename) {
        // write to filename.zc
        const char* last_dot = NULL;
        strforeach(character, filename) {
            if (*character == '.') {
                last_dot = character;
            }
        }

        size_t filename_len = (last_dot && strcmp(last_dot, ".zac") == 0) ? (last_dot - filename) : strlen(filename);
        const size_t suffix_len = 3;
        char new_filename_buffer[filename_len + suffix_len + 1];
        memcpy(new_filename_buffer, filename, filename_len);
        strcpy(new_filename_buffer + filename_len, ".zc");
        assert(!new_filename_buffer[filename_len + suffix_len]);

        FILE* file = fopen(new_filename_buffer, "wb");
        if (!file) {
            perror(filename);
            exit(EXIT_FAILURE);
        }   

        fwrite(bytecode, sizeof(Byte), length, file);
    } else {
        // write to stdout
        int written = write(STDOUT_FILENO, bytecode, length);
        if (written == -1) {
            perror("stdout");
            exit(EXIT_FAILURE);
        }

        assert(written == length);
    }
}
