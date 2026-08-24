#include "../lib/parser.h"

const char *node_kind_to_cstr(Node_Kind kind)
{
    switch(kind) {
        case NODE_TAG: return "TAG";
        case NODE_NOT: return "NOT";
        case NODE_OR:  return "OR";
        case NODE_AND: return "AND";
    }
    UNREACHABLE("Node_Kind");
}

const char *token_kind_to_cstr(Token_Kind kind)
{
    switch(kind) {
        case TOKEN_UNK:    return "UNK";
        case TOKEN_DOT:    return "DOT";
        case TOKEN_TAG:    return "TAG";
        case TOKEN_AND:    return "AND";
        case TOKEN_OR:     return "OR";
        case TOKEN_NOT:    return "NOT";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_EOF:    return "EOF";
    }
    UNREACHABLE("Token_Kind");
}

Parser *init_parser(Lexer *l)
{
    Parser *s = calloc(1, sizeof(Parser));
    if (s == NULL) {
        nob_log(NOB_ERROR, "failed to allocate space for a parser");
        exit(1);
    }

    s->l = l;
    advance_token(s);
    return s;
}

void clean_parser(Parser **s)
{
    if (*s) {
        clean_lexer(&(*s)->l);
    }
    free((*s));
    *s = NULL;
}

Node_t *create_tag_node(const char *name)
{
    Node_t *node = calloc(1, sizeof(Node_t));
    node->tag_name = strdup(name);
    node->kind = NODE_TAG;
    return node;
}

Node_t *__create_op_node(Node_Kind kind, Node_t *lhs, Node_t *rhs)
{
    Node_t *node = calloc(1, sizeof(Node_t));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node_t *create_and_node(Node_t *lhs, Node_t *rhs)
{
    return __create_op_node(NODE_AND, lhs, rhs);
}

Node_t *create_or_node(Node_t *lhs, Node_t *rhs)
{
    return __create_op_node(NODE_OR, lhs, rhs);
}

Node_t *create_not_node(Node_t *child)
{
    return __create_op_node(NODE_NOT, child, NULL);
}

void advance_token(Parser *s)
{
    free(s->prev.string);
    s->prev = s->curr;
    s->curr = next_token(s->l);
}

bool match_token_and_advance(Parser *s, Token_Kind expected_kind)
{
    if (s->curr.kind == expected_kind) {
        advance_token(s);
        return true;
    }
    return false;
}

void consume_token(Parser *s, Token_Kind expected_kind)
{
    if (s->curr.kind == expected_kind) {
        advance_token(s);
    }
}

Node_t *parse_query(Parser *s)
{
    Node_t *root = parse_expr(s);
    if (!match_token_and_advance(s, TOKEN_EOF)) { // Consume EOF
        printf("[WARNING] parse_query(): last token is not an EOF; token: %s", token_kind_to_cstr(s->curr.kind));
        if (s->curr.string != NULL) {
            printf(", string: |%s|", s->curr.string);
        }
        printf("\n");
    }
    return root;
}

Node_t *parse_expr(Parser *s)
{
    Node_t *lhs = parse_and(s);

    while (match_token_and_advance(s, TOKEN_OR)) {
        Node_t *rhs = parse_and(s);
        lhs = create_or_node(lhs, rhs);
    }

    return lhs;
}

Node_t *parse_and(Parser *s)
{
    Node_t *lhs = parse_not(s);

    while (match_token_and_advance(s, TOKEN_AND)) {
        Node_t *rhs = parse_not(s);
        lhs = create_and_node(lhs, rhs);
    }

    return lhs;
}

Node_t *parse_not(Parser *s)
{
    if (match_token_and_advance(s, TOKEN_NOT)) {
        Node_t *node = parse_not(s);
        return create_not_node(node);
    }

    return parse_elem(s);
}

Node_t *parse_elem(Parser *s)
{
    if (match_token_and_advance(s, TOKEN_TAG)) {
        char *name = s->prev.string;
        return create_tag_node(name);
    }

    if (match_token_and_advance(s, TOKEN_LPAREN)) {
        Node_t *sub_nodes = parse_expr(s);
        if (!match_token_and_advance(s, TOKEN_RPAREN)) { // Consume the ')' left
            printf("[WARNING] parse_query(): last token is not an RPAREN; token: %s", token_kind_to_cstr(s->curr.kind));
            if (s->curr.string != NULL) {
                printf(", string: |%s|", s->curr.string);
            }
            printf("\n");
        }
        return sub_nodes;
    }

    UNREACHABLE("Either a tag or a '(' character were expected. None were supplied, what happend?");
}

void __dump_ast(Node_t *node, size_t level)
{
    switch(node->kind) {
        case NODE_TAG: {
            for (size_t i = 0; i < level; ++i) {
                printf(" ");
            }
            printf("- TAG: %s\n", node->tag_name);
            break;
        }
        case NODE_NOT: {
            assert(node->lhs != NULL && "A not-node's LHS should not be NULL");
            __dump_ast(node->lhs, level);
            for (size_t i = 0; i < level + 2; ++i) {
                printf(" ");
            }
            printf("- UNARY: NOT\n");
            break;
        }
        case NODE_AND: {
            assert(node->lhs != NULL && "An and-node's LHS should not be NULL");
            assert(node->rhs != NULL && "An and-node's RHS should not be NULL");
            for (size_t i = 0; i < level; ++i) {
                printf(" ");
            }
            printf("- BOOLEAN: AND\n");
            __dump_ast(node->lhs, level+4);
            __dump_ast(node->rhs, level+4);
            break;
        }
        case NODE_OR: {
            assert(node->lhs != NULL && "An or-node's LHS should not be NULL");
            assert(node->rhs != NULL && "An or-node's RHS should not be NULL");
            for (size_t i = 0; i < level; ++i) {
                printf(" ");
            }
            printf("- BOOLEAN: OR\n");
            __dump_ast(node->lhs, level+4);
            __dump_ast(node->rhs, level+4);
            break;
        }
        default:
            UNREACHABLE("Node_Type");
    }
}

void clean_ast(Node_t *node)
{
    // For not-node handling, their rhs will be NULL
    if (!node) return;

    if (node->kind == NODE_TAG) {
        free(node->tag_name);
        node->tag_name = NULL;
        free(node);
    } else {
        clean_ast(node->lhs);
        clean_ast(node->rhs);
    }
}

