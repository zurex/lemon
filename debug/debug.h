//
//  debug.h
//  lemon
//
//  Created by Azure on 2018/1/28.
//  Copyright © 2018年 Azure. All rights reserved.
//

#ifndef debug_h
#define debug_h

#include <stdio.h>
#include "DBG.h"

struct DBG_Controller_tag {
    FILE        *debug_write_fp;
    int         current_debug_level;
};

#endif /* debug_h */
