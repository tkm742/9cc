#include <stdlib.h>
#include "9cc.h"

// Type型構造体を生成
Type *int_type = &(Type){ TY_INT };

bool is_integer(Type *ty){
    return ty->kind == TY_INT;
}



