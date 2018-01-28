//
//  native.c
//  lemon
//
//  Created by Azure on 2018/1/28.
//  Copyright © 2018年 Azure. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "LEN_dev.h"
#include "lemon.h"

#define NATIVE_LIB_NAME "lemon.lang.file"

static LEN_NativePointerInfo st_native_lib_info = {
    NATIVE_LIB_NAME
};

LEN_Value len_nv_print_proc(LEN_Interpreter *interpreter,
                            int arg_count, LEN_Value *args)
{
    LEN_Value value;
    
    value.type = LEN_NULL_VALUE;
    
    if (arg_count < 1) {
        len_runtime_error(0, ARGUMENT_TOO_FEW_ERR,
                          MESSAGE_ARGUMENT_END);
    } else if (arg_count > 1) {
        len_runtime_error(0, ARGUMENT_TOO_MANY_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    switch (args[0].type) {
        case LEN_BOOLEAN_VALUE:
            if (args[0].u.boolean_value) {
                printf("true");
            } else {
                printf("false");
            }
            break;
        case LEN_INT_VALUE:
            printf("%d", args[0].u.int_value);
            break;
        case LEN_DOUBLE_VALUE:
            printf("%f", args[0].u.double_value);
            break;
        case LEN_STRING_VALUE:
            printf("%s", args[0].u.string_value->string);
            break;
        case LEN_NATIVE_POINTER_VALUE:
            printf("(%s:%p)",
                   args[0].u.native_pointer.info->name,
                   args[0].u.native_pointer.pointer);
            break;
        case LEN_NULL_VALUE:
            printf("null");
            break;
    }
    
    return value;
}

LEN_Value len_nv_fopen_proc(LEN_Interpreter *interpreter,
                            int arg_count, LEN_Value *args)
{
    LEN_Value value;
    FILE *fp;
    
    if (arg_count < 2) {
        len_runtime_error(0, ARGUMENT_TOO_FEW_ERR,
                          MESSAGE_ARGUMENT_END);
    } else if (arg_count > 2) {
        len_runtime_error(0, ARGUMENT_TOO_MANY_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    if (args[0].type != LEN_STRING_VALUE
        || args[1].type != LEN_STRING_VALUE) {
        len_runtime_error(0, FOPEN_ARGUMENT_TYPE_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    
    fp = fopen(args[0].u.string_value->string,
               args[1].u.string_value->string);
    if (fp == NULL) {
        value.type = LEN_NULL_VALUE;
    } else {
        value.type = LEN_NATIVE_POINTER_VALUE;
        value.u.native_pointer.info = &st_native_lib_info;
        value.u.native_pointer.pointer = fp;
    }
    
    return value;
}

static LEN_Boolean
check_native_pointer(LEN_Value *value)
{
    return value->u.native_pointer.info == &st_native_lib_info;
}

LEN_Value len_nv_fclose_proc(LEN_Interpreter *interpreter,
                             int arg_count, LEN_Value *args)
{
    LEN_Value value;
    FILE *fp;
    
    value.type = LEN_NULL_VALUE;
    if (arg_count < 1) {
        len_runtime_error(0, ARGUMENT_TOO_FEW_ERR,
                          MESSAGE_ARGUMENT_END);
    } else if (arg_count > 1) {
        len_runtime_error(0, ARGUMENT_TOO_MANY_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    if (args[0].type != LEN_NATIVE_POINTER_VALUE
        || !check_native_pointer(&args[0])) {
        len_runtime_error(0, FCLOSE_ARGUMENT_TYPE_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    fp = args[0].u.native_pointer.pointer;
    fclose(fp);
    
    return value;
}

LEN_Value len_nv_fgets_proc(LEN_Interpreter *interpreter,
                            int arg_count, LEN_Value *args)
{
    LEN_Value value;
    FILE *fp;
    char buf[LINE_BUF_SIZE];
    char *ret_buf = NULL;
    int ret_len = 0;
    
    if (arg_count < 1) {
        len_runtime_error(0, ARGUMENT_TOO_FEW_ERR,
                          MESSAGE_ARGUMENT_END);
    } else if (arg_count > 1) {
        len_runtime_error(0, ARGUMENT_TOO_MANY_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    if (args[0].type != LEN_NATIVE_POINTER_VALUE
        || !check_native_pointer(&args[0])) {
        len_runtime_error(0, FGETS_ARGUMENT_TYPE_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    fp = args[0].u.native_pointer.pointer;
    
    while (fgets(buf, LINE_BUF_SIZE, fp)) {
        int new_len;
        new_len = ret_len + strlen(buf);
        ret_buf = MEM_realloc(ret_buf, new_len + 1);
        if (ret_len == 0) {
            strcpy(ret_buf, buf);
        } else {
            strcat(ret_buf, buf);
        }
        ret_len = new_len;
        if (ret_buf[ret_len-1] == '\n')
            break;
    }
    if (ret_len > 0) {
        value.type = LEN_STRING_VALUE;
        value.u.string_value = len_create_lemon_string(interpreter, ret_buf);
    } else {
        value.type = LEN_NULL_VALUE;
    }
    
    return value;
}

LEN_Value len_nv_fputs_proc(LEN_Interpreter *interpreter,
                            int arg_count, LEN_Value *args)
{
    LEN_Value value;
    FILE *fp;
    
    value.type = LEN_NULL_VALUE;
    if (arg_count < 2) {
        len_runtime_error(0, ARGUMENT_TOO_FEW_ERR,
                          MESSAGE_ARGUMENT_END);
    } else if (arg_count > 2) {
        len_runtime_error(0, ARGUMENT_TOO_MANY_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    if (args[0].type != LEN_STRING_VALUE
        || (args[1].type != LEN_NATIVE_POINTER_VALUE
            || !check_native_pointer(&args[1]))) {
            len_runtime_error(0, FPUTS_ARGUMENT_TYPE_ERR,
                              MESSAGE_ARGUMENT_END);
        }
    fp = args[1].u.native_pointer.pointer;
    
    fputs(args[0].u.string_value->string, fp);
    
    return value;
}

void
len_add_std_fp(LEN_Interpreter *inter)
{
    LEN_Value fp_value;
    
    fp_value.type = LEN_NATIVE_POINTER_VALUE;
    fp_value.u.native_pointer.info = &st_native_lib_info;
    
    fp_value.u.native_pointer.pointer = stdin;
    LEN_add_global_variable(inter, "STDIN", &fp_value);
    
    fp_value.u.native_pointer.pointer = stdout;
    LEN_add_global_variable(inter, "STDOUT", &fp_value);
    
    fp_value.u.native_pointer.pointer = stderr;
    LEN_add_global_variable(inter, "STDERR", &fp_value);
}

