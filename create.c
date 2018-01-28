//
//  create.c
//  lemon
//
//  这个文件主要用来构建分析树
//
//  Created by Azure on 2018/1/28.
//  Copyright © 2018年 Azure. All rights reserved.
//

#include <stdio.h>
#include "MEM.h"
#include "DBG.h"
#include "lemon.h"

void
len_function_define(char *identifier, ParameterList *parameter_list,
                    Block *block)
{
    FunctionDefinition *f;
    LEN_Interpreter *inter;

    if (len_search_function(identifier)) {
        len_compile_error(FUNCTION_MULTIPLE_DEFINE_ERR,
                          STRING_MESSAGE_ARGUMENT, "name", identifier,
                          MESSAGE_ARGUMENT_END);
        return;
    }
    inter = len_get_current_interpreter();

    f = len_malloc(sizeof(FunctionDefinition));
    f->name = identifier;
    f->type = LEMON_FUNCTION_DEFINITION;
    f->u.lemon_f.parameter = parameter_list;
    f->u.lemon_f.block = block;
    f->next = inter->function_list;
    inter->function_list = f;
}

ParameterList *
len_create_parameter(char *identifier)
{
    ParameterList       *p;

    p = len_malloc(sizeof(ParameterList));
    p->name = identifier;
    p->next = NULL;

    return p;
}

ParameterList *
len_chain_parameter(ParameterList *list, char *identifier)
{
    ParameterList *pos;

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = len_create_parameter(identifier);

    return list;
}

ArgumentList *
len_create_argument_list(Expression *expression)
{
    ArgumentList *al;

    al = len_malloc(sizeof(ArgumentList));
    al->expression = expression;
    al->next = NULL;

    return al;
}

ArgumentList *
len_chain_argument_list(ArgumentList *list, Expression *expr)
{
    ArgumentList *pos;

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = len_create_argument_list(expr);

    return list;
}

StatementList *
len_create_statement_list(Statement *statement)
{
    StatementList *sl;

    sl = len_malloc(sizeof(StatementList));
    sl->statement = statement;
    sl->next = NULL;

    return sl;
}

StatementList *
len_chain_statement_list(StatementList *list, Statement *statement)
{
    StatementList *pos;

    if (list == NULL)
        return len_create_statement_list(statement);

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = len_create_statement_list(statement);

    return list;
}

Expression *
len_create_assign_expression(char *variable, Expression *operand)
{
    Expression *exp;

    exp = len_alloc_expression(ASSIGN_EXPRESSION);
    exp->u.assign_expression.variable = variable;
    exp->u.assign_expression.operand = operand;

    return exp;
}

/**
 * 根据值类型，修改表达式类型
 */
static Expression
convert_value_to_expression(LEN_Value *v)
{
    Expression  expr;
    
    if (v->type == LEN_INT_VALUE) {
        expr.type = INT_EXPRESSION;
        expr.u.int_value = v->u.int_value;
    } else if (v->type == LEN_DOUBLE_VALUE) {
        expr.type = DOUBLE_EXPRESSION;
        expr.u.double_value = v->u.double_value;
    } else {
        DBG_assert(v->type == LEN_BOOLEAN_VALUE,
                   ("v->type..%d\n", v->type));
        expr.type = BOOLEAN_EXPRESSION;
        expr.u.boolean_value = v->u.boolean_value;
    }
    return expr;
}

/**
 * 构造二元运算表达式
 */
Expression *
len_create_binary_expression(ExpressionType operator, Expression *left, Expression *right){
    /* 常量折叠优化
     具体来说，对于纯粹是常量构成的表达式，在编译时提前计算结果
     */
    if ((left->type == INT_EXPRESSION
         || left->type == DOUBLE_EXPRESSION)
        && (right->type == INT_EXPRESSION
            || right->type == DOUBLE_EXPRESSION)) {
            LEN_Value v;
            v = len_eval_binary_expression(len_get_current_interpreter(),
                                           NULL, operator, left, right);
            // 用计算的结果复写左表达式.
            *left = convert_value_to_expression(&v);
            
            return left;
    } else {
        Expression *exp;
        exp = len_alloc_expression(operator);
        exp->u.binary_expression.left = left;
        exp->u.binary_expression.right = right;
        return exp;
    }
}

/**
 * 为指定的表达式类型分配内存，并且设定表达式类型和行号
 */
Expression *
len_alloc_expression(ExpressionType type){
    Expression *exp;
    exp = len_malloc(sizeof(Expression));
    exp->type = type;
    exp->line_number = len_get_current_interpreter()->current_line_number;
    return exp;
}

Expression *
len_create_minus_expression(Expression *operand)
{
    if (operand->type == INT_EXPRESSION
        || operand->type == DOUBLE_EXPRESSION) {
        LEN_Value       v;
        v = len_eval_minus_expression(len_get_current_interpreter(),
                                      NULL, operand);
        /* Notice! Overwriting operand expression. */
        *operand = convert_value_to_expression(&v);
        return operand;
    } else {
        Expression      *exp;
        exp = len_alloc_expression(MINUS_EXPRESSION);
        exp->u.minus_expression = operand;
        return exp;
    }
}

Expression *
len_create_identifier_expression(char *identifier)
{
    Expression  *exp;

    exp = len_alloc_expression(IDENTIFIER_EXPRESSION);
    exp->u.identifier = identifier;

    return exp;
}

Expression *
len_create_function_call_expression(char *func_name, ArgumentList *argument)
{
    Expression  *exp;

    exp = len_alloc_expression(FUNCTION_CALL_EXPRESSION);
    exp->u.function_call_expression.identifier = func_name;
    exp->u.function_call_expression.argument = argument;

    return exp;
}

Expression *
len_create_boolean_expression(LEN_Boolean value)
{
    Expression *exp;

    exp = len_alloc_expression(BOOLEAN_EXPRESSION);
    exp->u.boolean_value = value;

    return exp;
}

Expression *
len_create_null_expression(void)
{
    Expression  *exp;

    exp = len_alloc_expression(NULL_EXPRESSION);

    return exp;
}

static Statement *
alloc_statement(StatementType type)
{
    Statement *st;

    st = len_malloc(sizeof(Statement));
    st->type = type;
    st->line_number = len_get_current_interpreter()->current_line_number;

    return st;
}

Statement *
len_create_global_statement(IdentifierList *identifier_list)
{
    Statement *st;

    st = alloc_statement(GLOBAL_STATEMENT);
    st->u.global_s.identifier_list = identifier_list;

    return st;
}

IdentifierList *
len_create_global_identifier(char *identifier)
{
    IdentifierList      *i_list;

    i_list = len_malloc(sizeof(IdentifierList));
    i_list->name = identifier;
    i_list->next = NULL;

    return i_list;
}

IdentifierList *
len_chain_identifier(IdentifierList *list, char *identifier)
{
    IdentifierList *pos;

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = len_create_global_identifier(identifier);

    return list;
}

Statement *
len_create_if_statement(Expression *condition,
                        Block *then_block, Elsif *elsif_list,
                        Block *else_block)
{
    Statement *st;

    st = alloc_statement(IF_STATEMENT);
    st->u.if_s.condition = condition;
    st->u.if_s.then_block = then_block;
    st->u.if_s.elsif_list = elsif_list;
    st->u.if_s.else_block = else_block;

    return st;
}

Elsif *
len_chain_elsif_list(Elsif *list, Elsif *add)
{
    Elsif *pos;

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = add;

    return list;
}

Elsif *
len_create_elsif(Expression *expr, Block *block)
{
    Elsif *ei;

    ei = len_malloc(sizeof(Elsif));
    ei->condition = expr;
    ei->block = block;
    ei->next = NULL;

    return ei;
}

Statement *
len_create_while_statement(Expression *condition, Block *block)
{
    Statement *st;

    st = alloc_statement(WHILE_STATEMENT);
    st->u.while_s.condition = condition;
    st->u.while_s.block = block;

    return st;
}

Statement *
len_create_for_statement(Expression *init, Expression *cond,
                         Expression *post, Block *block)
{
    Statement *st;

    st = alloc_statement(FOR_STATEMENT);
    st->u.for_s.init = init;
    st->u.for_s.condition = cond;
    st->u.for_s.post = post;
    st->u.for_s.block = block;

    return st;
}

Block *
len_create_block(StatementList *statement_list)
{
    Block *block;

    block = len_malloc(sizeof(Block));
    block->statement_list = statement_list;

    return block;
}

Statement *
len_create_expression_statement(Expression *expression)
{
    Statement *st;

    st = alloc_statement(EXPRESSION_STATEMENT);
    st->u.expression_s = expression;

    return st;
}

Statement *
len_create_return_statement(Expression *expression)
{
    Statement *st;

    st = alloc_statement(RETURN_STATEMENT);
    st->u.return_s.return_value = expression;

    return st;
}

Statement *len_create_break_statement(void)
{
    return alloc_statement(BREAK_STATEMENT);
}

Statement *len_create_continue_statement(void)
{
    return alloc_statement(CONTINUE_STATEMENT);
}
