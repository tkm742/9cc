#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

Token *token; // 現在注目しているトークン
char *user_input; // 入力プログラム


int main(int argc, char **argv){

    if(argc != 2){
        fprintf(stderr, "コマンドライン引数の数が正しくありません。\n");
        return 1;
    }

	// トークナイズして、抽象構文木を生成
	user_input = argv[1];
	token = tokenize(user_input);
    Function *prog = program();

    // ローカル変数の offset を設定
    for(Function *fn = prog; fn; fn = fn->next){
        int offset = 0;
        for(LVar *lvar = prog->locals; lvar; lvar = lvar->next){
            offset += 8;
            lvar->offset = offset;
        }
        fn->stack_size = offset;
    }

    codegen(prog);

    return 0;
}
