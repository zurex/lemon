//
//  error_message.c
//  lemon
//
//  Created by Azure on 2018/1/28.
//  Copyright © 2018年 Azure. All rights reserved.
//

#include "lemon.h"

MessageFormat len_compile_error_message_format[] = {
    { "dummy" },
    { "($(token)附近有语法错误)"},
    { "错误的字符($(bad_char))"},
    { "函数名重复($(name))"},
    { "dummy" },
};

MessageFormat len_runtime_error_message_format[] = {
    { "dummy" },
    { "找不到变量($(name))" },
    { "找不到函数($(name))" },
    { "传递的参数多于函数所要求的参数。" },
    { "传递的参数小于函数所要求的参数。" },
    { "dummy" },
};
