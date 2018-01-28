//
//  interface.c
//  lemon
//
//  Created by Azure on 2018/1/28.
//  Copyright © 2018年 Azure. All rights reserved.
//

#include "MEM.h"
#include "DBG.h"
#define GLOBAL_VARIABLE_DEFINE
#include "lemon.h"

static void
add_native_functions(LEN_Interpreter *inter)
{
    LEN_add_native_function(inter, "print", len_nv_print_proc);
    LEN_add_native_function(inter, "fopen", len_nv_fopen_proc);
    LEN_add_native_function(inter, "fclose", len_nv_fclose_proc);
    LEN_add_native_function(inter, "fgets", len_nv_fgets_proc);
    LEN_add_native_function(inter, "fputs", len_nv_fputs_proc);
}

LEN_Interpreter *
LEN_create_interpreter(void)
{
    MEM_Storage storage;
    LEN_Interpreter *interpreter;
    
    storage = MEM_open_storage(0);
    interpreter = MEM_storage_malloc(storage,
                                     sizeof(struct LEN_Interpreter_tag));
    interpreter->interpreter_storage = storage;
    interpreter->execute_storage = NULL;
    interpreter->variable = NULL;
    interpreter->function_list = NULL;
    interpreter->statement_list = NULL;
    interpreter->current_line_number = 1;
    
    len_set_current_interpreter(interpreter);
    add_native_functions(interpreter);
    
    return interpreter;
}

void
LEN_compile(LEN_Interpreter *interpreter, FILE *fp)
{
    extern int yyparse(void);
    extern FILE *yyin;
    
    len_set_current_interpreter(interpreter);
    
    yyin = fp;
    if (yyparse()) {
        /* BUGBUG */
        fprintf(stderr, "Error ! Error ! Error !\n");
        exit(1);
    }
    len_reset_string_literal_buffer();
}

/**
 * 执行解释器
 */
void
LEN_interpret(LEN_Interpreter *interpreter){
    // 分配运行时需要的内存
    interpreter->execute_storage = MEM_open_storage(0);
    // 注册stdin, stdout, stderr
    len_add_std_fp(interpreter);
    // 执行语句链，statement_list是一个链表,所以可以按照顺序依次执行
    len_execute_statement_list(interpreter, NULL, interpreter->statement_list);
}

static void
release_global_strings(LEN_Interpreter *interpreter) {
    while (interpreter->variable) {
        Variable *temp = interpreter->variable;
        interpreter->variable = temp->next;
        if (temp->value.type == LEN_STRING_VALUE) {
            len_release_string(temp->value.u.string_value);
        }
    }
}

void
LEN_dispose_interpreter(LEN_Interpreter *interpreter)
{
    release_global_strings(interpreter);
    
    if (interpreter->execute_storage) {
        MEM_dispose_storage(interpreter->execute_storage);
    }
    
    MEM_dispose_storage(interpreter->interpreter_storage);
}

void
LEN_add_native_function(LEN_Interpreter *interpreter,
                        char *name, LEN_NativeFunctionProc *proc)
{
    FunctionDefinition *fd;
    
    fd = len_malloc(sizeof(FunctionDefinition));
    fd->name = name;
    fd->type = NATIVE_FUNCTION_DEFINITION;
    fd->u.native_f.proc = proc;
    fd->next = interpreter->function_list;
    
    interpreter->function_list = fd;
}

