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


//---- prototypes ----

void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);

bool consume(char *op);
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
bool startswith(char *p, char *q);
Token *tokenize(char *p);

Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();
void gen(Node *node);