//---- enum & typedef ----

typedef struct Token Token;
typedef struct LVar LVar;
typedef struct Function Function;
typedef struct Node Node;

typedef enum {
	TK_RESERVED, // 記号
	TK_IDENT, // 識別子
	TK_NUM, // 整数トークン
	TK_IF, // ifトークン
	TK_EOF, // 入力終わりトークン
	TK_RETURN, // returnトークン
} TokenKind;

// トークン型

struct Token {
	TokenKind kind;	// トークンの種類
	Token *next; 	// 次の入力トークン
	int val; 		// kindがTK_NUMの場合、その数値
	char *str;		// トークン文字列
	int len;		// トークンの長さ
};

typedef enum {
	ND_ADD, // +
	ND_SUB, // -
	ND_MUL, // *
	ND_DIV, // /
	ND_EQ, // ==
	ND_NE, // !=
	ND_LT, // <
	ND_LE, // <=
	ND_ASSIGN, // =
	ND_ADDR, // unary &
	ND_DEREF, // unary *
	ND_LVAR, // local variable
	ND_RETURN, // return
	ND_NUM, // integer
	ND_IF, // if
	ND_WHILE, // while
	ND_FOR, // for
	ND_BLOCK, // {...}
	ND_FUNCCALL, // function call
} NodeKind;

// 抽象構文木のノードの型
struct Node {
	NodeKind kind; // ノードの型
	Node *lhs; // 左辺
	Node *rhs; // 右辺
	Node *cond; // 条件式
	Node *then; // 条件を満たした場合の処理
	Node *els; // 条件を満たさなかった場合の処理
	Node *init; // forの初期化処理
	Node *inc; // forのincrement処理
	Node *body; // block
	Node *next; // next node (for block)
	char *funcname; // 関数名
	Node *args; // 引数
	int val; // kindがND_NUMのとき、その数値
	LVar *lvar; // kindがND_LVARのとき、lvarへのポインタ
	int offset; // kindがND_LVARのとき、RBPからのoffset
};

// ローカル変数の型
struct LVar{
	LVar *next; // 次のローカル変数
	char *name; // ローカル変数の名前
	int len; // 変数名の長さ
	int offset; // RBPからのオフセット
};

// 関数型
struct Function{
	Function *next;
	char *name;
	Node *node;
	LVar *locals;
	LVar *params;
	int stack_size;
};


//---- prototypes ----

void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);

bool consume(char *op);
Token *consume_ident();
void expect(char *op);
int expect_number();
char *expect_ident();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
bool startswith(char *p, char *q);
int is_alnum(char c);
char *strndup(char *str, size_t len);
Token *tokenize(char *p);

Node *new_node(NodeKind kind);
Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_ifelse(Node *cond, Node *then, Node *els);
Node *new_node_while(Node *cond, Node *then);
Node *new_node_for(Node *init, Node *cond, Node *inc, Node *then);
Node *new_node_num(int val);
Function *program();
Function *function();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

void gen_lval(Node *node);
void gen(Node *node);