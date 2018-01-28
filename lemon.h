//
//  lemon.h
//  lemon
//
//  Created by Azure on 2018/1/28.
//  Copyright © 2018年 Azure. All rights reserved.
//

#ifndef lemon_h
#define lemon_h

#include "MEM.h"
#include "LEN.h"
#include "LEN_dev.h"

#define smaller(a, b) ((a) < (b) ? (a) : (b))
#define larger(a, b) ((a) > (b) ? (a) : (b))

#define MESSAGE_ARGUMENT_MAX    (256)
#define LINE_BUF_SIZE           (1024)

/********************************** 开始错误信息定义 *****************************/

/**编译错误*/
typedef enum {
    PARSE_ERR = 1,
    CHARACTER_INVALID_ERR,
    FUNCTION_MULTIPLE_DEFINE_ERR,
    COMPILE_ERROR_COUNT_PLUS_1
} CompileError;

/**运行时错误*/
typedef enum {
    VARIABLE_NOT_FOUND_ERR = 1,
    FUNCTION_NOT_FOUND_ERR,
    ARGUMENT_TOO_MANY_ERR,
    ARGUMENT_TOO_FEW_ERR,
    NOT_BOOLEAN_TYPE_ERR,
    MINUS_OPERAND_TYPE_ERR,
    BAD_OPERAND_TYPE_ERR,
    NOT_BOOLEAN_OPERATOR_ERR,
    FOPEN_ARGUMENT_TYPE_ERR,
    FCLOSE_ARGUMENT_TYPE_ERR,
    FGETS_ARGUMENT_TYPE_ERR,
    FPUTS_ARGUMENT_TYPE_ERR,
    NOT_NULL_OPERATOR_ERR,
    DIVISION_BY_ZERO_ERR,
    GLOBAL_VARIABLE_NOT_FOUND_ERR,
    GLOBAL_STATEMENT_IN_TOPLEVEL_ERR,
    BAD_OPERATOR_FOR_STRING_ERR,
    RUNTIME_ERROR_COUNT_PLUS_1
} RuntimeError;

/**
 * 消息参数类型定义
 */
typedef enum {
    INT_MESSAGE_ARGUMENT = 1,
    DOUBLE_MESSAGE_ARGUMENT,
    STRING_MESSAGE_ARGUMENT,
    CHARACTER_MESSAGE_ARGUMENT,
    POINTER_MESSAGE_ARGUMENT,
    MESSAGE_ARGUMENT_END
} MessageArgumentType;

/**
 * 消息格式定义
 */
typedef struct {
    char *format;
} MessageFormat;

/*************************************** 结束错误信息定义 **********************/

typedef struct Expression_tag Expression;

/**
 * 参数定义
 */
typedef struct ParameterList_tag {
    /**参数名*/
    char *name;
    /**下一个参数*/
    struct ParameterList_tag *next;
} ParameterList;


typedef struct ArgumentList_tag {
    Expression *expression;
    struct ArgumentList_tag *next;
} ArgumentList;

/***************************************开始表达式定义**********************************/

/**
 * 表达式类型定义
 */
typedef enum {
    BOOLEAN_EXPRESSION = 1,
    INT_EXPRESSION,
    DOUBLE_EXPRESSION,
    STRING_EXPRESSION,
    IDENTIFIER_EXPRESSION,
    ASSIGN_EXPRESSION,
    ADD_EXPRESSION,
    SUB_EXPRESSION,
    MUL_EXPRESSION,
    DIV_EXPRESSION,
    MOD_EXPRESSION,
    EQ_EXPRESSION,
    NE_EXPRESSION,
    GT_EXPRESSION,
    GE_EXPRESSION,
    LT_EXPRESSION,
    LE_EXPRESSION,
    LOGICAL_AND_EXPRESSION,
    LOGICAL_OR_EXPRESSION,
    MINUS_EXPRESSION,
    FUNCTION_CALL_EXPRESSION,
    NULL_EXPRESSION,
    EXPRESSION_TYPE_COUNT_PLUS_1
} ExpressionType;

/**判断是否是数值操作符*/
#define dkc_is_math_operator(operator) \
((operator) == ADD_EXPRESSION || (operator) == SUB_EXPRESSION\
|| (operator) == MUL_EXPRESSION || (operator) == DIV_EXPRESSION\
|| (operator) == MOD_EXPRESSION)

/**判断是否是比较操作符*/
#define dkc_is_compare_operator(operator) \
((operator) == EQ_EXPRESSION || (operator) == NE_EXPRESSION\
|| (operator) == GT_EXPRESSION || (operator) == GE_EXPRESSION\
|| (operator) == LT_EXPRESSION || (operator) == LE_EXPRESSION)

/**判断是否是逻辑操作符*/
#define dkc_is_logical_operator(operator) \
((operator) == LOGICAL_AND_EXPRESSION || (operator) == LOGICAL_OR_EXPRESSION)

/**
 * 赋值表达式
 */
typedef struct {
    char        *variable;
    Expression  *operand;
} AssignExpression;

/**
 * 二元表达式
 */
typedef struct {
    Expression  *left;
    Expression  *right;
} BinaryExpression;

/**
 * 函数调用表达式
 */
typedef struct {
    char                *identifier;
    ArgumentList        *argument;
} FunctionCallExpression;

/**
 * 表达式定义
 */
struct Expression_tag {
    /**表达式类型*/
    ExpressionType type;
    /**行号*/
    int line_number;
    union {
        LEN_Boolean             boolean_value;
        int                     int_value;
        double                  double_value;
        char                    *string_value;
        char                    *identifier;
        AssignExpression        assign_expression;
        BinaryExpression        binary_expression;
        Expression              *minus_expression;
        FunctionCallExpression  function_call_expression;
    } u;
};

/*************************************开始语句定义*********************************/
typedef struct Statement_tag Statement;

/**
 * 语句链表定义
 */
typedef struct StatementList_tag {
    Statement   *statement;
    struct StatementList_tag    *next;
} StatementList;

/**
 * 函数主体定义
 */
typedef struct {
    StatementList *statement_list;
} Block;

typedef struct IdentifierList_tag {
    char        *name;
    struct IdentifierList_tag   *next;
} IdentifierList;

/**
 * 语句类型定义
 */
typedef enum {
    EXPRESSION_STATEMENT = 1,
    GLOBAL_STATEMENT,
    IF_STATEMENT,
    WHILE_STATEMENT,
    FOR_STATEMENT,
    RETURN_STATEMENT,
    BREAK_STATEMENT,
    CONTINUE_STATEMENT,
    STATEMENT_TYPE_COUNT_PLUS_1
} StatementType;

typedef struct {
    IdentifierList      *identifier_list;
} GlobalStatement;

typedef struct Elsif_tag {
    Expression  *condition;
    Block       *block;
    struct Elsif_tag    *next;
} Elsif;

typedef struct {
    Expression  *condition;
    Block       *then_block;
    Elsif       *elsif_list;
    Block       *else_block;
} IfStatement;

typedef struct {
    Expression  *condition;
    Block       *block;
} WhileStatement;

typedef struct {
    Expression  *init;
    Expression  *condition;
    Expression  *post;
    Block       *block;
} ForStatement;

typedef struct {
    Expression *return_value;
} ReturnStatement;

/**
 * 语句定义
 */
struct Statement_tag {
    /**语句类型*/
    StatementType type;
    /**行号*/
    int line_number;
    union {
        /**表达式语句*/
        Expression *expression_s;
        /**global语句*/
        GlobalStatement global_s;
        /**if语句*/
        IfStatement if_s;
        /**while语句*/
        WhileStatement while_s;
        /**for语句*/
        ForStatement for_s;
        /**return语句*/
        ReturnStatement return_s;
    } u;
};



/**
 * 语句结果类型定义
 */
typedef enum {
    NORMAL_STATEMENT_RESULT = 1,
    RETURN_STATEMENT_RESULT,
    BREAK_STATEMENT_RESULT,
    CONTINUE_STATEMENT_RESULT,
    STATEMENT_RESULT_TYPE_COUNT_PLUS_1
} StatementResultType;

/**
 * 语句结果定义
 */
typedef struct {
    StatementResultType type;
    union {
        LEN_Value       return_value;
    } u;
} StatementResult;



/**
 * 函数类型定义
 */
typedef enum{
    /**用户定义的函数*/
    LEMON_FUNCTION_DEFINITION = -1,
    /**内置函数*/
    NATIVE_FUNCTION_DEFINITION
} FunctionDefinitionType;

/**
 * 函数定义
 */
typedef struct FunctionDefinition_tag{
    /**函数名*/
    char *name;
    /**函数类型*/
    FunctionDefinitionType type;
    union {
        struct {
            /**参数的定义*/
            ParameterList *parameter;
            /**函数主体*/
            Block *block;
        } lemon_f;
        struct {
            LEN_NativeFunctionProc *proc;
        } native_f;
    } u;
    /**下一个函数*/
    struct FunctionDefinition_tag *next;
} FunctionDefinition;

/******************************* 开始变量信息定义 ********************************/

/**
 * 变量定义
 */
typedef struct Variable_tag{
    /**变量名*/
    char *name;
    /**变量值*/
    LEN_Value value;
    /**下一个变量*/
    struct Variable_tag *next;
} Variable;

/**
 * 全局变量引用定义
 */
typedef struct GlobalVariableRef_tag {
    Variable    *variable;
    struct GlobalVariableRef_tag *next;
} GlobalVariableRef;

/**
 * 局部环境变量定义
 */
typedef struct {
    Variable    *variable;
    GlobalVariableRef   *global_variable;
} LocalEnvironment;

/***********************************/

/**
 * string类型定义
 */
struct LEN_String_tag {
    int         ref_count;
    char        *string;
    LEN_Boolean is_literal;
};

/**
 * string池定义
 */
typedef struct {
    LEN_String  *strings;
} StringPool;

/**
 * 解释器定义
 */
struct LEN_Interpreter_tag {
    /**解释器的内存*/
    MEM_Storage interpreter_storage;
    /**运行时的内存*/
    MEM_Storage execute_storage;
    /**全局变量链表*/
    Variable *variable;
    /**函数定义链表*/
    FunctionDefinition *function_list;
    /**语句链表*/
    StatementList *statement_list;
    /**当前行号*/
    int current_line_number;
};
/*************************************函数声明**************************************/

/* create.c */
void len_function_define(char *identifier, ParameterList *parameter_list,
                         Block *block);
ParameterList *len_create_parameter(char *identifier);
ParameterList *len_chain_parameter(ParameterList *list,
                                   char *identifier);
ArgumentList *len_create_argument_list(Expression *expression);
ArgumentList *len_chain_argument_list(ArgumentList *list, Expression *expr);
StatementList *len_create_statement_list(Statement *statement);
StatementList *len_chain_statement_list(StatementList *list,
                                        Statement *statement);
Expression *len_create_assign_expression(char *variable,
                                             Expression *operand);
Expression *len_create_minus_expression(Expression *operand);
Expression *len_create_identifier_expression(char *identifier);
Expression *len_create_function_call_expression(char *func_name,
                                                ArgumentList *argument);
Expression *len_create_boolean_expression(LEN_Boolean value);
Expression *len_create_null_expression(void);
Statement *len_create_global_statement(IdentifierList *identifier_list);
IdentifierList *len_create_global_identifier(char *identifier);
IdentifierList *len_chain_identifier(IdentifierList *list, char *identifier);
Statement *len_create_if_statement(Expression *condition,
                                    Block *then_block, Elsif *elsif_list,
                                    Block *else_block);
Elsif *len_chain_elsif_list(Elsif *list, Elsif *add);
Elsif *len_create_elsif(Expression *expr, Block *block);
Statement *len_create_while_statement(Expression *condition, Block *block);
Statement *len_create_for_statement(Expression *init, Expression *cond,
                                    Expression *post, Block *block);
Block *len_create_block(StatementList *statement_list);
Statement *len_create_expression_statement(Expression *expression);
Statement *len_create_return_statement(Expression *expression);
Statement *len_create_break_statement(void);
Statement *len_create_continue_statement(void);
/**创建二元表达式*/
Expression *len_create_binary_expression(ExpressionType operator,
                                         Expression *left,
                                         Expression *right);

/**为指定的表达式类型分配内存*/
Expression *len_alloc_expression(ExpressionType type);

/* execute.c */
StatementResult len_execute_statement_list(LEN_Interpreter *inter, LocalEnvironment *env, StatementList *list);

/* string_pool.c */
/**将char数组转为String类型*/
LEN_String *len_literal_to_len_string(LEN_Interpreter *inter, char *str);
void len_refer_string(LEN_String *str);
void len_release_string(LEN_String *str);
LEN_String *len_search_len_string(LEN_Interpreter *inter, char *str);
LEN_String *len_create_lemon_string(LEN_Interpreter *inter, char *str);

/* util.c */
/**获取当前的解释器*/
LEN_Interpreter *len_get_current_interpreter(void);
/**设置当前的解释器*/
void len_set_current_interpreter(LEN_Interpreter *inter);
/**将指定的表达式转为字符串形式*/
char *len_get_operator_string(ExpressionType type);
/**根据符号查找局部变量*/
Variable *len_search_local_variable(LocalEnvironment *env, char *identifier);
/**根据符号查找全局变量*/
Variable *len_search_global_variable(LEN_Interpreter *inter, char *identifier);
/**分配指定大小的内存*/
void *len_malloc(size_t size);
/**增加局部变量*/
void len_add_local_variable(LocalEnvironment *env,char *identifier, LEN_Value *value);
/**根据函数名查找指定的函数*/
FunctionDefinition *len_search_function(char *name);

/* eval.c */
LEN_Value len_eval_binary_expression(LEN_Interpreter *inter,
                                     LocalEnvironment *env,
                                     ExpressionType operator,
                                     Expression *left, Expression *right);

LEN_Value len_eval_minus_expression(LEN_Interpreter *inter,
                                    LocalEnvironment *env, Expression *operand);
LEN_Value len_eval_expression(LEN_Interpreter *inter,
                              LocalEnvironment *env, Expression *expr);
/* error.c */
void len_compile_error(CompileError id, ...);
void len_runtime_error(int line_number, RuntimeError id, ...);

/* native.c */
LEN_Value len_nv_print_proc(LEN_Interpreter *interpreter, int arg_count, LEN_Value *args);
LEN_Value len_nv_fopen_proc(LEN_Interpreter *interpreter,
                            int arg_count, LEN_Value *args);
LEN_Value len_nv_fclose_proc(LEN_Interpreter *interpreter,
                             int arg_count, LEN_Value *args);
LEN_Value len_nv_fgets_proc(LEN_Interpreter *interpreter,
                            int arg_count, LEN_Value *args);
LEN_Value len_nv_fputs_proc(LEN_Interpreter *interpreter,
                            int arg_count, LEN_Value *args);
void len_add_std_fp(LEN_Interpreter *inter);

/* string.c */
char *len_create_identifier(char *str);
void len_open_string_literal(void);
void len_add_string_literal(int letter);
void len_reset_string_literal_buffer(void);
char *len_close_string_literal(void);

#endif /* lemon_h */
