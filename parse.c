#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"


extern Token *token;
LVar *locals;


// 変数を名前で検索する。ない場合はNULL。
LVar *find_lvar(Token *tok){
	for(LVar *var = locals; var; var = var->next){
		if(var->len == tok->len && !memcmp(tok->str, var->name, var->len)){
			return var;
		}
	}
	return NULL;
}


Node *new_node(NodeKind kind){
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	return node;
}

Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs){
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node *new_node_add(Node *lhs, Node *rhs){
	add_type(lhs);
	add_type(rhs);

	if(is_integer(lhs->ty) && is_integer(rhs->ty)){
		return new_node_binary(ND_ADD, lhs, rhs);
	}
	if(lhs->ty->base && is_integer(rhs->ty)){
		return new_node_binary(ND_PTR_ADD, lhs, rhs);
	}
	if(is_integer(lhs->ty) && rhs->ty->base){
		return new_node_binary(ND_PTR_ADD, rhs, lhs);
	}
}

Node *new_node_sub(Node *lhs, Node *rhs){
	add_type(lhs);
	add_type(rhs);

	if(is_integer(lhs->ty) && is_integer(rhs->ty)){
		return new_node_binary(ND_SUB, lhs, rhs);
	}
	if(lhs->ty->base && is_integer(rhs->ty)){
		return new_node_binary(ND_PTR_SUB, lhs, rhs);
	}
	if(lhs->ty->base && rhs->ty->base){
		return new_node_binary(ND_PTR_DIFF, lhs, rhs);
	}
}

Node *new_node_unary(NodeKind kind, Node *unary){
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = unary;
	return node;
}

Node *new_node_ifelse(Node *cond, Node *then, Node *els){
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_IF;
	node->cond = cond;
	node->then = then;
	node->els  = els;
	return node;
}

Node *new_node_while(Node *cond, Node *then){
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_WHILE;
	node->cond = cond;
	node->then = then;
	return node;
}

Node *new_node_for(Node *init, Node *cond, Node *inc, Node *then){
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_FOR;
	node->init = init;
	node->cond = cond;
	node->inc  = inc;
	node->then = then;
	return node;
}

Node *new_node_block(Node *body){
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_BLOCK;
	node->body = body;
	return node;
}

Node *new_node_num(int val){
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_NUM;
	node->val = val;
	return node;
}

Node *new_node_lvar(LVar *var){
	Node *node = new_node(ND_LVAR);
	node->lvar = var;
	return node;
}

LVar *new_lvar(char *name, Type *ty){
	LVar *var = calloc(1, sizeof(LVar));
	var->name = name;
	var->len  = strlen(name);
	var->ty   = ty;
	var->next = locals;
	locals = var;
	return var;
}

LVar *read_func_params(void){
	if(consume(")")) return NULL;

	Type *ty = basetype();
	char *name = expect_ident();
	ty = read_type_suffix(ty);
	LVar *params = new_lvar(name, ty);

	while(!consume(")")){
		expect(",");
		Type *ty = basetype();
		char *name = expect_ident();
		ty = read_type_suffix(ty);
		params = new_lvar(name, ty);
	}

	return params;
}

Function *program(){
	Function head = {};
	Function *cur = &head;

    while(!at_eof()){
        cur->next = function();
		cur = cur->next;
    }

    return head.next;
}

Type *basetype(){
	expect("int");
	Type *ty = int_type;
	while(consume("*")){
		ty = pointer_to(ty);
	}
	return ty;
}

Type *read_type_suffix(Type *base){
	if(!consume("[")){
		return base;
	}
	int array_len = expect_number();
	expect("]");
	return array_of(base, array_len);
}

Function *function(){
	locals = NULL;

	// Function 構造体を生成
	Function *fn = calloc(1, sizeof(Function));

	// 関数名をパース
	basetype();
	fn->name = expect_ident();

	// 引数をパース
	expect("(");
	fn->params = read_func_params();

	// ブロックをパース
	expect("{");

	Node head = {};
	Node *cur = &head;

	while(!consume("}")){
		cur->next = stmt();
		cur = cur->next;
	}

	fn->node = head.next;
	fn->locals = locals;

	return fn;
}

Node *declaration(){
	Type *ty = basetype();
	char *name = expect_ident();
	ty = read_type_suffix(ty);
	LVar *var = new_lvar(name, ty);

	if(consume(";")){
		return new_node(ND_NULL);
	}

	expect("=");
	Node *lhs = new_node_lvar(var);
	Node *rhs = expr();
	expect(";");

	Node *node = new_node_binary(ND_ASSIGN, lhs, rhs);
	return node;
}

Node *stmt(){
	Node *node = stmt2();
	add_type(node);
	return node;
}

Node *stmt2(){
    Node *node;

	if(consume("return")){
		Node *node = new_node_unary(ND_RETURN, expr());
		expect(";");
		return node;
	}
	else if(consume("if")){
		Node *cond, *then, *els;
		expect("(");
		cond = expr();
		expect(")");
		then = stmt();
		if(consume("else")){
			els = stmt();
		}
		return new_node_ifelse(cond, then, els);
	}
	else if(consume("while")){
		expect("(");
		node = expr();
		expect(")");
		return new_node_while(node, stmt());
	}
	else if(consume("for")){
		Node *init = NULL, *cond = NULL, *inc = NULL;
		expect("(");
		if(strncmp(token->str, ";", 1) != 0){
			init = expr();
		}
		expect(";");
		if(strncmp(token->str, ";", 1) != 0){
			cond = expr();
		}
		expect(";");
		if(strncmp(token->str, ")", 1) != 0){
			inc = expr();
		}
		expect(")");
		return new_node_for(init, cond, inc, stmt());
	}
	else if(consume("{")){
		Node head = {}; // define & initialize
		Node *cur = new_node(ND_BLOCK);
		head.next = cur;
		while(!consume("}")){
			cur->body = stmt();
			cur->next = new_node(ND_BLOCK);
			cur = cur->next;
		}
		cur->next = NULL;

		return head.next;
	}
	else if(!strncmp(token->str, "int", token->len)){
		return declaration();
	}
	else{
		node = expr();
	}

    expect(";");
    return node;
}

Node *expr(){
	return assign();
}

Node *assign(){
    Node *node = equality();

    if(consume("=")){
         node = new_node_binary(ND_ASSIGN, node, assign());
    }
    return node;
}

Node *equality(){
	Node *node = relational();

	for(;;){
		if(consume("==")){
			node = new_node_binary(ND_EQ, node, relational());
		}
		else if(consume("!=")){
			node = new_node_binary(ND_NE, node, relational());
		}
		else{
			return node;
		}
	}
}

Node *relational(){
	Node *node = add();

	for(;;){
		if(consume("<")){
			node = new_node_binary(ND_LT, node, add());
		}
		else if(consume(">")){
			node = new_node_binary(ND_LT, add(), node);
		}
		else if(consume("<=")){
			node = new_node_binary(ND_LE, node, add());
		}
		else if(consume(">=")){
			node = new_node_binary(ND_LE, add(), node);
		}
		else{
			return node;
		}
	}
}

Node *add(){
	Node *node = mul();

	for(;;){
		if(consume("+")){
			node = new_node_add(node, mul());
		}
		else if(consume("-")){
			node = new_node_binary(ND_SUB, node, mul());
		}
		else{
			return node;
		}
	}
}

Node *mul(){
	Node *node = unary();

	for(;;){
		if(consume("*")){
			node = new_node_binary(ND_MUL, node, unary());
		}
		else if(consume("/")){
			node = new_node_binary(ND_DIV, node, unary());
		}
		else{
			return node;
		}
	}
}

Node *unary(){
	if(consume("sizeof")){
		Node *node = unary();
		add_type(node);
		if(node->lvar->ty->kind == TY_INT){
			return new_node_num(4);
		}
		if(node->lvar->ty->kind == TY_PTR){
			return new_node_num(8);
		}
	}

	if(consume("+")){
		return primary();
	}
	if(consume("-")){
		return new_node_binary(ND_SUB, new_node_num(0), primary());
	}
	if(consume("*")){
		return new_node_unary(ND_DEREF, unary());
	}
	if(consume("&")){
		return new_node_unary(ND_ADDR, unary());
	}
	return postfix();
}

Node *postfix(){
	Node *node = primary();

	while(consume("[")){
		Node *exp = new_node_add(node, expr());
		expect("]");
		node = new_node_unary(ND_DEREF, exp);
	}
	return node;
}

Node *primary(){
	if(consume("(")){
		Node *node = expr();
		expect(")");
		return node;
	}

    Token *tok = consume_ident();
    if(tok){
		// function call
		if(consume("(")){
			Node *node = new_node(ND_FUNCCALL);
			node->funcname = strndup(tok->str, tok->len);
			if(consume(")")){
				node->args = NULL;
			}
			else{
				Node *head = assign();
				Node *cur = head;
				while(consume(",")){
					cur->next = assign();
					cur = cur->next;
				}
				expect(")");
				node->args = head;
			}
			return node;
		}
		// local variable
		else{
			Node *node = calloc(1, sizeof(Node));
			node->kind = ND_LVAR;

			LVar *lvar = find_lvar(tok);
			if(!lvar){
				lvar = calloc(1, sizeof(LVar));
				lvar->next = locals;
				lvar->name = tok->str;
				lvar->len = tok->len;
				locals = lvar;
			}
			node->lvar = lvar;
			return node;
		}
    }

	return new_node_num(expect_number());
}