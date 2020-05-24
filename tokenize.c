#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"


extern Token *token;
extern char *user_input;


void error_at(char *loc, char *fmt, ...){
	va_list ap;
	va_start(ap, fmt);

	int pos = loc - user_input;
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// エラー報告のための関数
// printfと同じ引数を取る
void error(char *fmt, ...){
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// 次のトークンが期待している記号の時は、トークンを１つ読み進めて
// trueを返す。それ以外はfalseを返す。
bool consume(char *op){
	if(token->kind != TK_RESERVED 
		|| strlen(op) != token->len
		|| memcmp(token->str, op, token->len)){
		return false;
	}
	token = token->next;
	return true;
}

Token *consume_ident(){
    if(token->kind != TK_IDENT){
        return NULL;
    }
    Token *tok_ident = token;
    token = token->next;
    return tok_ident;
}

// 次のトークンが期待している記号の時は、トークンを１つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *s){
	if(token->kind != TK_RESERVED 
		|| strlen(s) != token->len
		|| memcmp(token->str, s, token->len)){
		error_at(token->str, "'%s'ではありません。", s);
	}
	token = token->next;
}

// 次のトークンが数値の場合、トークンを１つ読み進めてその数値を返す。
// それ以外はエラーを報告する。
int expect_number(){
	if(token->kind != TK_NUM){
		error_at(token->str, "数ではありません。");
	}
	int val = token->val;
	token = token->next;
	return val;
}

// 次のトークンが識別子の場合、トークンを１つ読み進める。
// それ以外はエラーを報告する。
char *expect_ident(void){
	if(token->kind != TK_IDENT){
		error_at(token->str, "識別子が来るはずです。");
	}
	char *s = strndup(token->str, token->len);
	token = token->next;
	return s;
}

bool at_eof(){
	return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurにつなげる
Token *new_token(TokenKind kind, Token *cur, char *str, int len){
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	tok->len = len;
	cur->next = tok;
	return tok;
}

// 特定の文字列で始まっているか確認
bool startswith(char *p, char *q){
	return memcmp(p, q, strlen(q)) == 0;
}

int is_alnum(char c){
	return ('a' <= c && c <= 'z') ||
		   ('A' <= c && c <= 'Z') ||
		   ('0' <= c && c <= '9') ||
		   (c == '_');
}

char* strndup(char *str, size_t len) {

    char *buffer = malloc(len + 1);
    memcpy(buffer, str, len);
    buffer[len] = '\0';

    return buffer;
}

char *starts_with_reserved(char *p){

	// 予約語チェック
	static char *kw[] = {
		"return", "if", "else", "while", "for", "int", "sizeof"
	};
	for(int i = 0; i < sizeof(kw) / sizeof(*kw); i++){
		int len = strlen(kw[i]);
		if(startswith(p, kw[i]) && !is_alnum(p[len])){
			return kw[i];
		}
	}

	// ２文字演算子チェック
	static char *ops[] = {
		"==", "!=", "<=", ">="
	};
	for(int i = 0; i < sizeof(ops) / sizeof(*ops); i++){
		if(startswith(p, ops[i])){
			return ops[i];
		}
	}

	return NULL;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p){
	Token head;
	head.next = NULL;
	Token *cur = &head;

	while(*p){
		// 空白文字、改行をスキップ
		if(isspace(*p) || *p == '\n'){
			p++;
			continue;
		}

		char *kw = starts_with_reserved(p);
		if(kw){
			int len = strlen(kw);
			cur = new_token(TK_RESERVED, cur, p, len);
			p += len;
			continue;
		}

		if(strchr("+-*/()<>;={}&,[]", *p)){
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue;
		}

        if(isalpha(*p)){
			char *p_begin = p;
			int len = 0;
			while(is_alnum(*p)){
				len++;
				p++;
			}
            cur = new_token(TK_IDENT, cur, p_begin, len);
            continue;
        }

		if(isdigit(*p)){
			cur = new_token(TK_NUM, cur, p, 0);
			char *p_begin = p;
			cur->val = strtol(p, &p, 10);
			cur->len = p - p_begin;
			continue;
		}

		error_at(p, "トークナイズできません。");
	}

	new_token(TK_EOF, cur, p, 0);
	return head.next;
}
