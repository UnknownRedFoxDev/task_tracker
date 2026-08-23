#ifndef PARSER_H_
#define PARSER_H_

#include "lexer.h"

typedef enum {
    NODE_TAG,
    NODE_AND,
    NODE_OR,
    NODE_NOT,
} Node_Kind;

typedef struct Node_s Node_t;
struct Node_s {
    Node_Kind kind;
    union {
        char *tag_name;
        struct {
            Node_t *lhs;
            Node_t *rhs;
        };
    };
};

typedef struct {
    Lexer *l;
    Token_t curr;
    Token_t prev;
} Parser;

const char *node_kind_to_cstr(Node_Kind kind);
const char *token_kind_to_cstr(Token_Kind kind);

// ------ Parser
Parser *init_parser(Lexer *l);
void clean_parser(Parser **s);

// Node creation
Node_t *create_tag_node(const char *name);
Node_t *__create_op_node(Node_Kind kind, Node_t *lhs, Node_t *rhs);
Node_t *create_and_node(Node_t *lhs, Node_t *rhs);
Node_t *create_or_node(Node_t *lhs, Node_t *rhs);
Node_t *create_not_node(Node_t *child);

// Token stepping
void advance_token(Parser *s);
bool match_token_and_advance(Parser *s, Token_Kind expected_kind);
void consume_token(Parser *s, Token_Kind expected_kind);

// Expression parsing
Node_t *parse_query(Parser *s);
Node_t *parse_expr(Parser *s);
Node_t *parse_and(Parser *s);
Node_t *parse_not(Parser *s);
Node_t *parse_elem(Parser *s);
// ---------------------


// ------ AST
void __dump_ast(Node_t *node, size_t level);
#define dump_ast(node) __dump_ast((node), 0)
void clean_ast(Node_t *node);
// ---------------------

#endif // PARSER_H_
