//
//  string_pool.c
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

static LEN_String *
alloc_len_string(LEN_Interpreter *inter, char *str, LEN_Boolean is_literal)
{
    LEN_String *ret;
    ret = MEM_malloc(sizeof(LEN_String));
    ret->ref_count = 0;
    ret->is_literal = is_literal;
    ret->string = str;
    return ret;
}

LEN_String *
len_literal_to_len_string(LEN_Interpreter *inter, char *str)
{
    LEN_String *ret;
    ret = alloc_len_string(inter, str, LEN_TRUE);
    // 引用计数
    ret->ref_count = 1;
    return ret;
}

void
len_refer_string(LEN_String *str)
{
    str->ref_count++;
}

void
len_release_string(LEN_String *str)
{
    str->ref_count--;
    
    DBG_assert(str->ref_count >= 0, ("str->ref_count..%d\n",
                                     str->ref_count));
    if (str->ref_count == 0) {
        if (!str->is_literal) {
            MEM_free(str->string);
        }
        MEM_free(str);
    }
}

LEN_String *
len_create_lemon_string(LEN_Interpreter *inter, char *str)
{
    LEN_String *ret = alloc_len_string(inter, str, LEN_FALSE);
    ret->ref_count = 1;
    return ret;
}
