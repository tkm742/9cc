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
	token = tokenize(argv[1]);
	Node *node = expr();

	// アセンブリの前半部分を出力
	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

	// 抽象構文木を下りながらコード生成
	gen(node);

	// スタックトップに式全体の値が残っているはずなので
	// それをraxにロードし返り値とする
	printf("	pop rax\n");
	printf("	ret\n");
    return 0;
}
