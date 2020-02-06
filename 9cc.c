#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//---- enum & typedef ---- 

typedef enum {
	TK_RESERVED, // 記号
	TK_NUM, // 整数トークン
	TK_EOF // 入力終わりトークン
} TokenKind;

// トークン型
typedef struct Token Token;
struct Token {
	TokenKind kind;	// トークンの種類
	Token *next; 	// 次の入力トークン
	int val; 		// kindがTK_NUMの場合、その数値
	char *str;		// トークン文字列
};

typedef enum {
	ND_ADD, // +
	ND_SUB, // -
	ND_MUL, // *
	ND_DIV, // /
	ND_NUM, // integer
} NodeKind;

// 抽象構文木のノードの型
typedef struct Node Node;
struct Node {
	NodeKind kind; // ノードの型
	Node *lhs; // 左辺
	Node *rhs; // 右辺
	int val; // kindがND_NUMのとき、その数値
};


//---- global variable ----

Token *token; // 現在注目しているトークン
char *user_input; // 入力プログラム


//---- prototype ----

Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *expr();
Node *mul();
Node *primary();
void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);
bool consume(char op);
void expect(char op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str);
Token *tokenize(char *p);


//---- functions ----

Node *new_node(NodeKind kind, Node *lhs, Node *rhs){
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node *new_node_num(int val){
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_NUM;
	node->val = val;
	return node;
}

Node *expr(){
	Node *node = mul();

	for(;;){
		if(consume('+')){
			node = new_code(ND_ADD, node, mul());
		}
		else if(consume('-')){
			node = new_code(ND_SUB, node, mul());
		}
		else{
			return node;
		}
	}
}

Node *mul(){
	Node *node = primary();

	for(;;){
		if(consume('*')){
			node = new_node(ND_MUL, node, primary());
		}
		else if(consume('/')){
			node = new_node(ND_DIV, node, primary());
		}
		else{
			return node;
		}
	}
}

Node *primary(){
	if(consume('(')){
		Node *node = expr();
		expect(')');
		return node;
	}

	return new_node_num(expect_number());
}

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
bool consume(char op){
	if(token->kind != TK_RESERVED || token->str[0] != op){
		return false;
	}
	token = token->next;
	return true;
}

// 次のトークンが期待している記号の時は、トークンを１つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char op){
	if(token->kind != TK_RESERVED || token->str[0] != op){
		error_at(token->str, "'%c'ではありません。", op);
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

bool at_eof(){
	return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurにつなげる
Token *new_token(TokenKind kind, Token *cur, char *str){
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	cur->next = tok;
	return tok;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p){
	Token head;
	head.next = NULL;
	Token *cur = &head;

	while(*p){
		// 空白文字をスキップ
		if(isspace(*p)){
			p++;
			continue;
		}

		if(*p == '+' ||  *p == '-'){
			cur = new_token(TK_RESERVED, cur, p++);
			continue;
		}

		if(isdigit(*p)){
			cur = new_token(TK_NUM, cur, p);
			cur->val = strtol(p, &p, 10);
			continue;
		}

		error_at(p, "トークナイズできません。");
	}

	new_token(TK_EOF, cur, p);
	return head.next;
}



int main(int argc, char **argv){

    if(argc != 2){
        fprintf(stderr, "コマンドライン引数の数が正しくありません。\n");
        return 1;
    }

	user_input = argv[1];

	// トークナイズする
	token = tokenize(argv[1]);

	// アセンブリの前半部分を出力
	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

	// 式の最初が数かチェックし、最初のmov命令を出力
	printf("	mov rax, %d\n", expect_number());

	// '+<数>'あるいは'-<数>'というトークンｎ並びを消費しつつ
	// アセンブリを出力
	while(!at_eof()){
		if(consume('+')){
			printf("	add rax, %d\n", expect_number());
			continue;
		}

		expect('-');
		printf("	sub rax, %d\n", expect_number());
	}

    printf("    ret\n");
    return 0;
}
