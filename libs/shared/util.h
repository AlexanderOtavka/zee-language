//
//  util.h
//  libs/shared
//
//  Created by Zander Otavka on 8/6/16
//  Copyright (c) 2016 Zander Otavka All Rights Reserved.
//

#ifndef zee_util_h
#define zee_util_h

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

typedef uint8_t Byte;
typedef void* Ptr;
typedef size_t (*Instruction)(const Byte* args);

static const size_t BYTE_BIT_SIZE = 8;
static const Byte BYTE_MAX_VAL = 255;

static const size_t CONSTANT_MAX_SIZE = 8;
typedef struct {
    size_t size;
    Byte value[CONSTANT_MAX_SIZE];
} Constant;

#define foreach(i, a, l) \
    for (typeof(a) i = a, __end = a + l; i < __end; i++)

#define strforeach(i, s) \
    for (typeof(s) i = s; *i; i++)


#endif // zee_util_h
