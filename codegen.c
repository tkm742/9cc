#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

int cnt_Lend;
int cnt_Lelse;
int cnt_Lbegin;

void gen_lval(Node *node){
    if(node->kind != ND_LVAR){
        error("代入の左辺値が変数ではありません。");
    }

    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->offset);
    printf("    push rax\n");
}


void gen(Node *node){
	if(node == NULL) return;

	int cnt_Lelse_tmp;
	int cnt_Lbegin_tmp;
	int cnt_Lend_tmp;

    switch(node->kind){
	case ND_RETURN:
		gen(node->lhs);
		printf("	pop rax\n");
		printf("	mov rsp, rbp\n");
		printf("	pop rbp\n");
		printf("	ret\n");
		return;
	case ND_IF:
		if(node->els){
			cnt_Lelse_tmp = cnt_Lelse++;
			cnt_Lend_tmp = cnt_Lend++;
			gen(node->cond);
			printf("	pop rax\n");
			printf("	cmp rax, 0\n");
			printf("	je .Lelse%03d\n", cnt_Lelse_tmp);
			gen(node->then);
			printf("	jmp .Lend%03d\n", cnt_Lend_tmp);
			printf(".Lelse%03d:\n", cnt_Lelse_tmp);
			gen(node->els);
			printf(".Lend%03d:\n", cnt_Lend_tmp);
		}
		else{
			cnt_Lend_tmp = cnt_Lend++;
			gen(node->cond);
			printf("	pop rax\n");
			printf("	cmp rax, 0\n");
			printf("	je .Lend%03d\n", cnt_Lend_tmp);
			gen(node->then);
			printf(".Lend%03d:\n", cnt_Lend_tmp);
		}
		return;
	case ND_WHILE:
		cnt_Lbegin_tmp = cnt_Lbegin++;
		cnt_Lend_tmp = cnt_Lend++;
		printf(".Lbegin%03d:\n", cnt_Lbegin_tmp);
		gen(node->cond);
		printf("	pop rax\n");
		printf("	cmp rax, 0\n");
		printf("	je .Lend%03d\n", cnt_Lend_tmp);
		gen(node->then);
		printf("	jmp .Lbegin%03d\n", cnt_Lbegin_tmp);
		printf(".Lend%03d:\n", cnt_Lend_tmp);
		return;
	case ND_FOR:
		cnt_Lbegin_tmp = cnt_Lbegin++;
		cnt_Lend_tmp = cnt_Lend++;
		gen(node->init);
		printf(".Lbegin%03d:\n", cnt_Lbegin_tmp);
		gen(node->cond);
		printf("	pop rax\n");
		printf("	cmp rax, 0\n");
		printf("	je .Lend%03d\n", cnt_Lend_tmp);
		gen(node->then);
		gen(node->inc);
		printf("	jmp .Lbegin%03d\n", cnt_Lbegin_tmp);
		printf(".Lend%03d:", cnt_Lend_tmp);
		return;
    case ND_NUM:
        printf("    push %d\n", node->val);
        return;
    case ND_LVAR:
        gen_lval(node);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    case ND_ASSIGN:
        gen_lval(node->lhs);
        gen(node->rhs);
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");
        return;
    }

	gen(node->lhs);
	gen(node->rhs);

	printf("	pop rdi\n");
	printf("	pop rax\n");

	switch(node->kind){
	case ND_ADD:
		printf("	add rax, rdi\n");
		break;
	case ND_SUB:
		printf("	sub rax, rdi\n");
		break;
	case ND_MUL:
		printf("	imul rax, rdi\n");
		break;
	case ND_DIV:
		printf("	cqo\n");
		printf("	idiv rdi\n");
		break;
	case ND_EQ:
		printf("	cmp rax, rdi\n");
		printf("	sete al\n");
		printf("	movzb rax, al\n");
		break;
	case ND_NE:
		printf("	cmp rax, rdi\n");
		printf("	setne al\n");
		printf("	movzb rax, al\n");
		break;
	case ND_LT:
		printf("	cmp rax, rdi\n");
		printf("	setl al\n");
		printf("	movzb rax, al\n");
		break;
	case ND_LE:
		printf("	cmp rax, rdi\n");
		printf("	setle al\n");
		printf("	movzb rax, al\n");
		break;
	}

	printf("	push rax\n");
}


