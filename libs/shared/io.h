//
//  io.h
//  libs/shared
//
//  Created by Zander Otavka on 8/6/16
//  Copyright (c) 2016 Zander Otavka All Rights Reserved.
//

#ifndef zee_fileio_h
#define zee_fileio_h

#include "util.h"

char* io_load_input(int num_args, const char** args);
void io_free_input(char* input);

void io_dump_bytecode_string(size_t length, const Byte* bytecode);
void io_dump_bytes(int num_args, const char** args, size_t length, const Byte* bytecode);

#endif // zee_fileio_h
