#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"


int cnt_label;
char *funcname;
char *argreg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

void load(){
	printf("  pop rax\n");
	printf("  mov rax, [rax]\n");
	printf("  push rax\n");
}

void store(){
	printf("  pop rdi\n");
	printf("  pop rax\n");
	printf("  mov [rax], rdi\n");
	printf("  push rdi\n");
}

void gen_lval(Node *node){
    if(node->ty->kind == TY_ARRAY){
		fprintf(stderr, "左辺値ではありません");
		exit(1);
	}

	gen_addr(node);
}

void gen_addr(Node *node){
	switch(node->kind){
	case ND_LVAR:
		printf("  mov rax, rbp\n");
		printf("  sub rax, %d\n", node->lvar->offset);
		printf("  push rax\n");
		return;
	case ND_DEREF:
		gen(node->lhs);
		return;
	}

	fprintf(stderr, "左辺値ではありません");
	exit(1);
}

void codegen(Function *prog){
		
	// アセンブリの前半部分を出力
	printf(".intel_syntax noprefix\n");
    for(Function *fn = prog; fn; fn = fn->next){
        printf(".global %s\n", fn->name);
        printf("%s:\n", fn->name);
        funcname = fn->name;

        // プロローグ
        // ローカル変数の領域を確保する
        printf("  push rbp\n");
        printf("  mov rbp, rsp\n");
        printf("  sub rsp, %d\n", fn->stack_size); 

        // 関数の引数の領域を確保する
        int i = 0;
        for(LVar *var = fn->params; var; var = var->next){
            printf("  mov [rbp-%d], %s\n", var->offset, argreg[i++]);
        }

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
}

void gen(Node *node){
	if(node == NULL) return;

    switch(node->kind){
	case ND_RETURN:
		gen(node->lhs);
		printf("  pop rax\n");
		printf("  jmp .Lreturn_%s\n", funcname);
		return;
	case ND_IF:
		if(node->els){
			int cnt_label_tmp = cnt_label++;
			gen(node->cond);
			printf("  pop rax\n");
			printf("  cmp rax, 0\n");
			printf("  je .Lelse%d\n", cnt_label_tmp);
			gen(node->then);
			printf("  jmp .Lend%d\n", cnt_label_tmp);
			printf(".Lelse%d:\n", cnt_label_tmp);
			gen(node->els);
			printf(".Lend%d:\n", cnt_label_tmp);
		}
		else{
			int cnt_label_tmp = cnt_label++;
			gen(node->cond);
			printf("  pop rax\n");
			printf("  cmp rax, 0\n");
			printf("  je .Lend%d\n", cnt_label_tmp);
			gen(node->then);
			printf(".Lend%d:\n", cnt_label_tmp);
		}
		return;
	case ND_WHILE: {
		int cnt_label_tmp = cnt_label++;
		printf(".Lbegin%d:\n", cnt_label_tmp);
		gen(node->cond);
		printf("  pop rax\n");
		printf("  cmp rax, 0\n");
		printf("  je .Lend%d\n", cnt_label_tmp);
		gen(node->then);
		printf("  jmp .Lbegin%d\n", cnt_label_tmp);
		printf(".Lend%d:\n", cnt_label_tmp);
		return;
	}
	case ND_FOR: {
		int cnt_label_tmp = cnt_label++;
		gen(node->init);
		printf(".Lbegin%d:\n", cnt_label_tmp);
		gen(node->cond);
		printf("  pop rax\n");
		printf("  cmp rax, 0\n");
		printf("  je .Lend%d\n", cnt_label_tmp);
		gen(node->then);
		gen(node->inc);
		printf("  jmp .Lbegin%d\n", cnt_label_tmp);
		printf(".Lend%d:\n", cnt_label_tmp);
		return;
	}
	case ND_BLOCK:
		while(1){
			gen(node->body);
			if(node->next == NULL)
				break;
			node = node->next;
			if(node->next != NULL)
				printf("  pop rax\n");
		}
		return;
	case ND_FUNCCALL: {
		int n_args = 0;
		for(Node *arg = node->args; arg; arg = arg->next){
			gen(arg);
			n_args++;
		}
		for(int i = n_args - 1; i >= 0; i--){
			printf("  pop %s\n", argreg[i]);
		}
		int cnt_label_tmp = cnt_label++;
		printf("  mov rax, rsp\n");
		printf("  and rax, 15\n");
		printf("  jnz .Lcall%d\n", cnt_label_tmp);
		printf("  mov rax, 0\n");
		printf("  call %s\n", node->funcname);
		printf("  jmp .Lend%d\n", cnt_label_tmp);
		printf(".Lcall%d:\n", cnt_label_tmp);
		printf("  sub rsp, 8\n");
		printf("  mov rax, 0\n");
		printf("  call %s\n", node->funcname);
		printf("  add rsp, 8\n");
		printf(".Lend%d:\n", cnt_label_tmp);
		printf("  push rax\n");
		return;
	}
    case ND_NUM:
        printf("  push %d\n", node->val);
        return;
    case ND_LVAR:
		gen_addr(node);
		if(node->ty->kind != TY_ARRAY){
			load();
		}
        return;
    case ND_ASSIGN:
        gen_lval(node->lhs);
        gen(node->rhs);
		store();
        return;
	case ND_ADDR:
		gen_addr(node->lhs);
		return;
	case ND_DEREF:
		gen(node->lhs);
		if(node->ty->kind != TY_ARRAY){
			load();
		}
		return;
    }

	gen(node->lhs);
	gen(node->rhs);

	printf("  pop rdi\n");
	printf("  pop rax\n");

	switch(node->kind){
	case ND_ADD:
		printf("  add rax, rdi\n");
		break;
	case ND_PTR_ADD:
		printf("  imul rdi, %d\n", node->ty->base->size);
		printf("  add rax, rdi");
		break;
	case ND_SUB:
		printf("  sub rax, rdi\n");
		break;
	case ND_PTR_SUB:
		printf("  imul rdi, %d\n", node->ty->base->size);
		printf("  sub rax, rdi\n");
		break;
	case ND_PTR_DIFF:
		printf("  sub rax, rdi\n");
		printf("  cqo\n");
		printf("  mov rdi, %d\n", node->ty->base->size);
		printf("  idiv rdi\n");
	case ND_MUL:
		printf("  imul rax, rdi\n");
		break;
	case ND_DIV:
		printf("  cqo\n");
		printf("  idiv rdi\n");
		break;
	case ND_EQ:
		printf("  cmp rax, rdi\n");
		printf("  sete al\n");
		printf("  movzb rax, al\n");
		break;
	case ND_NE:
		printf("  cmp rax, rdi\n");
		printf("  setne al\n");
		printf("  movzb rax, al\n");
		break;
	case ND_LT:
		printf("  cmp rax, rdi\n");
		printf("  setl al\n");
		printf("  movzb rax, al\n");
		break;
	case ND_LE:
		printf("  cmp rax, rdi\n");
		printf("  setle al\n");
		printf("  movzb rax, al\n");
		break;
	}

	printf("  push rax\n");
}


