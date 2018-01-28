//
//  eval.c
//  lemon
//  这个文件主要用来对各种表达式求值
//  Created by Azure on 2018/1/28.
//  Copyright © 2018年 Azure. All rights reserved.
//

#include <math.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "lemon.h"

static LEN_Value eval_expression(LEN_Interpreter *inter, LocalEnvironment *env,
                                 Expression *expr);

static LEN_Value
eval_boolean_expression(LEN_Boolean boolean_value)
{
    LEN_Value v;
    v.type = LEN_BOOLEAN_VALUE;
    v.u.boolean_value = boolean_value;
    return v;
}

static LEN_Value
eval_int_expression(int int_value)
{
    LEN_Value   v;
    v.type = LEN_INT_VALUE;
    v.u.int_value = int_value;
    return v;
}

static LEN_Value
eval_double_expression(double double_value)
{
    LEN_Value v;
    v.type = LEN_DOUBLE_VALUE;
    v.u.double_value = double_value;
    return v;
}

/**
 * 返回LEN_String， 并且将ref_count置为1
 */
static LEN_Value
eval_string_expression(LEN_Interpreter *inter, char *string_value)
{
    LEN_Value v;
    v.type = LEN_STRING_VALUE;
    v.u.string_value = len_literal_to_len_string(inter, string_value);
    return v;
}

static LEN_Value
eval_null_expression(void)
{
    LEN_Value v;
    v.type = LEN_NULL_VALUE;
    return v;
}

/**
 * 使用了引用类型是LEN_STRING的引用计数+1
 */
static void
refer_if_string(LEN_Value *v)
{
    if (v->type == LEN_STRING_VALUE) {
        len_refer_string(v->u.string_value);
    }
}

/**
 * ref_count-=1 加入引用计数为0，回收内存
 */
static void
release_if_string(LEN_Value *v)
{
    if (v->type == LEN_STRING_VALUE) {
        len_release_string(v->u.string_value);
    }
}

LEN_String *
chain_string(LEN_Interpreter *inter, LEN_String *left, LEN_String *right)
{
    int len;
    char *str;
    LEN_String *ret;
    
    len = strlen(left->string) + strlen(right->string);
    str = MEM_malloc(len + 1);
    strcpy(str, left->string);
    strcat(str, right->string);
    ret = len_create_lemon_string(inter, str);
    len_release_string(left);
    len_release_string(right);
    return ret;
}

/**
 * 查找全局变量
 */
static Variable *
search_global_variable_from_env(LEN_Interpreter *inter,
                                LocalEnvironment *env, char *name)
{
    GlobalVariableRef *pos;
    
    if (env == NULL) {
        return len_search_global_variable(inter, name);
    }
    
    for (pos = env->global_variable; pos; pos = pos->next) {
        if (!strcmp(pos->variable->name, name)) {
            return pos->variable;
        }
    }
    return NULL;
}

/**
 * int类型求值
 */
static void
eval_binary_int(LEN_Interpreter *inter, ExpressionType operator,
                int left, int right,
                LEN_Value *result, int line_number)
{
    // 数值操作
    if (dkc_is_math_operator(operator)) {
        result->type = LEN_INT_VALUE;
    }
    // 比较操作
    else if (dkc_is_compare_operator(operator)) {
        result->type = LEN_BOOLEAN_VALUE;
    } else {
        DBG_panic(("operator..%d\n", operator));
    }
    
    switch (operator) {
        case BOOLEAN_EXPRESSION:    /* FALLTHRU */
        case INT_EXPRESSION:        /* FALLTHRU */
        case DOUBLE_EXPRESSION:     /* FALLTHRU */
        case STRING_EXPRESSION:     /* FALLTHRU */
        case IDENTIFIER_EXPRESSION: /* FALLTHRU */
        case ASSIGN_EXPRESSION:
            DBG_panic(("bad case...%d", operator));
            break;
        case ADD_EXPRESSION:
            result->u.int_value = left + right;
            break;
        case SUB_EXPRESSION:
            result->u.int_value = left - right;
            break;
        case MUL_EXPRESSION:
            result->u.int_value = left * right;
            break;
        case DIV_EXPRESSION:
            result->u.int_value = left / right;
            break;
        case MOD_EXPRESSION:
            result->u.int_value = left % right;
            break;
        case LOGICAL_AND_EXPRESSION:        /* FALLTHRU */
        case LOGICAL_OR_EXPRESSION:
            DBG_panic(("bad case...%d", operator));
            break;
        case EQ_EXPRESSION:
            result->u.boolean_value = left == right;
            break;
        case NE_EXPRESSION:
            result->u.boolean_value = left != right;
            break;
        case GT_EXPRESSION:
            result->u.boolean_value = left > right;
            break;
        case GE_EXPRESSION:
            result->u.boolean_value = left >= right;
            break;
        case LT_EXPRESSION:
            result->u.boolean_value = left < right;
            break;
        case LE_EXPRESSION:
            result->u.boolean_value = left <= right;
            break;
        case MINUS_EXPRESSION:              /* FALLTHRU */
        case FUNCTION_CALL_EXPRESSION:      /* FALLTHRU */
        case NULL_EXPRESSION:               /* FALLTHRU */
        case EXPRESSION_TYPE_COUNT_PLUS_1:  /* FALLTHRU */
        default:
            DBG_panic(("bad case...%d", operator));
    }
}

static void
eval_binary_double(LEN_Interpreter *inter, ExpressionType operator,
                   double left, double right,
                   LEN_Value *result, int line_number)
{
    if (dkc_is_math_operator(operator)) {
        result->type = LEN_DOUBLE_VALUE;
    } else if (dkc_is_compare_operator(operator)) {
        result->type = LEN_BOOLEAN_VALUE;
    } else {
        DBG_panic(("operator..%d\n", operator));
    }
    
    switch (operator) {
        case BOOLEAN_EXPRESSION:    /* FALLTHRU */
        case INT_EXPRESSION:        /* FALLTHRU */
        case DOUBLE_EXPRESSION:     /* FALLTHRU */
        case STRING_EXPRESSION:     /* FALLTHRU */
        case IDENTIFIER_EXPRESSION: /* FALLTHRU */
        case ASSIGN_EXPRESSION:
            DBG_panic(("bad case...%d", operator));
            break;
        case ADD_EXPRESSION:
            result->u.double_value = left + right;
            break;
        case SUB_EXPRESSION:
            result->u.double_value = left - right;
            break;
        case MUL_EXPRESSION:
            result->u.double_value = left * right;
            break;
        case DIV_EXPRESSION:
            result->u.double_value = left / right;
            break;
        case MOD_EXPRESSION:
            result->u.double_value = fmod(left, right);
            break;
        case LOGICAL_AND_EXPRESSION:        /* FALLTHRU */
        case LOGICAL_OR_EXPRESSION:
            DBG_panic(("bad case...%d", operator));
            break;
        case EQ_EXPRESSION:
            result->u.int_value = left == right;
            break;
        case NE_EXPRESSION:
            result->u.int_value = left != right;
            break;
        case GT_EXPRESSION:
            result->u.int_value = left > right;
            break;
        case GE_EXPRESSION:
            result->u.int_value = left >= right;
            break;
        case LT_EXPRESSION:
            result->u.int_value = left < right;
            break;
        case LE_EXPRESSION:
            result->u.int_value = left <= right;
            break;
        case MINUS_EXPRESSION:              /* FALLTHRU */
        case FUNCTION_CALL_EXPRESSION:      /* FALLTHRU */
        case NULL_EXPRESSION:               /* FALLTHRU */
        case EXPRESSION_TYPE_COUNT_PLUS_1:  /* FALLTHRU */
        default:
            DBG_panic(("bad default...%d", operator));
    }
}

static LEN_Boolean
eval_compare_string(ExpressionType operator,
                    LEN_Value *left, LEN_Value *right, int line_number)
{
    LEN_Boolean result;
    int cmp;
    
    cmp = strcmp(left->u.string_value->string, right->u.string_value->string);
    
    if (operator == EQ_EXPRESSION) {
        result = (cmp == 0);
    } else if (operator == NE_EXPRESSION) {
        result = (cmp != 0);
    } else if (operator == GT_EXPRESSION) {
        result = (cmp > 0);
    } else if (operator == GE_EXPRESSION) {
        result = (cmp >= 0);
    } else if (operator == LT_EXPRESSION) {
        result = (cmp < 0);
    } else if (operator == LE_EXPRESSION) {
        result = (cmp <= 0);
    } else {
        char *op_str = len_get_operator_string(operator);
        len_runtime_error(line_number, BAD_OPERATOR_FOR_STRING_ERR,
                          STRING_MESSAGE_ARGUMENT, "operator", op_str,
                          MESSAGE_ARGUMENT_END);
    }
    len_release_string(left->u.string_value);
    len_release_string(right->u.string_value);
    return result;
}

static LEN_Boolean
eval_binary_null(LEN_Interpreter *inter, ExpressionType operator,
                 LEN_Value *left, LEN_Value *right, int line_number)
{
    LEN_Boolean result;
    
    if (operator == EQ_EXPRESSION) {
        result = left->type == LEN_NULL_VALUE && right->type == LEN_NULL_VALUE;
    } else if (operator == NE_EXPRESSION) {
        result =  !(left->type == LEN_NULL_VALUE && right->type == LEN_NULL_VALUE);
    } else {
        char *op_str = len_get_operator_string(operator);
        len_runtime_error(line_number, NOT_NULL_OPERATOR_ERR,
                          STRING_MESSAGE_ARGUMENT, "operator", op_str,
                          MESSAGE_ARGUMENT_END);
    }
    release_if_string(left);
    release_if_string(right);
    return result;
}

static LEN_Boolean
eval_binary_boolean(LEN_Interpreter *inter, ExpressionType operator,
                    LEN_Boolean left, LEN_Boolean right, int line_number)
{
    LEN_Boolean result;
    
    if (operator == EQ_EXPRESSION) {
        result = left == right;
    } else if (operator == NE_EXPRESSION) {
        result = left != right;
    } else {
        char *op_str = len_get_operator_string(operator);
        len_runtime_error(line_number, NOT_BOOLEAN_OPERATOR_ERR,
                          STRING_MESSAGE_ARGUMENT, "operator", op_str,
                          MESSAGE_ARGUMENT_END);
    }
    
    return result;
}

/**
 * 二元表达式求值
 */
LEN_Value
len_eval_binary_expression(LEN_Interpreter *inter, LocalEnvironment *env,
                           ExpressionType operator,
                           Expression *left, Expression *right)
{
    LEN_Value left_val;
    LEN_Value right_val;
    LEN_Value result;
    
    left_val = eval_expression(inter, env, left);
    right_val = eval_expression(inter, env, right);
    
    if (left_val.type == LEN_INT_VALUE
        && right_val.type == LEN_INT_VALUE) {
        eval_binary_int(inter, operator,
                        left_val.u.int_value, right_val.u.int_value,
                        &result, left->line_number);
    } else if (left_val.type == LEN_DOUBLE_VALUE
               && right_val.type == LEN_DOUBLE_VALUE) {
        eval_binary_double(inter, operator,
                           left_val.u.double_value, right_val.u.double_value,
                           &result, left->line_number);
    } else if (left_val.type == LEN_INT_VALUE
               && right_val.type == LEN_DOUBLE_VALUE) {
        left_val.u.double_value = left_val.u.int_value;
        eval_binary_double(inter, operator,
                           left_val.u.double_value, right_val.u.double_value,
                           &result, left->line_number);
    } else if (left_val.type == LEN_DOUBLE_VALUE
               && right_val.type == LEN_INT_VALUE) {
        right_val.u.double_value = right_val.u.int_value;
        eval_binary_double(inter, operator,
                           left_val.u.double_value, right_val.u.double_value,
                           &result, left->line_number);
    } else if (left_val.type == LEN_BOOLEAN_VALUE
               && right_val.type == LEN_BOOLEAN_VALUE) {
        result.type = LEN_BOOLEAN_VALUE;
        result.u.boolean_value
        = eval_binary_boolean(inter, operator,
                              left_val.u.boolean_value,
                              right_val.u.boolean_value,
                              left->line_number);
    } else if (left_val.type == LEN_STRING_VALUE
               && operator == ADD_EXPRESSION) {
        char    buf[LINE_BUF_SIZE];
        LEN_String *right_str;
        
        if (right_val.type == LEN_INT_VALUE) {
            sprintf(buf, "%d", right_val.u.int_value);
            right_str = len_create_lemon_string(inter, MEM_strdup(buf));
        } else if (right_val.type == LEN_DOUBLE_VALUE) {
            sprintf(buf, "%f", right_val.u.double_value);
            right_str = len_create_lemon_string(inter, MEM_strdup(buf));
        } else if (right_val.type == LEN_BOOLEAN_VALUE) {
            if (right_val.u.boolean_value) {
                right_str = len_create_lemon_string(inter, MEM_strdup("true"));
            } else {
                right_str = len_create_lemon_string(inter, MEM_strdup("false"));
            }
        } else if (right_val.type == LEN_STRING_VALUE) {
            right_str = right_val.u.string_value;
        } else if (right_val.type == LEN_NATIVE_POINTER_VALUE) {
            sprintf(buf, "(%s:%p)",
                    right_val.u.native_pointer.info->name,
                    right_val.u.native_pointer.pointer);
            right_str = len_create_lemon_string(inter, MEM_strdup(buf));
        } else if (right_val.type == LEN_NULL_VALUE) {
            right_str = len_create_lemon_string(inter, MEM_strdup("null"));
        }
        result.type = LEN_STRING_VALUE;
        result.u.string_value = chain_string(inter,
                                             left_val.u.string_value,
                                             right_str);
    } else if (left_val.type == LEN_STRING_VALUE
               && right_val.type == LEN_STRING_VALUE) {
        result.type = LEN_BOOLEAN_VALUE;
        result.u.boolean_value
        = eval_compare_string(operator, &left_val, &right_val,
                              left->line_number);
    } else if (left_val.type == LEN_NULL_VALUE
               || right_val.type == LEN_NULL_VALUE) {
        result.type = LEN_BOOLEAN_VALUE;
        result.u.boolean_value
        = eval_binary_null(inter, operator, &left_val, &right_val,
                           left->line_number);
    } else {
        char *op_str = len_get_operator_string(operator);
        len_runtime_error(left->line_number, BAD_OPERAND_TYPE_ERR,
                          STRING_MESSAGE_ARGUMENT, "operator", op_str,
                          MESSAGE_ARGUMENT_END);
    }
    
    return result;
}

/**
 * 解析变量的值
 */
static LEN_Value
eval_identifier_expression(LEN_Interpreter *inter,
                           LocalEnvironment *env, Expression *expr)
{
    LEN_Value   v;
    Variable    *vp;
    
    // 先查询局部变量
    vp = len_search_local_variable(env, expr->u.identifier);
    if (vp != NULL) {
        v = vp->value;
    }
    // 查询全局变量
    else {
        vp = search_global_variable_from_env(inter, env, expr->u.identifier);
        if (vp != NULL) {
            v = vp->value;
        } else {
            len_runtime_error(expr->line_number, VARIABLE_NOT_FOUND_ERR,
                              STRING_MESSAGE_ARGUMENT,
                              "name", expr->u.identifier,
                              MESSAGE_ARGUMENT_END);
        }
    }
    // string类型增加引用计数
    refer_if_string(&v);
    
    return v;
}

/**
 * 处理赋值语句
 */
static LEN_Value
eval_assign_expression(LEN_Interpreter *inter, LocalEnvironment *env,
                       char *identifier, Expression *expression)
{
    LEN_Value   v;
    Variable    *left;
    
    v = eval_expression(inter, env, expression);
    
    left = len_search_local_variable(env, identifier);
    if (left == NULL) {
        left = search_global_variable_from_env(inter, env, identifier);
    }
    if (left != NULL) {
        // 如果是string类型 释放引用
        release_if_string(&left->value);
        left->value = v;
        // 如果是string类型 增加引用计数
        refer_if_string(&v);
    } else {
        // 增加环境变量
        if (env != NULL) {
            len_add_local_variable(env, identifier, &v);
        } else {
            LEN_add_global_variable(inter, identifier, &v);
        }
        refer_if_string(&v);
    }
    
    return v;
}

/**
 * 处理逻辑表达式
 */
static LEN_Value
eval_logical_and_or_expression(LEN_Interpreter *inter,
                               LocalEnvironment *env,
                               ExpressionType operator,
                               Expression *left, Expression *right)
{
    LEN_Value   left_val;
    LEN_Value   right_val;
    LEN_Value   result;
    
    result.type = LEN_BOOLEAN_VALUE;
    left_val = eval_expression(inter, env, left);
    
    if (left_val.type != LEN_BOOLEAN_VALUE) {
        len_runtime_error(left->line_number, NOT_BOOLEAN_TYPE_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    if (operator == LOGICAL_AND_EXPRESSION) {
        if (!left_val.u.boolean_value) {
            result.u.boolean_value = LEN_FALSE;
            return result;
        }
    } else if (operator == LOGICAL_OR_EXPRESSION) {
        if (left_val.u.boolean_value) {
            result.u.boolean_value = LEN_TRUE;
            return result;
        }
    } else {
        DBG_panic(("bad operator..%d\n", operator));
    }
    
    right_val = eval_expression(inter, env, right);
    if (right_val.type != LEN_BOOLEAN_VALUE) {
        len_runtime_error(right->line_number, NOT_BOOLEAN_TYPE_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    result.u.boolean_value = right_val.u.boolean_value;
    
    return result;
}

LEN_Value
len_eval_minus_expression(LEN_Interpreter *inter, LocalEnvironment *env,
                          Expression *operand)
{
    LEN_Value   operand_val;
    LEN_Value   result;

    operand_val = eval_expression(inter, env, operand);
    if (operand_val.type == LEN_INT_VALUE) {
        result.type = LEN_INT_VALUE;
        result.u.int_value = -operand_val.u.int_value;
    } else if (operand_val.type == LEN_DOUBLE_VALUE) {
        result.type = LEN_DOUBLE_VALUE;
        result.u.double_value = -operand_val.u.double_value;
    } else {
        len_runtime_error(operand->line_number, MINUS_OPERAND_TYPE_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    return result;
}

/**
 * 分配局部环境变量
 */
static LocalEnvironment *
alloc_local_environment()
{
    LocalEnvironment *ret;
    
    ret = MEM_malloc(sizeof(LocalEnvironment));
    ret->variable = NULL;
    ret->global_variable = NULL;
    
    return ret;
}

/**
 * 释放局部环境变量
 */
static void
dispose_local_environment(LEN_Interpreter *inter, LocalEnvironment *env)
{
    // 遍历每一个变量
    while (env->variable) {
        Variable *temp;
        temp = env->variable;
        if (env->variable->value.type == LEN_STRING_VALUE) {
            len_release_string(env->variable->value.u.string_value);
        }
        env->variable = temp->next;
        MEM_free(temp);
    }
    // 释放全局变量
    while (env->global_variable) {
        GlobalVariableRef *ref;
        ref = env->global_variable;
        env->global_variable = ref->next;
        // TODO 查看具体实现
        MEM_free(ref);
    }
    
    MEM_free(env);
}

/**
 * 执行用户函数
 */
static LEN_Value
call_lemon_function(LEN_Interpreter *inter, LocalEnvironment *env,
                      Expression *expr, FunctionDefinition *func)
{
    LEN_Value   value;
    StatementResult     result;
    ArgumentList        *arg_p;
    ParameterList       *param_p;
    LocalEnvironment    *local_env;
    
    local_env = alloc_local_environment();
    
    for (arg_p = expr->u.function_call_expression.argument,
         param_p = func->u.lemon_f.parameter;
         arg_p;
         arg_p = arg_p->next, param_p = param_p->next) {
        LEN_Value arg_val;
        
        if (param_p == NULL) {
            len_runtime_error(expr->line_number, ARGUMENT_TOO_MANY_ERR,
                              MESSAGE_ARGUMENT_END);
        }
        arg_val = eval_expression(inter, env, arg_p->expression);
        len_add_local_variable(local_env, param_p->name, &arg_val);
    }
    if (param_p) {
        len_runtime_error(expr->line_number, ARGUMENT_TOO_FEW_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    result = len_execute_statement_list(inter, local_env,
                                        func->u.lemon_f.block
                                        ->statement_list);
    if (result.type == RETURN_STATEMENT_RESULT) {
        value = result.u.return_value;
    } else {
        value.type = LEN_NULL_VALUE;
    }
    dispose_local_environment(inter, local_env);
    
    return value;
}

/**
 * 调用native函数
 */
static LEN_Value
call_native_function(LEN_Interpreter *inter, LocalEnvironment *env,
                     Expression *expr, LEN_NativeFunctionProc *proc)
{
    LEN_Value value;
    int arg_count;
    ArgumentList *arg_p;
    LEN_Value *args;
    int i;
    
    for (arg_count = 0, arg_p = expr->u.function_call_expression.argument;
         arg_p; arg_p = arg_p->next) {
        arg_count++;
    }
    
    args = MEM_malloc(sizeof(LEN_Value) * arg_count);
    
    for (arg_p = expr->u.function_call_expression.argument, i = 0;
         arg_p; arg_p = arg_p->next, i++) {
        args[i] = eval_expression(inter, env, arg_p->expression);
    }
    value = proc(inter, arg_count, args);
    for (i = 0; i < arg_count; i++) {
        release_if_string(&args[i]);
    }
    MEM_free(args);
    
    return value;
}

/**
 * 函数调用
 */
static LEN_Value
eval_function_call_expression(LEN_Interpreter *inter, LocalEnvironment *env,
                              Expression *expr)
{
    LEN_Value           value;
    FunctionDefinition  *func;
    
    char *identifier = expr->u.function_call_expression.identifier;
    
    func = len_search_function(identifier);
    if (func == NULL) {
        len_runtime_error(expr->line_number, FUNCTION_NOT_FOUND_ERR,
                          STRING_MESSAGE_ARGUMENT, "name", identifier,
                          MESSAGE_ARGUMENT_END);
    }
    switch (func->type) {
        case LEMON_FUNCTION_DEFINITION:
            value = call_lemon_function(inter, env, expr, func);
            break;
        case NATIVE_FUNCTION_DEFINITION:
            value = call_native_function(inter, env, expr, func->u.native_f.proc);
            break;
        default:
            DBG_panic(("bad case..%d\n", func->type));
    }
    
    return value;
}

/**
 * 根据表达式的类型， 求出表达式的值
 */
static LEN_Value
eval_expression(LEN_Interpreter *inter, LocalEnvironment *env,
                Expression *expr)
{
    LEN_Value   v;
    switch (expr->type) {
        case BOOLEAN_EXPRESSION:
            v = eval_boolean_expression(expr->u.boolean_value);
            break;
        case INT_EXPRESSION:
            v = eval_int_expression(expr->u.int_value);
            break;
        case DOUBLE_EXPRESSION:
            v = eval_double_expression(expr->u.double_value);
            break;
        case STRING_EXPRESSION:
            v = eval_string_expression(inter, expr->u.string_value);
            break;
        case IDENTIFIER_EXPRESSION:
            v = eval_identifier_expression(inter, env, expr);
            break;
        case ASSIGN_EXPRESSION:
            v = eval_assign_expression(inter, env,
                                       expr->u.assign_expression.variable,
                                       expr->u.assign_expression.operand);
            break;
        case ADD_EXPRESSION:        /* FALLTHRU */
        case SUB_EXPRESSION:        /* FALLTHRU */
        case MUL_EXPRESSION:        /* FALLTHRU */
        case DIV_EXPRESSION:        /* FALLTHRU */
        case MOD_EXPRESSION:        /* FALLTHRU */
        case EQ_EXPRESSION: /* FALLTHRU */
        case NE_EXPRESSION: /* FALLTHRU */
        case GT_EXPRESSION: /* FALLTHRU */
        case GE_EXPRESSION: /* FALLTHRU */
        case LT_EXPRESSION: /* FALLTHRU */
        case LE_EXPRESSION:
            v = len_eval_binary_expression(inter, env,
                                           expr->type,
                                           expr->u.binary_expression.left,
                                           expr->u.binary_expression.right);
            break;
        case LOGICAL_AND_EXPRESSION:/* FALLTHRU */
        case LOGICAL_OR_EXPRESSION:
            v = eval_logical_and_or_expression(inter, env, expr->type,
                                               expr->u.binary_expression.left,
                                               expr->u.binary_expression.right);
            break;
        case MINUS_EXPRESSION:
            v = len_eval_minus_expression(inter, env, expr->u.minus_expression);
            break;
        case FUNCTION_CALL_EXPRESSION:
            v = eval_function_call_expression(inter, env, expr);
            break;
        case NULL_EXPRESSION:
            v = eval_null_expression();
            break;
        case EXPRESSION_TYPE_COUNT_PLUS_1:  /* FALLTHRU */
        default:
            DBG_panic(("bad case. type..%d\n", expr->type));
    }
    return v;
}

/**
 * 计算表达式的值
 */
LEN_Value
len_eval_expression(LEN_Interpreter *inter, LocalEnvironment *env,
                    Expression *expr)
{
    return eval_expression(inter, env, expr);
}
