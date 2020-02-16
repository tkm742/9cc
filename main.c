#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

Token *token; // 現在注目しているトークン
Node *code[100];
char *user_input; // 入力プログラム
extern char *funcname;


int main(int argc, char **argv){

    if(argc != 2){
        fprintf(stderr, "コマンドライン引数の数が正しくありません。\n");
        return 1;
    }

	// トークナイズして、抽象構文木を生成
	user_input = argv[1];
	token = tokenize(argv[1]);
    Function *prog = program();

    for(Function *fn = prog; fn; fn = fn->next){
        int offset = 0;
        for(LVar *lvar = prog->locals; lvar; lvar = lvar->next){
            offset += 8;
            lvar->offset = offset;
        }
        fn->stack_size = offset;
    }

	// // アセンブリの前半部分を出力
	printf(".intel_syntax noprefix\n");
	// printf(".global main\n");
	// printf("main:\n");
    for(Function *fn = prog; fn; fn = fn->next){
        printf(".global %s\n", fn->name);
        printf("%s:\n", fn->name);
        funcname = fn->name;

        // プロローグ
        // 変数26個分の領域を確保する
        printf("  push rbp\n");
        printf("  mov rbp, rsp\n");
        printf("  sub rsp, %d\n", fn->stack_size); 

        // 先頭の式から、抽象構文木を下りコード生成
        for(Node *node = fn->node; node; node = node->next){
            gen(node);
        }

        // エピローグ
        // 最後の式の結果がRAXに残っているので、それが返り値
        printf(".Lreturn_%s:\n", funcname);
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
    }

    return 0;
}
