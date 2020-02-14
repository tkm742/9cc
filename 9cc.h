//---- enum & typedef ----

typedef enum {
	TK_RESERVED, // 記号
	TK_IDENT, // 識別子
	TK_NUM, // 整数トークン
	TK_IF, // ifトークン
	TK_EOF, // 入力終わりトークン
	TK_RETURN, // returnトークン
} TokenKind;

// トークン型
typedef struct Token Token;
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
	ND_LVAR, // local variable
	ND_RETURN, // return
	ND_NUM, // integer
	ND_IF, // if
	ND_ELSE, // else
} NodeKind;

// 抽象構文木のノードの型
typedef struct Node Node;
struct Node {
	NodeKind kind; // ノードの型
	Node *lhs; // 左辺
	Node *rhs; // 右辺
	int val; // kindがND_NUMのとき、その数値
	int offset; // kindがND_LVARのとき、RBPからのoffset
};

// ローカル変数の型
typedef struct LVar LVar;
struct LVar{
	LVar *next; // 次のローカル変数
	char *name; // ローカル変数の名前
	int len; // 変数名の長さ
	int offset; // RBPからのオフセット
};


//---- prototypes ----

void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);

bool consume(char *op);
Token *consume_ident();
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
bool startswith(char *p, char *q);
int is_alnum(char c);
Token *tokenize(char *p);

Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
void *program();
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