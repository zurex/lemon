//
//  memory.h
//  lemon
//
//  Created by Azure on 2018/1/28.
//  Copyright © 2018年 Azure. All rights reserved.
//

#ifndef memory_h
#define memory_h

#include "MEM.h"

typedef union Header_tag Header;


struct MEM_Controller_tag {
    FILE        *error_fp;
    MEM_ErrorHandler    error_handler;
    MEM_FailMode        fail_mode;
    Header      *block_header;
};

#endif /* memory_h */
