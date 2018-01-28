//
//  util.c
//  lemon
//
//  Created by Azure on 2018/1/28.
//  Copyright © 2018年 Azure. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "lemon.h"

/**保存了当前的解释器*/
static LEN_Interpreter *st_current_interpreter;

LEN_Interpreter *
len_get_current_interpreter(void)
{
    return st_current_interpreter;
}

void
len_set_current_interpreter(LEN_Interpreter *inter)
{
    st_current_interpreter = inter;
}

void *
len_execute_malloc(LEN_Interpreter *inter, size_t size)
{
    void *p;
    
    p = MEM_storage_malloc(inter->execute_storage, size);
    
    return p;
}

void *
len_malloc(size_t size)
{
    /* 原书实现
    void *p;
    LEN_Interpreter *inter;
    
    inter = len_get_current_interpreter();
    p = MEM_storage_malloc(inter->interpreter_storage, size);
    
    return p;
     */
    
    // 修改过后的实现
    return len_execute_malloc(len_get_current_interpreter(), size);
}

/**
 * 查找指定的函数
 */
FunctionDefinition *
len_search_function(char *name)
{
    FunctionDefinition *pos;
    LEN_Interpreter *inter;
    
    inter = len_get_current_interpreter();
    for (pos = inter->function_list; pos; pos = pos->next) {
        if (!strcmp(pos->name, name))
            break;
    }
    return pos;
}

/**
 * 搜索局部变量
 */
Variable *
len_search_local_variable(LocalEnvironment *env, char *identifier)
{
    Variable *pos;
    
    if (env == NULL)
        return NULL;
    for (pos = env->variable; pos; pos = pos->next) {
        if (!strcmp(pos->name, identifier))
            break;
    }
    if (pos == NULL) {
        return NULL;
    } else {
        return pos;
    }
}

/**
 * 查找全局变量
 */
Variable *
len_search_global_variable(LEN_Interpreter *inter, char *identifier)
{
    Variable *pos;
    
    for (pos = inter->variable; pos; pos = pos->next) {
        if (!strcmp(pos->name, identifier))
            return pos;
    }
    
    return NULL;
}

void
len_add_local_variable(LocalEnvironment *env,
                       char *identifier, LEN_Value *value)
{
    Variable    *new_variable;
    // 申请内存
    new_variable = MEM_malloc(sizeof(Variable));
    new_variable->name = identifier;
    new_variable->value = *value;
    // 在链表头部添加
    new_variable->next = env->variable;
    env->variable = new_variable;
}

/**
 * 增加全局变量, 与len_add_local_variable()实现基本类似
 */
void
LEN_add_global_variable(LEN_Interpreter *inter, char *identifier,
                        LEN_Value *value)
{
    Variable    *new_variable;
    
    new_variable = len_execute_malloc(inter, sizeof(Variable));
    new_variable->name = len_execute_malloc(inter, strlen(identifier) + 1);
    strcpy(new_variable->name, identifier);
    new_variable->next = inter->variable;
    inter->variable = new_variable;
    new_variable->value = *value;
}

/**
 * 表达式转为字符形式
 */
char *
len_get_operator_string(ExpressionType type)
{
    char        *str;
    
    switch (type) {
        case BOOLEAN_EXPRESSION:    /* FALLTHRU */
        case INT_EXPRESSION:        /* FALLTHRU */
        case DOUBLE_EXPRESSION:     /* FALLTHRU */
        case STRING_EXPRESSION:     /* FALLTHRU */
        case IDENTIFIER_EXPRESSION:
            DBG_panic(("bad expression type..%d\n", type));
            break;
        case ASSIGN_EXPRESSION:
            str = "=";
            break;
        case ADD_EXPRESSION:
            str = "+";
            break;
        case SUB_EXPRESSION:
            str = "-";
            break;
        case MUL_EXPRESSION:
            str = "*";
            break;
        case DIV_EXPRESSION:
            str = "/";
            break;
        case MOD_EXPRESSION:
            str = "%";
            break;
        case LOGICAL_AND_EXPRESSION:
            str = "&&";
            break;
        case LOGICAL_OR_EXPRESSION:
            str = "||";
            break;
        case EQ_EXPRESSION:
            str = "==";
            break;
        case NE_EXPRESSION:
            str = "!=";
            break;
        case GT_EXPRESSION:
            str = "<";
            break;
        case GE_EXPRESSION:
            str = "<=";
            break;
        case LT_EXPRESSION:
            str = ">";
            break;
        case LE_EXPRESSION:
            str = ">=";
            break;
        case MINUS_EXPRESSION:
            str = "-";
            break;
        case FUNCTION_CALL_EXPRESSION:  /* FALLTHRU */
        case NULL_EXPRESSION:  /* FALLTHRU */
        case EXPRESSION_TYPE_COUNT_PLUS_1:
        default:
            DBG_panic(("bad expression type..%d\n", type));
    }
    
    return str;
}
