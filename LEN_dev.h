//
//  LEN_dev.h
//  lemon
//
//  Created by Azure on 2018/1/28.
//  Copyright © 2018年 Azure. All rights reserved.
//

#ifndef LEN_dev_h
#define LEN_dev_h


#include "LEN.h"

typedef enum {
    LEN_FALSE = 0,
    LEN_TRUE = 1
} LEN_Boolean;

typedef struct LEN_String_tag LEN_String;

/**
 * 定义指针信息
 */
typedef struct {
    char        *name;
} LEN_NativePointerInfo;

/**
 * 定义指针类型
 */
typedef struct {
    LEN_NativePointerInfo       *info;
    void                        *pointer;
} LEN_NativePointer;

/**
 * 定义所有的值类型
 */
typedef enum {
    LEN_BOOLEAN_VALUE = 1,
    LEN_INT_VALUE,
    LEN_DOUBLE_VALUE,
    LEN_STRING_VALUE,
    LEN_NATIVE_POINTER_VALUE,
    LEN_NULL_VALUE
} LEN_ValueType;

/**
 * 定义值
 */
typedef struct {
    LEN_ValueType       type;
    union {
        LEN_Boolean     boolean_value;
        int             int_value;
        double          double_value;
        LEN_String      *string_value;
        LEN_NativePointer       native_pointer;
    } u;
} LEN_Value;

typedef LEN_Value LEN_NativeFunctionProc(LEN_Interpreter *interpreter,
                                         int arg_count, LEN_Value *args);

void LEN_add_native_function(LEN_Interpreter *interpreter,
                             char *name, LEN_NativeFunctionProc *proc);
void LEN_add_global_variable(LEN_Interpreter *inter,
                             char *identifier, LEN_Value *value);

#endif /* LEN_dev_h */
