//
//  execute.c
//  lemon
//
//  Created by Azure on 2018/1/28.
//  Copyright © 2018年 Azure. All rights reserved.
//

#include <math.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "lemon.h"

static StatementResult
execute_statement(LEN_Interpreter *inter, LocalEnvironment *env,
                  Statement *statement);

/**
 * 执行表达式语句
 */
static StatementResult
execute_expression_statement(LEN_Interpreter *inter, LocalEnvironment *env,
                             Statement *statement)
{
    StatementResult result;
    LEN_Value v;
    
    result.type = NORMAL_STATEMENT_RESULT;
    // 计算表达式的值
    v = len_eval_expression(inter, env, statement->u.expression_s);
    if (v.type == LEN_STRING_VALUE) {
        // 引用计数释放
        len_release_string(v.u.string_value);
    }
    
    return result;
}

/**
 * 执行全局语句
 */
static StatementResult
execute_global_statement(LEN_Interpreter *inter, LocalEnvironment *env,
                         Statement *statement)
{
    IdentifierList *pos;
    StatementResult result;
    
    result.type = NORMAL_STATEMENT_RESULT;
    
    if (env == NULL) {
        len_runtime_error(statement->line_number,
                          GLOBAL_STATEMENT_IN_TOPLEVEL_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    for (pos = statement->u.global_s.identifier_list; pos; pos = pos->next) {
        GlobalVariableRef *ref_pos;
        GlobalVariableRef *new_ref;
        Variable *variable;
        for (ref_pos = env->global_variable; ref_pos;ref_pos = ref_pos->next) {
            if (!strcmp(ref_pos->variable->name, pos->name))
                goto NEXT_IDENTIFIER;
        }
        variable = len_search_global_variable(inter, pos->name);
        if (variable == NULL) {
            crb_runtime_error(statement->line_number,
                              GLOBAL_VARIABLE_NOT_FOUND_ERR,
                              STRING_MESSAGE_ARGUMENT, "name", pos->name,
                              MESSAGE_ARGUMENT_END);
        }
        new_ref = MEM_malloc(sizeof(GlobalVariableRef));
        new_ref->variable = variable;
        new_ref->next = env->global_variable;
        env->global_variable = new_ref;
    NEXT_IDENTIFIER:
        ;
    }
    
    return result;
}

/**
 * 执行elseif语句
 */
static StatementResult
execute_elsif(LEN_Interpreter *inter, LocalEnvironment *env,
              Elsif *elsif_list, LEN_Boolean *executed)
{
    StatementResult result;
    LEN_Value   cond;
    Elsif *pos;
    
    *executed = LEN_FALSE;
    result.type = NORMAL_STATEMENT_RESULT;
    for (pos = elsif_list; pos; pos = pos->next) {
        cond = len_eval_expression(inter, env, pos->condition);
        if (cond.type != LEN_BOOLEAN_VALUE) {
            crb_runtime_error(pos->condition->line_number,
                              NOT_BOOLEAN_TYPE_ERR, MESSAGE_ARGUMENT_END);
        }
        if (cond.u.boolean_value) {
            result = len_execute_statement_list(inter, env,
                                                pos->block->statement_list);
            *executed = LEN_TRUE;
            if (result.type != NORMAL_STATEMENT_RESULT)
                goto FUNC_END;
        }
    }
    
FUNC_END:
    return result;
}

/**
 * 执行if语句
 */
static StatementResult
execute_if_statement(LEN_Interpreter *inter, LocalEnvironment *env,
                     Statement *statement)
{
    StatementResult result;
    LEN_Value   cond;
    
    result.type = NORMAL_STATEMENT_RESULT;
    cond = len_eval_expression(inter, env, statement->u.if_s.condition);
    if (cond.type != LEN_BOOLEAN_VALUE) {
        len_runtime_error(statement->u.if_s.condition->line_number,
                          NOT_BOOLEAN_TYPE_ERR, MESSAGE_ARGUMENT_END);
    }
    DBG_assert(cond.type == LEN_BOOLEAN_VALUE, ("cond.type..%d", cond.type));
    
    // 条件为真，执行if语句块内容
    if (cond.u.boolean_value) {
        result = len_execute_statement_list(inter, env,
                                            statement->u.if_s.then_block
                                            ->statement_list);
    } else {
        // 执行else语句内容
        LEN_Boolean elsif_executed;
        result = execute_elsif(inter, env, statement->u.if_s.elsif_list,
                               &elsif_executed);
        if (result.type != NORMAL_STATEMENT_RESULT)
            goto FUNC_END;
        if (!elsif_executed && statement->u.if_s.else_block) {
            result = len_execute_statement_list(inter, env,
                                                statement->u.if_s.else_block
                                                ->statement_list);
        }
    }
    
FUNC_END:
    return result;
}

/**
 * 执行while语句
 */
static StatementResult
execute_while_statement(LEN_Interpreter *inter, LocalEnvironment *env,
                        Statement *statement)
{
    StatementResult result;
    LEN_Value   cond;
    
    result.type = NORMAL_STATEMENT_RESULT;
    for (;;) {
        // 获得条件语句的执行结果
        cond = len_eval_expression(inter, env, statement->u.while_s.condition);
        if (cond.type != LEN_BOOLEAN_VALUE) {
            len_runtime_error(statement->u.while_s.condition->line_number,
                              NOT_BOOLEAN_TYPE_ERR, MESSAGE_ARGUMENT_END);
        }
        DBG_assert(cond.type == LEN_BOOLEAN_VALUE,
                   ("cond.type..%d", cond.type));
        // 条件为真结束循环
        if (!cond.u.boolean_value)
            break;
        // 继续执行循环体内部的语句
        result = len_execute_statement_list(inter, env,
                                            statement->u.while_s.block
                                            ->statement_list);
        // 处理return, break, continue
        if (result.type == RETURN_STATEMENT_RESULT) {
            break;
        } else if (result.type == BREAK_STATEMENT_RESULT) {
            result.type = NORMAL_STATEMENT_RESULT;
            break;
        }
    }
    
    return result;
}

/**
 * 执行for语句
 */
static StatementResult
execute_for_statement(LEN_Interpreter *inter, LocalEnvironment *env,
                      Statement *statement)
{
    StatementResult result;
    LEN_Value   cond;
    
    result.type = NORMAL_STATEMENT_RESULT;
    
    if (statement->u.for_s.init) {
        len_eval_expression(inter, env, statement->u.for_s.init);
    }
    for (;;) {
        if (statement->u.for_s.condition) {
            cond = len_eval_expression(inter, env,
                                       statement->u.for_s.condition);
            if (cond.type != LEN_BOOLEAN_VALUE) {
                len_runtime_error(statement->u.for_s.condition->line_number,
                                  NOT_BOOLEAN_TYPE_ERR, MESSAGE_ARGUMENT_END);
            }
            DBG_assert(cond.type == LEN_BOOLEAN_VALUE,
                       ("cond.type..%d", cond.type));
            if (!cond.u.boolean_value)
                break;
        }
        result = len_execute_statement_list(inter, env,
                                            statement->u.for_s.block
                                            ->statement_list);
        if (result.type == RETURN_STATEMENT_RESULT) {
            break;
        } else if (result.type == BREAK_STATEMENT_RESULT) {
            result.type = NORMAL_STATEMENT_RESULT;
            break;
        }
        
        if (statement->u.for_s.post) {
            len_eval_expression(inter, env, statement->u.for_s.post);
        }
    }
    
    return result;
}

/**
 * 执行return语句
 */
static StatementResult
execute_return_statement(LEN_Interpreter *inter, LocalEnvironment *env,
                         Statement *statement)
{
    StatementResult result;
    
    result.type = RETURN_STATEMENT_RESULT;
    // 返回结果赋值
    if (statement->u.return_s.return_value) {
        result.u.return_value = len_eval_expression(inter, env, statement->u.return_s.return_value);
    } else {
        result.u.return_value.type = LEN_NULL_VALUE;
    }
    
    return result;
}

/**
 * 执行break语句
 */
static StatementResult
execute_break_statement(LEN_Interpreter *inter, LocalEnvironment *env,
                        Statement *statement)
{
    StatementResult result;
    
    result.type = BREAK_STATEMENT_RESULT;
    
    return result;
}

/**
 * 执行continue语句
 */
static StatementResult
execute_continue_statement(LEN_Interpreter *inter, LocalEnvironment *env,
                           Statement *statement)
{
    StatementResult result;
    
    result.type = CONTINUE_STATEMENT_RESULT;
    
    return result;
}

/**
 * 执行单条语句
 */
static StatementResult
execute_statement(LEN_Interpreter *inter, LocalEnvironment *env, Statement *statement){
    StatementResult result;
    result.type = NORMAL_STATEMENT_RESULT;
    
    switch (statement->type) {
        case EXPRESSION_STATEMENT:
            
            break;
            
        default:
            DBG_panic(("bad case ...%s", statement->type));
    }
    return result;
}

/**
 * 执行语句链
 */
StatementResult
len_execute_statement_list(LEN_Interpreter *inter, LocalEnvironment *env, StatementList *list){
    StatementList *pos;
    StatementResult result;
    
    result.type = NORMAL_STATEMENT_RESULT;
    // 按照链表顺序执行语句
    for (pos = list; pos; pos = pos->next) {
        result = execute_statement(inter, env, pos->statement);
        if (result.type != NORMAL_STATEMENT_RESULT) {
            goto FUNC_END;
        }
    }
    
FUNC_END:
    return result;
}
