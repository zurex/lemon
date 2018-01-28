//
//  main.c
//  lemon
//
//  Created by Azure on 2018/1/28.
//  Copyright © 2018年 Azure. All rights reserved.
//

#include <stdio.h>
#include "LEN.h"
#include "MEM.h"

int
main(int argc, char **argv)
{
    LEN_Interpreter     *interpreter;
    FILE *fp;
    
    if (argc != 2) {
        fprintf(stderr, "usage:%s filename", argv[0]);
        exit(1);
    }
    
    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "%s not found.\n", argv[1]);
        exit(1);
    }
    interpreter = LEN_create_interpreter();
    LEN_compile(interpreter, fp);
    LEN_interpret(interpreter);
    LEN_dispose_interpreter(interpreter);
    
    MEM_dump_blocks(stdout);
    
    return 0;
}

